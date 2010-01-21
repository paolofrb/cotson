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

#include "main_memory.h"

using namespace std;
using namespace boost;

namespace Memory {

registerClass<Interface,Main> main_memory_c("memory");

Main::Main(Parameters& p)
{
	latency=p.get<uint32_t>("latency");
	name=p.get<string>("name");

	add("read", read_);
	add("write", write_);
    add("access", access_);


    string timing = p.get<string>("timing", "basic");
    if (timing == "basic")
    {
      _is_timer = false;
    }
    else if (timing == "l2")
    {
      _is_timer = true;
      timer = new Timing::MainMemory(p);
      add("timer", *timer);
    }
    else if (timing == "mainmem")
    {
      _is_timer = true;
      timer = new Timing::MainMemory(p);
      add("timer", *timer);
    }
    else
    {
      //assume basic by default
      _is_timer = false;
    }

	clear_metrics();
}

MemState Main::read(const Access& m,uint64_t tstamp,Trace& mt,MOESI_state ms)
{
	LOG(name,"read",m,mt);
    access_++;
	read_++;
    uint32_t new_latency = _is_timer ? timer->latency(tstamp,mt,m) : latency;
    return MemState(new_latency,EXCLUSIVE); 
}

MemState Main::readx(const Access& m,uint64_t tstamp,Trace& mt,MOESI_state ms)
{
	LOG(name,"readx",m,mt);
	if (ms != NOT_FOUND) {
        access_++;
		return MemState(0,MODIFIED); 
	}
	mt.add(this,READ);
	return read(m,tstamp,mt,ms);
}

MemState Main::write(const Access& m,uint64_t tstamp,Trace& mt,MOESI_state ms)
{
	LOG(name,"write",m,mt);
    uint32_t new_latency;
    access_++;
	write_++;
    new_latency = _is_timer ? timer->latency(tstamp,mt,m) : latency;
 	return MemState(new_latency, MODIFIED);
}

MOESI_state Main::state(const Access&,uint64_t)
{
	throw runtime_error("Main should not answer to state");
	return INVALID;
}

void Main::invalidate(const Access&,uint64_t tstamp,const Trace&,MOESI_state)
{
	throw runtime_error("Main should not answer to invalidate");
}

void Main::updatePending()
{
  //timer.updatePending();
}

}
