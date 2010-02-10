#include "timing_message.h"
#include "mac.h"

#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/ethernet.h>

#include <vector>

using namespace std;

class FakeNode
{
public:
    FakeNode(const string&, int, const string&, int, int);
    void send_timestamp(TimingMessage::Type, uint64_t);
    void send_message(MacAddress, const string&);
	bool ok() const { return !error; }
	uint8_t mac(uint i) const { return xmac[i]; }
	// uint64_t gt() { return GlobalTime(sync_sock).gt(); }
private:
    void send_packet(const void *, size_t);
    int get_data_port();

	int id; // node id
    int seqno; // sequence number

    int med_sock; // socket for mediator communication
    int sync_sock; // synchronization socket
    sockaddr_in med_ctl_addr; // mediator ctrl address
    sockaddr_in med_data_addr; // mediator data address
	bool error; // was there an error?
	MacAddress xmac; // mac address
};

FakeNode::FakeNode(
    const string& med_host, 
	int med_port,
	const string& mcast_ip, 
	int sync_port,
    int nodeid) : id(nodeid),seqno(0),error(false)
{
	// open UDP control socket to the mediator
	med_sock = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (med_sock == -1) {
		cerr << "Cannot create mediator socket" << endl;
		error=true;
		return;
	}
	const hostent *mh = ::gethostbyname(med_host.c_str());
	if (!mh) {
		cerr << "Cannot resolve mediator host " << med_host << endl;
	    ::close(med_sock);
		error=true;
		return;
	}
	::memset(&med_ctl_addr,0,sizeof(sockaddr_in));
	::memcpy((char *)&med_ctl_addr.sin_addr.s_addr,mh->h_addr_list[0],mh->h_length);
	med_ctl_addr.sin_family = mh->h_addrtype;
	med_ctl_addr.sin_port = htons(med_port);
    
    // Open UDP sync socket for timestamps from mediator
	sync_sock = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (sync_sock == -1) {
		cerr << "Cannot create sync socket" << endl;
		error=true;
		return;
	}
	// Let port be reused by others
	int yes = 1;
	if (::setsockopt(sync_sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
		cerr << "Cannot reuse port addresses" << endl;
		::close(sync_sock);
		sync_sock = -1;
		error=true;
		return;
	}
	// Create and bind to multicast address for timestamps
	sockaddr_in mcaddr;
	::memset(&mcaddr,0,sizeof(sockaddr_in));
	mcaddr.sin_family=AF_INET;
	mcaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	mcaddr.sin_port=htons(sync_port);
	if (::bind(sync_sock,(sockaddr *) &mcaddr,sizeof(mcaddr)) < 0) {
		cerr << "Cannot bind to the multicast address" << endl;
		::close(sync_sock);
		sync_sock = -1;
		error=true;
		return;
	}
	// Setup multicast IP membership
	ip_mreq mreq;
	mreq.imr_multiaddr.s_addr=inet_addr(mcast_ip.c_str());
	mreq.imr_interface.s_addr=htonl(INADDR_ANY);
	if (::setsockopt(sync_sock,IPPROTO_IP,IP_ADD_MEMBERSHIP, &mreq,sizeof(mreq)) < 0) {
		cerr << "Cannot add multicast membership" << endl;
		::close(sync_sock);
		sync_sock = -1;
		error=true;
		return;
	}
	// Get data port
	memcpy((char*)&med_data_addr,&med_ctl_addr,sizeof(sockaddr_in));
	med_data_addr.sin_port = htons(get_data_port());

	// assign MAC address and send initial message
	const uint8_t m[6]={0xfa,0xcd,1,0,0,nodeid};
	xmac=MacAddress(m);
}

int FakeNode::get_data_port()
{
     // send port request
     DataPort msg(med_sock);
     msg.sendto(med_sock,&med_ctl_addr); 
     // wait for port reply
     ssize_t nb = ::read(med_sock,msg.bytes(),msg.len());
     if (nb!=msg.len()) { error=true; return 0; }
     cout << "Node " << id << ": got data port " << msg.port() << endl;
     return msg.port();
}

void FakeNode::send_timestamp(TimingMessage::Type t, uint64_t ts)
{
    TimeStamp(t,ts,id,++seqno).sendto(med_sock,&med_ctl_addr);
}

void FakeNode::send_packet(const void *data, size_t n)
{
    size_t l = ::sendto(med_sock,data,n,0,
		                (const sockaddr*)&med_data_addr,
						sizeof(sockaddr_in));
	if (l != n) 
	    cerr << "Error sending packet: sent " << n << " returned " << l << endl;
}

void FakeNode::send_message(MacAddress m, const string& data)
{
	ether_header eh;
    eh.ether_type = ETHERTYPE_IP;
	for(uint i=0;i<6;++i) {
	    eh.ether_shost[i]=mac(i);
	    eh.ether_dhost[i]=m[i];
	}
	size_t sn = data.size();
	size_t en = sizeof(ether_header);
    uint8_t *buf = new uint8_t[sn+en];
	::memcpy(buf,&eh,en);
	::memcpy(buf+en,data.c_str(),sn);
    send_packet(buf,en+sn);
}

int main(int argc, char **argv)
{
    // Request data port
	if (argc < 8) {
	    cerr <<  "Usage: " 
		    << argv[0] 
			<< " med_host med_port sync_mcast_ip sync_port n_nodes n_timestamps mode" 
			<< endl;
		return 1;
	}
	string med_host(argv[1]);
	int med_port = atoi(argv[2]);
	string sync_mcast_ip(argv[3]);
	int sync_port = atoi(argv[4]);
	uint NN = atoi(argv[5]);
	uint NTS = atoi(argv[6]);
	int mode = atoi(argv[7]);

	vector<FakeNode> nodes;
	const uint8_t dest[6] = {1,2,3,4,5,6};
	MacAddress destmac(dest);

	if (mode==0) { // timestamp before data
	    // create 10 nodes
	    for (uint i=0; i < NN; ++i) {
		    FakeNode n(med_host,med_port, sync_mcast_ip,sync_port, i+1);
		    if (!n.ok()) return 1;
	        nodes.push_back(n);
        }
	    // send initial timestamp
	    for (uint i=0; i < NN; ++i)
	        nodes[i].send_timestamp(TimingMessage::TimeStampMsg,10);

	    // send data messages
	    for (uint i=0; i < NN; ++i)
	        nodes[i].send_message(destmac,"hello1");
	}
	if (mode==1) { // timestamp+data mixed
	    for (uint i=0; i < NN; ++i) {
		    FakeNode n(med_host,med_port, sync_mcast_ip,sync_port, i+1);
		    if (!n.ok()) return 1;
	        n.send_timestamp(TimingMessage::TimeStampMsg,10);
	        n.send_message(destmac,"hello2");
	        nodes.push_back(n);
	    }
	}
	if (mode==2) { // data before timestamps
	    // create 10 nodes
	    for (uint i=0; i < NN; ++i) {
		    FakeNode n(med_host,med_port, sync_mcast_ip,sync_port, i+1);
		    if (!n.ok()) return 1;
	        nodes.push_back(n);
        }
	    // send data messages
	    for (uint i=0; i < NN; ++i)
	        nodes[i].send_message(destmac,"hello0");
	    // send initial timestamp
	    for (uint i=0; i < NN; ++i)
	        nodes[i].send_timestamp(TimingMessage::TimeStampMsg,10);
	}
	if (mode==3) { // data+timestamp mixed (BAD)
	    for (uint i=0; i < NN; ++i) {
		    FakeNode n(med_host,med_port, sync_mcast_ip,sync_port, i+1);
		    if (!n.ok()) return 1;
	        n.send_message(destmac,"hello3");
	        n.send_timestamp(TimingMessage::TimeStampMsg,10);
	        nodes.push_back(n);
	    }
	}
	// send other timestamps
    uint64_t t = 100;
	for (uint i = 0; i < NTS; ++i) {
	    cout << "Send timestamp "<< t << endl;
	    for (uint i=0; i < nodes.size(); ++i)
	        nodes[i].send_timestamp(TimingMessage::TimeStampMsg,t+10*i);
	    t += 100;
	    sleep(0.2);
	}
	// send terminate to quit
	nodes[0].send_timestamp(TimingMessage::TerminateMsg,t);
    return 0;
}

