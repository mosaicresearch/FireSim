#ifndef CLICK_STATEMACHINE_HH
#define CLICK_STATEMACHINE_HH
#include <click/element.hh>
#include <click/string.hh>
#include <click/vector.hh>
CLICK_DECLS

enum State {
	NEW,
	ESTABLISHED,
	RELATED,
	INVALID
};

enum Protocol {
	ICMP = 1,
	TCP = 6,
	UDP = 17,
};

struct addrEntry {
	in_addr srcIP;
	in_addr dstIP;

	//TCP & UDP only
	uint16_t srcPort;
	uint16_t dstPort;
};

struct StateEntry {
	Protocol protocol;
	addrEntry addr;

	//for use with NAT
	addrEntry replyAddr;

	//ICMP only
	uint8_t type;
	uint8_t code;

	bool gotReply;
};

class StateMachine : public Element { public:

	StateMachine();
    ~StateMachine();

    const char *class_name() const		{ return "StateMachine"; }
    const char *port_count() const		{ return "1/1"; }
    const char *processing() const		{ return PUSH; }

    int configure(Vector<String> &conf, ErrorHandler *errh);

    void push(int, Packet *);

    void doNat(WritablePacket*, String type, in_addr addr, uint16_t port);

private:
	uint8_t _anno;
	static Vector<StateEntry> _stateEntryList;

};

CLICK_ENDDECLS
#endif // CLICK_STATEMACHINE_HH
