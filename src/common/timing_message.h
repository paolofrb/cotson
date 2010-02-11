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
#ifndef TIMING_MESSAGE_H
#define TIMING_MESSAGE_H

#include <string.h>
#include <stdint.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#if defined(__GNUC__)
# define __PACKED__ __attribute__((__packed__));
#else
# define __PACKED__ /**/
#endif

namespace TimingMessage 
{
	enum Type {
		UnknownMsg = 0x0,
	    PortRequestMsg = 0x0700, // Request data port
		TimeStampMsg = 0x0701,   // Timestamp
		NodeStartMsg = 0x0702,   // Node start
		NodeStopMsg = 0x0703,    // Node stop
		GTimeMsg = 0x0704,       // Global time
		TerminateMsg = 0x0705,   // Terminate mediator
		TimeQueryMsg = 0x0706,   // What's the time?
	};

	// Message type from packet
	inline static uint16_t type(const uint8_t *p)
	{
        return ntohs(reinterpret_cast<const ether_header*>(p)->ether_type);
	}

	class Base 
	{
	public:
		Base(void* p, ssize_t l):ptr_(p),sz_(l){}
	    const void* data() const { return ptr_; }
	    ssize_t len() const { return sz_; }
		void copy(const void* p,ssize_t f=0, ssize_t l=0) 
		{
		    ::memcpy(reinterpret_cast<uint8_t*>(ptr_)+f,p,(l==0?sz_:l)); 
		}
        ssize_t sendto(int, const sockaddr_in*) const;
        ssize_t recv(int fd);
	protected:
		void * const ptr_;
		const ssize_t sz_;
	};
};

// timestamp from nodes to mediator
class TimeStamp : public TimingMessage::Base
{
public:
	TimeStamp(const void *);
	TimeStamp(uint16_t, uint64_t, uint32_t, uint16_t);
	TimeStamp(uint16_t);
	uint16_t seqno() const { return data_.seqno; }
	uint64_t tstamp() const { return data_.tstamp; }
	uint32_t nodeid() const { return data_.nodeid; }
	bool valid() { return data_.type != TimingMessage::UnknownMsg; }
	static bool check(ssize_t l) { return l == sizeof(Data); }
private:
	struct Data { // Must overlap with Ethernet header (src[6],dst[6],type[2])
        uint64_t tstamp; // simulated time (us) -- 8B align 8
        uint32_t nodeid; // node                -- 4B align 4
		uint16_t type;   // type                -- 2B align 2
		uint16_t seqno;  // sequence number     -- 2B align 2
	} __PACKED__; // It should already be packed (see alignments above)
	Data data_;
};

// global time from mediator to nodes
class GlobalTime : public TimingMessage::Base
{
public:
	GlobalTime();
	GlobalTime(uint16_t);
	GlobalTime(uint64_t, uint32_t, uint16_t);
	uint32_t lat() const { return data_.lat; }
	uint64_t gt() const { return data_.gt; }
	uint16_t seqno() const { return data_.seqno; }
	bool is_terminate() const { return data_.gt == static_cast<uint64_t>(-1); }
	void mkterminate() { data_.gt = static_cast<uint64_t>(-1); }
	bool valid() { return data_.type != TimingMessage::UnknownMsg; }
private:
	struct Data { // Must overlap with Ethernet header (src[6],dst[6],type[2])
        uint64_t gt;      // next sync time (us)     -- 8B align 8
        uint32_t lat;     // network latency (us)    -- 4B align 4
		uint16_t type;    // type                    -- 2B align 2
		uint16_t seqno;   // sequence number         -- 2B align 2
	} __PACKED__; // It should already be packed (see alignments above)
	Data data_;
};

// dataport message
class DataPort : public TimingMessage::Base
{
public:
	DataPort(uint16_t);
	uint16_t port() const { return data_.data_port; }
private:
    struct Data {
	    uint8_t  srcdst[12];  // unused    -- 12B align 1
	    uint16_t type;        // type      --  2B align 2
	    uint16_t data_port;   // data_port --  2B align 2
	} __PACKED__; // It should already be packed (see alignments above)
	Data data_;
};

// generic ethernet packet
class EtherPacket : public TimingMessage::Base
{
public:
    EtherPacket(const ether_header* eh, const void* p, size_t n)
	    : Base(new uint8_t(sizeof(ether_header)+n), sizeof(ether_header)+n)
	{
		copy(eh,0,sizeof(ether_header));
		copy(p,sizeof(ether_header),n);
	}
	~EtherPacket() { delete[] reinterpret_cast<uint8_t*>(Base::ptr_); }
private:
};

#undef __PACKED__
#endif
