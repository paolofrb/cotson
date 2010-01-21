// -*- C++ -*-
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

#include "net_timing_model.h"
#include "liboptions.h"
#include "parameters.h"
#include <iostream>
#include <fstream>

using namespace std;

// A VERY simple switch timing nodel.

class SimpleSwitch : public TimingModel
{
public:
    SimpleSwitch(Parameters &);
    ~SimpleSwitch() {};

    void startquantum(uint64_t gt, uint64_t nextgt);
    void packet_event(const MacAddress& src, const MacAddress& dst, int size, bool bcast = false);
    uint64_t get_delay();
    uint32_t register_lat() { return 0; }

    enum {DEFAULT_MAX_BANDWITH = 80000, DEFAULT_LATENCY = 0, DEFAULT_CFACTOR = 8};

protected:

    uint64_t max_bandwith_;
    uint64_t latency_;
    uint64_t cfactor_;

protected:
    // Current quantum size in ns
    uint64_t quantum_;

    // Messages and bytes processed in the current quantum
    uint64_t messages_;
    uint64_t bytes_;
};

namespace simple_switch {
	registerClass<TimingModel,SimpleSwitch> mediator_simple_timer_c("simple");
};

SimpleSwitch::SimpleSwitch(Parameters &p) :
        max_bandwith_(DEFAULT_MAX_BANDWITH),
        latency_(DEFAULT_LATENCY),
        cfactor_(DEFAULT_CFACTOR),
        quantum_(0),
        messages_(0),
        bytes_(0)
{ 
    if (p.has("max_bandwith"))
        max_bandwith_ = p.get<int>("max_bandwith");
    if (p.has("latency"))
        latency_ = p.get<int>("latency");
    if (p.has("cfactor"))
        cfactor_ = p.get<int>("cfactor");

	if (verbose_ > 0) 
	{
        cout << "latency=" << latency_ << endl;
        cout << "max_bandwith=" << max_bandwith_ << endl;
        cout << "cfactor=" << cfactor_ << endl;
    }
}

void 
SimpleSwitch::startquantum(uint64_t gt, uint64_t nextgt)
{
	uint64_t q = nextgt - gt;
    if (q != 0)
	    quantum_ = q;
    messages_ = 0 ;
    bytes_ = 0;
}

void
SimpleSwitch::packet_event(const MacAddress& src, const MacAddress& dst, int size, bool bcast)
{
    messages_++;
    bytes_ += size;
}

uint64_t
SimpleSwitch::get_delay()
{
    // Models a simple switch with constant throuput below some value 
	// if max bandwith == 0 there is no limit

    if (max_bandwith_ == 0) 
	    return latency_;

    if (bytes_* 8 < quantum_* max_bandwith_ * 1000000)
	    return latency_;
    else 
	    return latency_ * cfactor_; // N times more latency
}

/* vi: set sw=4 ts=4: */
