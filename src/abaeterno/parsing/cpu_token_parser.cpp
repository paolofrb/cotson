// (C) Copyright 2006-2009 Hewlett-Packard Development Company, L.P.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//

// $Id$
#include "abaeterno_config.h"

#include "cpu_token_parser.h"
#include "tokens.h"
#include "liboptions.h"
#include "logger.h"
#include "error.h"
#include "cotson.h"
#include "interleaver.h"
#include "cpuid_call.h"

using namespace std;
using namespace boost;

CpuTokenParser::CpuTokenParser(TokenQueue* q,vector<uint64_t>& h,uint64_t d) :
	devid(d),
	trace_needs(0),
	queue(q),
	heartbeat(h)
{
	queue->anything=this;
	Cotson::Cpu::Tokens::code_off(devid);
	cur_cr3=0;
}

void CpuTokenParser::provide(const TraceNeeds *tn)
{
	trace_needs=tn;
	Interleaver::get().config(devid,insns,trace_needs);
}

void CpuTokenParser::run()
{
	uint64_t *tail = queue->tail;
	LOG(hex,static_cast<void*>(this),"empty queue: tokens=", dec, tail-queue->head);

	bool pending_prefetch = false;
	Memory::Access prefetch_address(0);

	Instruction* current_inst=0;
	int count = 0;
	for(const uint64_t *p=queue->head;p!=tail;p+=count)
	{
		uint64_t p0 = p[0];
		uint64_t p1 = p[1];
		count = TOKEN_COUNT(p0);

		switch(TOKEN_EVENT(p0))
		{
			case EVENT_INSTRUCTION:
			{
				if (count!=4 && count!=5) ERROR("incorrect instruction event token count");
				ERROR_IF(TOKEN_OPTIONS(p0)!=INSTRUCTIONS_ON, "type: INSTRUCTIONS_ON");
					
				uint64_t p2 = p[2];
				current_inst=insns.alloc();
				Instruction::init(current_inst,p1,p2,p[count-1],cur_cr3); 
				LOG(hex,static_cast<void*>(this),"inst",hex,p1);

				const Opcodes::datum *op = opcodes.find(p2);
				if (!op)
				{
					WARNING("no opcode at PC ", p2, "(",  static_cast<void*>(this), ")");
					current_inst = 0;
				}
				else
				{
					current_inst->CopyOpcode(op->getOpcode(),op->getLength());
					if (pending_prefetch)  // complete pending prefetch
					{
						current_inst->Type(InstType::PREFETCH);
					    current_inst->LoadsPush(prefetch_address);
					    pending_prefetch=false;
					}
					else
					    current_inst->Type(op->getType());
				}
			}
			break;

			case EVENT_MEMORY:
			{
				ERROR_IF(TOKEN_OPTIONS(p0)!=MEMORY_ON, "type: MEMORY_ON");
				if (!current_inst) ; // <- nothing to do here
					//WARNING("memory datum, no instruction, TOKEN=",p0);
				else
				{
					ERROR_IF(!(MEMORY_ISREAD(p1)||MEMORY_ISWRITE(p1)),
					         "read or write should be set");
					if (MEMORY_HASPHYS(p1)) // skip if there's no physical address
					{
					    uint64_t length=p[2];
				        if (count!=5) ERROR("incorrect memory event token count");
						if (MEMORY_ISREAD(p1))
						    current_inst->LoadsPush(Memory::Access(p[4],p[3],length));
						if (MEMORY_ISWRITE(p1)) 
						    current_inst->StoresPush(Memory::Access(p[4],p[3],length));
					}
				}
			}
			break;

			case EVENT_EXCEPTION:
			{
				if(count!=2) ERROR("an exception should have 2 tokens->");
				ERROR_IF(TOKEN_OPTIONS(p0)!=EXCEPTION_ON,"type: EXCEPTION_ON");

				uint64_t type=TOKEN_COUNT(p1)<<2;
				heartbeat[type-1]++;
			}
			break;

			case EVENT_HEARTBEAT:
			{
				if(count!=3) ERROR("a heartbeat should have 3 tokens");
				ERROR_IF(TOKEN_OPTIONS(p0)!=HEARTBEAT_ON, "type: heaHEARTBEAT_ON");
				heartbeat[p1-1]++;
			}
			break;
			
			case EVENT_CPUID:
			{
				if(count!=9) ERROR("a cpuid should have 9 tokens");
				if(p[2] == RESERVED_CPUID_FUNCTION) // RAX
				{
					uint64_t RDI=p[4];
					uint64_t RSI=p[6];
					uint64_t RBX=p[8];

					LOG("EVENT_CPUID");
					LOG("\tdevid: ",devid);
					LOG("\tRDI:   ",hex,RDI);
					LOG("\tRSI:   ",hex,RSI);
					LOG("\tRBX:   ",hex,RBX);
					// Fake instruction for the interleaver
					Instruction* fake_insn = insns.alloc();
					Instruction::init(fake_insn,UINT64_MAX,UINT64_MAX,1,cur_cr3); 
					fake_insn->Type(InstType::CPUID);
					fake_insn->cpuid_registers(RDI,RSI,RBX);
				}
			}
			break;

			case EVENT_PREFETCH: // p1:phys.addr, p2:virt.addr
			{
			    // Note: the prefetch address is injected *before* CODEFETCH
			    // by TaggedExecution, so we save it and add it to the next inst
			    if(count!=3) ERROR("a prefetch should have 3 tokens");
			    pending_prefetch=true; 
			    prefetch_address=Memory::Access(p1,p[2],8);
			}
			break;

			case EVENT_CR3:
			{
				if(count!=3) ERROR("A cr3 event should have 3 tokens");
				LOG("CR3 event",hex,cur_cr3);
			    cur_cr3=p[2];
			}
			break;
			
			default:
				ERROR("Event '",TOKEN_EVENT(p0),"' not defined!");
		}
	}
	queue->head = tail;
}

