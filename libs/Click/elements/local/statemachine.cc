/*
 * backtracker.{cc,hh}
 * Jens De Wit
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <click/config.h>
#include "statemachine.hh"
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <clicknet/ip.h>
#include <clicknet/icmp.h>
#include <clicknet/tcp.h>
#include <clicknet/udp.h>
CLICK_DECLS

StateMachine::StateMachine()
{
}

StateMachine::~StateMachine()
{
}

int
StateMachine::configure(Vector<String> &conf, ErrorHandler *errh)
{
    if (cp_va_kparse(conf, this, errh,
    		"ANNO", cpkP+cpkM, cpAnno, 1, &_anno,
    		cpEnd) < 0)
    {
			errh->error("Expecting an annotation to set the state");
    		return -1;
    }
    return 0;
}

void
StateMachine::push(int, Packet* p) {
	const click_ip* ip_header = p->ip_header();
	StateEntry entry;
	entry.protocol = Protocol(ip_header->ip_p);
	entry.srcIP = ip_header->ip_src;
	entry.dstIP = ip_header->ip_dst;
	entry.srcPort = 0;
	entry.dstPort = 0;
	entry.type = 0;
	entry.code = 0;
	entry.gotReply = false;

	switch(entry.protocol) {
		case ICMP:
		{
			const click_icmp* icmp_header = p->icmp_header();
			entry.type = icmp_header->icmp_type;
			entry.code = icmp_header->icmp_code;
			break;
		}
		case TCP:
		{
			const click_tcp* tcp_header = p->tcp_header();
			entry.srcPort = tcp_header->th_sport;
			entry.dstPort = tcp_header->th_dport;
			break;
		}
		case UDP:
		{
			const click_udp* udp_header = p->udp_header();
			entry.srcPort = udp_header->uh_sport;
			entry.dstPort = udp_header->uh_dport;
			break;
		}
		default:
			click_chatter("Statemachine: unsupported protocol, expecting icmp, tcp or udp.");
			p->kill();
			return;
	}

	State state = INVALID;
	if (_stateEntryList.empty()) {
		if (entry.protocol == TCP || entry.protocol == UDP
				|| (entry.protocol == ICMP && (entry.type == ICMP_ECHO || entry.type == ICMP_TSTAMP
						|| entry.type == ICMP_IREQ || entry.type == ICMP_MASKREQ))) {
			//add first connection
			_stateEntryList.push_back(entry);
			state = NEW;
		}
	} else {
		if (entry.protocol == TCP || entry.protocol == UDP) {
			bool foundConnection = false;
			for (std::vector<StateEntry>::iterator it = _stateEntryList.begin() ; it < _stateEntryList.end(); it++) {
				if (entry.protocol == it->protocol && entry.srcIP == it->srcIP && entry.dstIP == it->dstIP
						&& entry.srcPort == it->srcPort && entry.dstPort == it->dstPort) {
					//packet from connection initiator
					if (it->gotReply)
						state = ESTABLISHED;
					else
						state = NEW;
					foundConnection = true;
					break;
				} else if (entry.protocol == it->protocol && entry.srcIP == it->dstIP && entry.dstIP == it->srcIP
						&& entry.srcPort == it->dstPort	&& entry.dstPort == it->srcPort) {
					//reply packet to connection initiator
					it->gotReply = true;
					state = ESTABLISHED;
					foundConnection = true;
					break;
				}
			}
			if (!foundConnection) {
				//add new connection
				_stateEntryList.push_back(entry);
				state = NEW;
			}
		} else if (entry.protocol == ICMP) {
			if (entry.type == ICMP_ECHO || entry.type == ICMP_TSTAMP
					|| entry.type == ICMP_IREQ || entry.type == ICMP_MASKREQ) {
				bool foundConnection = false;
				for (std::vector<StateEntry>::iterator it = _stateEntryList.begin() ; it < _stateEntryList.end(); it++) {
					if (entry.protocol == it->protocol && entry.srcIP == it->srcIP && entry.dstIP == it->dstIP
							&& entry.type == it->type) {
						//packet from connection initiator
						state = NEW;
						foundConnection = true;
						break;
					}
				}
				if (!foundConnection) {
					//add new connection
					_stateEntryList.push_back(entry);
					state = NEW;
				}
			} else if (entry.type == ICMP_ECHOREPLY) {
				for (std::vector<StateEntry>::iterator it = _stateEntryList.begin() ; it < _stateEntryList.end(); it++) {
					if (entry.protocol == it->protocol && entry.srcIP == it->dstIP && entry.dstIP == it->srcIP
							&& ICMP_ECHO == it->type) {
						//reply packet to connection initiator
						state = ESTABLISHED;
						break;
					}
				}
			} else if (entry.type == ICMP_TSTAMPREPLY || entry.type == ICMP_IREQREPLY || entry.type == ICMP_MASKREQREPLY) {
				for (std::vector<StateEntry>::iterator it = _stateEntryList.begin() ; it < _stateEntryList.end(); it++) {
					if (entry.protocol == it->protocol && entry.srcIP == it->dstIP && entry.dstIP == it->srcIP
							&& ICMP_ECHO == it->type) {
						//reply packet to connection initiator
						state = ESTABLISHED;
						break;
					}
				}
			} else {
				for (std::vector<StateEntry>::iterator it = _stateEntryList.begin() ; it < _stateEntryList.end(); it++) {
					if (entry.protocol != it->protocol && ((entry.srcIP == it->srcIP && entry.dstIP == it->dstIP)
							|| (entry.srcIP == it->dstIP && entry.dstIP == it->srcIP))) {
						//related icmp error
						state = RELATED;
						break;
					}
				}
			}
		}
	}

	p->set_anno_u8(_anno, uint8_t(state));
	output(0).push(p);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(StateMachine)
