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
#include "sampler.h"
#include "liboptions.h"

class Simple : public Sampler
{
	public:
	Simple(Parameters&);
	~Simple() {}

	StateChange changeState(SimState);

	private:
	const uint64_t quantum;
};

registerClass<Sampler,Simple> simple_c("simple");

Simple::Simple(Parameters&p) : 
	Sampler(FULL_WARMING),
	quantum(p.get<uint64_t>("quantum"))
{
}

Sampler::StateChange Simple::changeState(SimState)
{
	return StateChange(SIMULATION,quantum);
}
