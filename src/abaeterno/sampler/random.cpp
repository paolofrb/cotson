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
#include "error.h"
#include "sampler.h"
#include "liboptions.h"

class Random : public Sampler
{
	public:

	Random(Parameters&);
	StateChange changeState(SimState);

	private:

	typedef boost::mt19937                                         generator;
	typedef boost::scoped_ptr<generator>                       ptr_generator;

	typedef boost::uniform_int<>                                distribution;
	typedef boost::variate_generator<generator&,distribution>        variate;
	typedef boost::scoped_ptr<variate>                           ptr_variate;

	ptr_generator gen;
	ptr_variate   uFunc;

	const uint64_t functional;
	const uint64_t warming;
	const uint64_t simulation;
};

using namespace std;

registerClass<Sampler,Random> random_c("random");

Random::Random(Parameters&p) : 
	Sampler(FULL_WARMING),
	functional (p.get<uint64_t>("functional")),
	warming    (p.get<uint64_t>("warming")),
	simulation (p.get<uint64_t>("simulation"))
{
	gen.reset(new generator(seed()));
	uFunc.reset(new variate(*gen,distribution(0,functional*2)));
}

Sampler::StateChange Random::changeState(SimState curr)
{
	StateChange next;
	switch(curr)
	{
		case FUNCTIONAL:  
			next = StateChange(FULL_WARMING,warming);
			break;

		case FULL_WARMING:
			next = StateChange(SIMULATION,simulation);
			break;
	
		case SIMULATION: 
		{
			uint64_t ts = (*uFunc)();
			next = StateChange(FUNCTIONAL, ts);
			break;
        }

		default: ERROR(true,"SimState not handled");
	}
	return next;
}
