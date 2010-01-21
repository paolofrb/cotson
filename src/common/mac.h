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
#ifndef MACADDRESS_H
#define MACADDRESS_H

#include <string>
#include <ostream>
#include <string.h>
#include <stdint.h>
#include <netinet/ether.h>
#include <arpa/inet.h>

union MacAddress
{
public:
	MacAddress() { u64=0; } 
	MacAddress(const uint8_t x[6]) { u64=0; ::memcpy(b,x,6); }
	MacAddress(uint32_t b, uint32_t id) { u32[0]=htonl(b|(id>>16)); u32[1]=htonl(id<<16); }

	inline bool operator <(const MacAddress& t) const { return u64 < t.u64; }
	inline bool operator ==(const MacAddress& t) const { return u64 == t.u64; }
	inline bool operator !=(const MacAddress& t) const { return u64 != t.u64; }
	inline bool is_broadcast() const { return u64 == 0x0000FFFFFFFFFFFFULL; }

private:
	uint64_t u64;
	uint32_t u32[2];
	uint8_t b[8];
	friend std::ostream& operator<<(std::ostream&,const MacAddress&);
};

#endif 