void CpuTokenParser::inject(InjectState state)
{
	switch(state)
	{
		case CODE:
		{
			const Cotson::Inject::info_instruction& ii=
				Cotson::Inject::current_opcode(bind(&Opcodes::malloc,&opcodes,_1));
			opcodes.insert(ii.pc,ii.opcode,ii.length,InstType(ii.opcode));
			LOG(hex,static_cast<void*>(this),"new pc",hex,ii.pc);

			if(ii.is_cr3_change)
			{
				LOG(hex,static_cast<void*>(this),"CR3 change detected (static code)");
				Cotson::Inject::token(EVENT_CR3,3); // length: 3
				Cotson::Inject::register_token("CR3");
			}

			if(ii.is_cpuid)
			{
				LOG(hex,static_cast<void*>(this),"CPUID detected (static code)");
				Cotson::Inject::token(EVENT_CPUID,9); // length: 9
				Cotson::Inject::register_token("RAX");
				Cotson::Inject::register_token("RDI");
				Cotson::Inject::register_token("RSI");
				Cotson::Inject::register_token("RBX");
			}
			Cotson::Inject::code();
		}
		break;
		case MEMORY:
			Cotson::Inject::memory();
			break;
		case EXCEPT:
			Cotson::Inject::exception();
			break;
		case HEARTBEAT:
			Cotson::Inject::heartbeat();
			break;
		default:
			ERROR("Inject state '",state,"' not valid");
	}
}

// Tagged CODE injection and execution

inline void CpuTokenParser::add_tokens(size_t n, const uint64_t* tokens)
{
    if (queue->tail + n >= queue->last)
    {
        LOG("add_tokens: empty queue");
        run(); // no space: empty the queue
        queue->head = queue->tail = queue->first;
    }
    (void)memcpy(queue->tail,tokens,n*sizeof(uint64_t));
    queue->tail += n;
}

void CpuTokenParser::tag(uint32_t tag, const Cotson::Inject::info_tag& ti)
{
    tagged_insts[tag]=ti;
}

bool CpuTokenParser::execute(uint32_t tag)
{
	ERROR_IF(tag==0,"Attempting to execute with tag==0");
    tag_map::const_iterator i = tagged_insts.find(tag);
	if (i == tagged_insts.end())
	    return false;

    const Cotson::Inject::info_tag& ti = i->second;
	switch (ti.type)
	{
	    case Cotson::Inject::PREFETCH:
		{
			uint64_t pft_va = Cotson::Memory::address_from_tag(ti);
			uint64_t pft_pa = Cotson::Memory::physical_address(pft_va);
			uint64_t tokens[3] = { MAKE_EVENT(EVENT_PREFETCH,3), pft_va, pft_pa };
			add_tokens(3,tokens);
		}
		break;

	    default:
		    ERROR("Unknown tag type",ti.type);
	}
	return true;
}

void CpuTokenParser::doSetState(SimState new_state)
{
	if(new_state==sim_state()) 
	    return;

	LOG("doSetState", devid, new_state);
	
	bool flush_now=false;
	const StateNeeds& sn_old=trace_needs->st[sim_state()];
	const StateNeeds& sn_new=trace_needs->st[new_state];

	if(sn_old.needs_code != sn_new.needs_code)
	{
		flush_now=true;
		sn_new.needs_code ? 
			Cotson::Cpu::Tokens::code_on(devid) : 
			Cotson::Cpu::Tokens::code_off(devid) ;
	}

	if(sn_old.needs_memory != sn_new.needs_memory)
	{
		flush_now=true;
		sn_new.needs_memory ? 
			Cotson::Cpu::Tokens::memory_on(devid) : 
			Cotson::Cpu::Tokens::memory_off(devid) ;
	}

	if(sn_old.needs_exception != sn_new.needs_exception)
	{
		flush_now=true;
		sn_new.needs_exception ? 
			Cotson::Cpu::Tokens::exception_on(devid) : 
			Cotson::Cpu::Tokens::exception_off(devid) ;
	}

	if(sn_old.needs_heartbeat != sn_new.needs_heartbeat)
	{
		flush_now=true;
		sn_new.needs_heartbeat ? 
			Cotson::Cpu::Tokens::heartbeat_on(devid) : 
			Cotson::Cpu::Tokens::heartbeat_off(devid) ;
	}

	if(sn_old.needs_register != sn_new.needs_register)
	{
		flush_now=true;
		sn_new.needs_register ? 
			Cotson::Cpu::Tokens::register_on(devid) : 
			Cotson::Cpu::Tokens::register_off(devid) ;
	}

	if(flush_now)
	{
		opcodes.clear();
		insns.clear();
		tagged_insts.clear();
	}
}
