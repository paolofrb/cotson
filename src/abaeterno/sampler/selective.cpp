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
#include "abaeterno.h"
#include "error.h"
#include "cpuid_call.h"
#include "cotson.h"

#include "sampler.h"
#include "liboptions.h"
#include "cpuid_call.h"

class Selective : public Sampler
{
	public:
	Selective(Parameters&);
	~Selective() {}

	StateChange changeState(SimState);

private:
	int current;
	std::string changer;
	std::vector<boost::shared_ptr<Sampler> > samplers;
	void* functional_call(FunctionalState,uint64_t,uint64_t);
	InstructionInQueue simulation_call(Instruction*);
};

registerClass<Sampler,Selective> selective_c("selective_tracing");

using namespace std;
using namespace boost;
using namespace boost::assign;

void* Selective::functional_call(FunctionalState fs,uint64_t b,uint64_t c)
{
	if(fs == ONLY_FUNCTIONAL) 
	{
	 	current=lexical_cast<int>(Option::run_function(changer, c==0, b));
		AbAeterno::get().break_sample();
	}
	return 0;
}

InstructionInQueue Selective::simulation_call(Instruction* inst)
{
	uint64_t  b = inst->get_xdata(2);
	uint64_t  c = inst->get_xdata(3);
	current=lexical_cast<int>(Option::run_function(changer, c==0, b));
	AbAeterno::get().break_sample();
	return DISCARD;
}

Selective::Selective(Parameters&p) : 
	Sampler(FUNCTIONAL),
	current(0),
	changer(p.get<string>("changer"))
{
	FunctionalCall f=bind(&Selective::functional_call,this,_1,_5,_6);
	SimulationCall s=bind(&Selective::simulation_call,this,_1);
	CpuidCall::add(COTSON_RESERVED_CPUID_SELECTIVE,f,s,0); 

	string constructor=p.get<string>("constructor");
	int quantity=p.get<int>("quantity");

	for(int i=0;i<quantity;i++)
	{
		luabind::object& luatable = Option::run_function(constructor, i);
		set<string> required_sam;
		required_sam += "type";
		Parameters o=Option::from_lua_table(luatable,required_sam,"Sampler");
		o.track();
		shared_ptr<Sampler> sp(Factory<Sampler>::create(o));
		ERROR_IF(!sp,"error creating sampler number ", i);
		samplers.push_back(sp);
	}
}

Sampler::StateChange Selective::changeState(SimState st)
{
	return samplers[current]->changeState(st);
}
