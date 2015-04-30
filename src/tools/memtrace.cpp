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
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <getopt.h>
#include <boost/format.hpp>
#include "read_gzip.h"

using namespace std;
using namespace boost;

namespace memtrace { // anonymous
    bool debug = false;
    uint64_t t_start = 0;
    uint64_t t_end = numeric_limits<uint64_t>::max();
	const char* gfile = 0;
    const int MAXCPU = 128;
    const int LINESZ = 64;
    int tdelta = 1000;
	ofstream gout;

	struct event {
        // IMPORTANT: keep in sync with timer_cpu/memory_tracer.cpp
        uint64_t ts;
        uint64_t addr;
        uint16_t cr3;
        int cpu;
		bool w;
		event():ts(0),addr(0),cr3(0),cpu(0),w(false) {}
		bool is_marker() { return cpu==0 && addr==0 && cr3==0; }
	};

    struct mstat {
        uint64_t nr;
        uint64_t nw;
        inline void dump(const char* s, int n) 
		{
            if (nr > 0)
                cout << boost::format("==== %s %5d R %10d W %10d\n") % s % n % nr % nw;
        }
		inline void update(const event& e) { if (e.w) nw++; else nr++; }
    };


	inline ReadGzip& operator>> (ReadGzip& gz, event& t)
	{
        uint8_t tmp;
        gz >> t.ts; 
        gz >> t.addr;
        gz >> t.cr3;
        gz >> tmp;
	    t.cpu = tmp>>1;
	    t.w = tmp&0x1;
	    return gz;
	}

	inline ostream& operator<< (ostream& s, const event& e) 
	{
        const char* rw = e.w ? "W" : "R";
        s << boost::format("%10llu %s 0x%016X %8d [%d]") % e.ts % rw % e.addr % e.cr3 % e.cpu;
		return s;
	}

	inline double MBps (uint64_t b, uint64_t ns) 
	{
	    return ((double)b * (1000000000/(1024*1024))) / (double)ns; 
	}

    mstat cstats[MAXCPU];
} // namespace memtrace

using namespace memtrace;

static void usage(const char * s)
{
    cerr << "Usage: " << s << " tracefile.gz [flags]" << endl;
	cerr << "    -d			debug mode" << endl;
	cerr << "    -t0 t 		start time" << endl;
	cerr << "    -t1 t 		end time" << endl;
	cerr << "    -g file 	write time/bw graph file" << endl;
	cerr << "    -td delta 	delta time (for time graphs)" << endl;
    exit(1);
}

int main(int argc, char *argv[]) 
{
    if (argc < 2)
        usage(argv[0]);
    ReadGzip trace(argv[1]);
	for(int i = 2; i<argc; ++i) {
	     if (! ::strcmp(argv[i],"-d")) {
		     debug = true;
		 }
	     else if (! ::strcmp(argv[i],"-t0")) {
		     t_start = ::atoll(argv[++i]);
			 cout << "T_start " << t_start << endl;
		 }
	     else if (! ::strcmp(argv[i],"-t1")) {
		     t_end = ::atoll(argv[++i]);
			 cout << "T_end " << t_end << endl;
		 }
	     else if (! ::strcmp(argv[i],"-td")) {
		     tdelta = ::atoll(argv[++i]);
			 cout << "Tdelta " << tdelta << endl;
		 }
	     else if (! ::strcmp(argv[i],"-g")) {
		     gfile = argv[++i];
			 gout.open(gfile);
			 cout << "graph file " << gfile << endl;
		 }
		 else usage(argv[0]);
	}

    if (!trace) {
        cerr << "Error: cannot open tracefile '" << argv[1] << "'" << endl;
        exit(2);
    }

    int nt = 0;
	uint64_t ts0 = 0;
    uint64_t tdcur = 0;
    uint64_t rlines = 0;
    uint64_t wlines = 0;

	event e;
    while (!trace.eof())  {
		trace >> e;
		if (ts0 == 0)
		    ts0 = e.ts;

        if (e.ts - ts0 > t_end)
		    break;

		if (e.ts > ts0 && e.ts >= ts0 + t_start) {
            if (debug && e.is_marker()) {
                cout << "==== Time "<< e.ts << endl;
                for (int i=0; i<MAXCPU; ++i)
                    cstats[i].dump("CPU",i);
            }
            else  {
				cstats[e.cpu].update(e);
                if (debug) 
				    cout << e << endl;

				if (gfile) {
				   uint64_t td = e.ts/tdelta;
				   if (td <= tdcur) {
					   if (e.w)
                           wlines++;
					   else
                           rlines++;
                   }
                   else {
					  double rbw = MBps(rlines*LINESZ,tdelta); // MB/s
					  double wbw = MBps(wlines*LINESZ,tdelta); // MB/s
					  double ms = (double)tdcur*tdelta*1e-6;
                      if (rbw > 0 && wbw > 0)
                          gout << ms << " " << rbw << " " << wbw << endl;
					  tdcur = td;
					  rlines = wlines = 0; 
                  }
			   }
            }
	    }
        if (nt++ % (1024*1024) == 0) cout << nt/(1024*1024) << " " << flush;
    }
	if (gfile)
	    gout.close();

    cout << endl << "=== FINAL" << endl;

	uint64_t trd = 0;
	uint64_t twr = 0;
    for (int i=0; i<MAXCPU; ++i) {
		trd += cstats[i].nr;
		twr += cstats[i].nw;
        cstats[i].dump("CPU",i);
    }
	double nsec = (double)(e.ts-ts0); // total time in ns
	double rbw = (double)(trd * LINESZ * 1024)/nsec;
	double wbw = (double)(twr * LINESZ * 1024)/nsec;
	cout << "Time (msec)         " << nsec*1e-6 << endl;
	cout << "GB read             " << trd * LINESZ/(1024*1024) << endl;
	cout << "GB written          " << twr * LINESZ/(1024*1024) << endl;
	cout << "Read bw avg (MB/s)  " << rbw << endl;
	cout << "Write bw avg (MB/s) " << wbw << endl;

    return 0;
}

