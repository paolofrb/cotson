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
#ifndef MAIN_MEMORY_H
#define MAIN_MEMORY_H

#include "memory_interface.h"
#include "liboptions.h"
#include "cache_timing_l2.h"

namespace Memory {

class Main : public Interface
{   
    public:
    Main(Parameters&);

	MemState read(const Access&,uint64_t,Trace&,MOESI_state);
	MemState readx(const Access&,uint64_t,Trace&,MOESI_state);
	MemState write(const Access&,uint64_t,Trace&,MOESI_state);
    MOESI_state state(const Access&,uint64_t);
    void invalidate(const Access&,uint64_t,const Trace&,MOESI_state);

    void updatePending();
	uint32_t item_size() const { return 64; }
    
    protected:
    void beginSimulation(){
      if (_is_timer && timer)
        timer->reset();
      updatePending();
    }

    private:
    uint64_t read_;  
    uint64_t write_;
    uint64_t access_;  // read + write + coherence traffic
    uint32_t latency;

    Timing::MainMemory * timer;
    bool _is_timer;
}; 

}
#endif
