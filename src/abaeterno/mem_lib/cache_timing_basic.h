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
#ifndef TIMING_BASIC_H
#define TIMING_BASIC_H

#include "libmetric.h"
#include "liboptions.h"
#include "memory_interface.h"

namespace Memory {
namespace Timing {

class Basic : public metric
{
public:
	Basic(const Parameters &p):base_latency(p.get<uint32_t>("latency")) {}
	INLINE void reset() {}
	INLINE uint32_t latency(uint64_t,const Trace&, const Access&) const
	{
		return base_latency; 
	}
private:
    const uint32_t base_latency;
};

}}

#endif
