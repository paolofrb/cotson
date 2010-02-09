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
};

// timestamp from nodes to mediator
class TimeStamp
{
public:
	inline TimeStamp(const uint8_t *p){ ::memcpy(bytes(),p,len()); }
	TimeStamp(uint16_t, uint64_t, uint32_t, uint16_t);
	TimeStamp(uint16_t);
	static inline const ssize_t len() { return sizeof(Data); }
	inline uint8_t* bytes() { return reinterpret_cast<uint8_t*>(&data_); }
	inline const uint8_t* bytes() const { return reinterpret_cast<const uint8_t*>(&data_); }
	inline uint16_t seqno() const { return data_.seqno; }
	inline uint64_t tstamp() const { return data_.tstamp; }
	inline uint32_t nodeid() const { return data_.nodeid; }
    ssize_t sendto(int, const sockaddr_in*) const;
	inline bool valid() { return data_.type != TimingMessage::UnknownMsg; }
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
class GlobalTime
{
public:
	GlobalTime();
	GlobalTime(uint16_t);
	GlobalTime(uint64_t, uint32_t, uint16_t);
	static inline const ssize_t len() { return sizeof(Data); }
	inline uint8_t* bytes() { return reinterpret_cast<uint8_t*>(&data_); }
	inline const uint8_t* bytes() const { return reinterpret_cast<const uint8_t*>(&data_); }
	inline uint32_t lat() const { return data_.lat; }
	inline uint64_t gt() const { return data_.gt; }
	inline uint16_t seqno() const { return data_.seqno; }
	inline bool is_terminate() const { return data_.gt == static_cast<uint64_t>(-1); }
	inline void mkterminate() { data_.gt = static_cast<uint64_t>(-1); }
    ssize_t sendto(int, const sockaddr_in*) const;
	inline bool valid() { return data_.type != TimingMessage::UnknownMsg; }
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
class DataPort
{
public:
	DataPort(uint16_t);
	static inline const ssize_t len() { return sizeof(Data); }
	inline uint8_t* bytes() { return reinterpret_cast<uint8_t*>(&data_); }
	inline const uint8_t* bytes() const { return reinterpret_cast<const uint8_t*>(&data_); }
    ssize_t sendto(int, const sockaddr_in*) const;
	uint16_t port() const { return data_.data_port; }
private:
    struct Data {
	    uint8_t  srcdst[12];  // unused    -- 12B align 1
	    uint16_t type;        // type      --  2B align 2
	    uint16_t data_port;   // data_port --  2B align 2
	} __PACKED__; // It should already be packed (see alignments above)
	Data data_;
};

#undef __PACKED__
#endif
