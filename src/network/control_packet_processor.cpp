// -*- C++ -*-
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


#include "control_packet_processor.h"
#include "switch.h"
#include "stats.h"
#include "timing_message.h"
#include "log.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

ControlPacketProcessor::ControlPacketProcessor(
    Switch *sw,
    const uint16_t &control_port,
    const uint16_t &data_port,
	int v
) : PacketProcessor(sw,v),
    control_port_(control_port),
    data_port_(data_port),
    data_port_msg_(data_port)
{}

ControlPacketProcessor::~ControlPacketProcessor()
{}

int ControlPacketProcessor::process(EventHandler *handler)
{
    // Process the control packet
    uint16_t type = TimingMessage::type(packet_.buf());
    switch (type) {
        case TimingMessage::PortRequestMsg:
            LOG2("(CTRL) Port request from", Sockaddr::str(handler->from()));
			if (!port_reply(handler))
			    return -1;
            break;

        case TimingMessage::TimeStampMsg:
			if (packet_.len() == TimeStamp::len()) {
                LOG2("(CTRL) Timestamp from", Sockaddr::str(handler->from()));
			    timestamp(handler);
			}
			else 
			    cerr << "Warning: discarded timestamp, len " << packet_.len()
				     << ", expected " << TimeStamp::len() << endl;
		    break;

        case TimingMessage::NodeStartMsg:
			if (packet_.len() == TimeStamp::len()) {
                LOG2("(CTRL) Node start from", Sockaddr::str(handler->from()));
			    start_node(handler);
			}
			else 
			    cerr << "Warning: discarded nodestart, len " << packet_.len()
				     << ", expected " << TimeStamp::len() << endl;
		    break;

        case TimingMessage::NodeStopMsg:
			if (packet_.len() == TimeStamp::len()) {
                LOG2("(CTRL) Node stop from", Sockaddr::str(handler->from()));
			    stop_node(handler);
			}
			else 
			    cerr << "Warning: discarded nodestop, len " << packet_.len()
				     << ", expected " << TimeStamp::len() << endl;
		    break;

        case TimingMessage::TimeQueryMsg:
			if (packet_.len() == TimeStamp::len()) {
                LOG2("(CTRL) Time query from", Sockaddr::str(handler->from()));
			    time_query(handler);
			}
			else
			    cerr << "Warning: discarded time query, len " << packet_.len()
				     << ", expected " << TimeStamp::len() << endl;
		    break;

		case TimingMessage::GTimeMsg:
		    break; // We shouldn't be getting it, ignore

		case TimingMessage::TerminateMsg:
			if (packet_.len() == TimeStamp::len()) {
			    cout << "(CTRL) Got a termination message from node "
			         << process_timing_message(handler)->id()
				     << ": goodbye!" << endl;
				switch_->send_terminate();
			    Stats::get().print_stats(1);
		        exit(0); // Goodbye!
			}
			else 
			    cerr << "Warning: discarded terminate, len " << packet_.len()
				     << ", expected " << TimeStamp::len() << endl;
		    break; 

        default:
            cout << "(CTRL) Unknown timing message: 0x" 
			     << hex << type << dec << endl;
            break;
    }
    return 0;
}

bool ControlPacketProcessor::port_reply(EventHandler *h)
{
    // send data port message back
    const uint8_t* msg = data_port_msg_.bytes();
    const size_t len = data_port_msg_.len();
    size_t nb = h->sendto(msg,len,h->from(),sizeof(sockaddr_in));
    if (nb != len) {
        cerr << "Error in port request reply "
             << "(len=" << len << ", sent=" << nb << ")" 
             << endl;
        return false;
    }
	// Register the new node in the switch
	const sockaddr_in *from = reinterpret_cast<const sockaddr_in*>(h->from());
	switch_->register_node(*from);
	return true;
}

void ControlPacketProcessor::timestamp(EventHandler *h)
{
	Stats::get().n_msg_++;
	switch_->heartbeat_node(*process_timing_message(h));
}

void ControlPacketProcessor::start_node(EventHandler *h)
{
	// Message from abaeterno signaling a node has started
	Stats::get().n_msg_++;
	switch_->heartbeat_node(*process_timing_message(h));
}

void ControlPacketProcessor::stop_node(EventHandler *h)
{
	// Message from abaeterno signaling a node has stopped
	Stats::get().n_msg_++;
	switch_->stop_node(*process_timing_message(h));
}

void ControlPacketProcessor::time_query(EventHandler *h)
{
	// Message from abaeterno asking a resend of the last time
	Stats::get().n_msg_++;
	Node::Ptr node = process_timing_message(h);
	if (node->valid())
	    switch_->timeout(); // force a sync
}

Node::Ptr ControlPacketProcessor::process_timing_message(EventHandler *h)
{
	TimeStamp tdata(packet_.buf());
	uint32_t nodeid = tdata.nodeid();
	uint64_t tstamp = tdata.tstamp();
	uint16_t seqno = tdata.seqno();

	// Register the new node in the switch
	const sockaddr_in *from = reinterpret_cast<const sockaddr_in*>(h->from());
	Node::Ptr node = switch_->register_node(nodeid,*from);
    node->setSimtime(tstamp,seqno); // Set the node sim time
    LOG2("(CTRL) Timing Message: Nodeid", nodeid, "Time", tstamp, "MAC", node->mac().str());
    return node;
}

