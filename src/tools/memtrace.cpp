// (C) Copyright 2015 Hewlett-Packard Development Company, L.P.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//

#include <zlib.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include "read_gzip.h"
#include <boost/format.hpp>

using namespace std;

#define MAXCPU 128
struct cpustat {
    uint64_t nr;
    uint64_t nw;
    void dump(int n) {
        if (nr > 0)
            cout << boost::format("==== CPU %4d R %10d W %10d\n") % n % nr % nw;
    }
};
cpustat stats[MAXCPU];


static void usage(const char * s)
{
    cerr << "Usage: " << s << " tracefile.gz [mode[:params]]" << endl;
    exit(1);
}

int main(int argc, char *argv[]) 
{
    if (argc < 2)
        usage(argv[0]);

    ReadGzip trace(argv[1]);
    if (!trace) {
        cerr << "Error: cannot open tracefile '" << argv[1] << "'" << endl;
        exit(2);
    }
    bool marker = false;
    while (trace && !trace.eof()) {
        // IMPORTANT: keep in sync with timer_cpu/memory_tracer.cpp
        uint64_t tstamp;
        uint64_t addr;
        uint16_t pid;
        uint8_t tmp;

        trace >> tstamp;
        trace >> addr;
        trace >> pid;
        trace >> tmp;

	uint32_t cpu = tmp >>1;

	if (pid==0 && cpu==0 && addr==0) {
            if (!marker) {
                 cout << "===================================================================\n"; // sample marker
		 for (int i=0; i<MAXCPU; ++i)
		         stats[i].dump(i);
            } 
            marker=true;
        }
	else  {
	    const char* rw = "R";
            if (tmp & 1) {
                rw = "W";
                stats[cpu].nw++;
            }
            else
                stats[cpu].nr++;
	    cout << boost::format("%10llu %s 0x%016X %8d [%d]\n") % tstamp % rw % addr % pid % cpu;
            marker=false;
        }
    }
    cout << "=== FINAL" << endl;
    for (int i=0; i<MAXCPU; ++i)
        stats[i].dump(i);
    return 0;
}

