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
#ifndef INTERLEAVER_H
#define INTERLEAVER_H

#include "instructions.h"
#include "trace_needs.h"
#include "state_observer.h"

class Interleaver : public SimpleStateObserver
{
	Interleaver() {}
	~Interleaver() {}

public:
	inline static Interleaver& get()
	{
		static Interleaver singleton;
		return singleton;
	}

	void config(uint64_t,Instructions&,const TraceNeeds*);
	void notify();
	void break_sample();

protected:
	std::vector<std::pair<Instructions*,const TraceNeeds*> > it;
};

#endif
