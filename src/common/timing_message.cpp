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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include <iostream>
#include <stdlib.h>
#include "timing_message.h"

#define NUM_RETRIES 5

#define __str(x) #x
#define ___str(x) __str(x)

using namespace std;

// Local utility
static inline ssize_t xsendto(int fd, const uint8_t* b, ssize_t l, const sockaddr_in* to)
{
	ssize_t n = 0;
	for (int t=0; t<NUM_RETRIES; ++t) {
        n = ::sendto(fd,b,l,0,reinterpret_cast<const sockaddr*>(to),sizeof(sockaddr_in));
        if (n == l)
            return n;
	}
	cout << "Cannot send message (size=" << l << " sent=" << n << ")" << endl;
    return -1;
}

// TimeStamp messages
TimeStamp::TimeStamp(uint16_t t, uint64_t ts, uint32_t id, uint16_t sn)
{
    data_.type = htons(t); // TimeStampMsg, NodeStartMsg, NodeStopMsg, TerminateMsg
    data_.tstamp = ts;
    data_.nodeid = id;
    data_.seqno = sn;
}

ssize_t TimeStamp::sendto(int fd, const sockaddr_in* to) const
{
    return xsendto(fd,bytes(),len(),to);
}

TimeStamp::TimeStamp(uint16_t fd)
{
    ssize_t nb = ::read(fd,bytes(),len());
    if (nb==len()) {
        switch(ntohs(data_.type)) {
            case TimingMessage::TimeStampMsg:
            case TimingMessage::NodeStartMsg:
            case TimingMessage::NodeStopMsg:
            case TimingMessage::TimeQueryMsg:
                return;
            default:
                break;
        }
    }
    data_.type = TimingMessage::UnknownMsg;
}

// GlobalTime messages
GlobalTime::GlobalTime()
{
    ::memset(bytes(),0,len()); 
    data_.type = htons(TimingMessage::GTimeMsg);
}

GlobalTime::GlobalTime(uint16_t fd)
{
    ssize_t nb = ::read(fd,bytes(),len());
    if (nb == len() && ntohs(data_.type) == TimingMessage::GTimeMsg)
        return;
    data_.type = TimingMessage::UnknownMsg;
}

GlobalTime::GlobalTime(uint64_t t0, uint32_t l, uint16_t sn)
{
    data_.type = htons(TimingMessage::GTimeMsg);
    data_.gt = t0;
    data_.lat = l;
    data_.seqno = sn;
}

ssize_t GlobalTime::sendto(int fd, const sockaddr_in* to) const
{
    return xsendto(fd,bytes(),len(),to);
}

// DataPort messages
DataPort::DataPort(uint16_t p) 
{
    data_.type = htons(TimingMessage::PortRequestMsg);
    data_.data_port = p;
    ::memset(bytes(),0,12);
}

ssize_t DataPort::sendto(int fd, const sockaddr_in* to) const
{
    return xsendto(fd,bytes(),len(),to);
}
