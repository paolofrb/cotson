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

class ExpRandom : public Sampler
{
	public:
	ExpRandom(Parameters&);
	StateChange changeState(SimState);

	private:

	typedef boost::mt19937                                         generator;
	typedef boost::scoped_ptr<generator>                       ptr_generator;

	typedef boost::exponential_distribution<>                   distribution;
	typedef boost::variate_generator<generator&,distribution>        variate;
	typedef boost::scoped_ptr<variate>                           ptr_variate;

	ptr_generator gen;
	ptr_variate   eFunctional,eWarming,eSimulation;
};

using namespace std;

registerClass<Sampler,ExpRandom> exp_random_c("exp_random");

ExpRandom::ExpRandom(Parameters&p) : Sampler(FULL_WARMING)
{
	gen.reset(new generator(seed()));

	uint64_t functional =p.get<uint64_t>("functional");
	uint64_t warming    =p.get<uint64_t>("warming");
	uint64_t simulation =p.get<uint64_t>("simulation");

	double lFunctional=static_cast<double>(1)/functional;
	double lWarming=static_cast<double>(1)/warming;
	double lSimulation=static_cast<double>(1)/simulation;

	eFunctional.reset(new variate(*gen,distribution(lFunctional)));
	eWarming.reset(new variate(*gen,distribution(lWarming)));
	eSimulation.reset(new variate(*gen,distribution(lSimulation)));
}

Sampler::StateChange ExpRandom::changeState(SimState curr)
{
	StateChange next;
	switch(curr)
	{
		case FUNCTIONAL:
		{
			uint64_t t = (*eWarming)();
			next = StateChange(FULL_WARMING, t);
			break;
        }

		case FULL_WARMING:
		{
			uint64_t t = (*eSimulation)();
			next = StateChange(SIMULATION, t);
			break;
        }
	
		case SIMULATION:
		{
			uint64_t t = (*eFunctional)();
			next = StateChange(FUNCTIONAL, t);
			break;
        }

		default: ERROR("SimState not handled");
	}
	return next;
}
