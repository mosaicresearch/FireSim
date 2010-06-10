/*
 * statemachine.{cc,hh}
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

Vector<StateEntry> StateMachine::_stateEntryList;

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
	assert(p->has_network_header());
	const click_ip* ip_header = p->ip_header();
	StateEntry entry;
	entry.protocol = Protocol(ip_header->ip_p);
	entry.addr.srcIP = ip_header->ip_src;
	entry.addr.dstIP = ip_header->ip_dst;
	entry.addr.srcPort = 0;
	entry.addr.dstPort = 0;
	entry.replyAddr.srcIP = entry.addr.dstIP;
	entry.replyAddr.dstIP = entry.addr.srcIP;
	entry.replyAddr.srcPort = 0;
	entry.replyAddr.dstPort = 0;
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
		entry.addr.srcPort = tcp_header->th_sport;
		entry.addr.dstPort = tcp_header->th_dport;
		entry.replyAddr.srcPort = entry.addr.dstPort;
		entry.replyAddr.dstPort = entry.addr.srcPort;
		break;
	}
	case UDP:
	{
		const click_udp* udp_header = p->udp_header();
		entry.addr.srcPort = udp_header->uh_sport;
		entry.addr.dstPort = udp_header->uh_dport;
		entry.replyAddr.srcPort = entry.addr.dstPort;
		entry.replyAddr.dstPort = entry.addr.srcPort;
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
			for (Vector<StateEntry>::iterator it = _stateEntryList.begin() ; it < _stateEntryList.end(); it++) {
				if (entry.protocol == it->protocol && entry.addr.srcIP == it->addr.srcIP
						&& entry.addr.dstIP == it->addr.dstIP && entry.addr.srcPort == it->addr.srcPort
						&& entry.addr.dstPort == it->addr.dstPort) {
					//packet from connection initiator
					if (it->gotReply)
						state = ESTABLISHED;
					else
						state = NEW;
					foundConnection = true;
					break;
				} else if (entry.protocol == it->protocol && entry.addr.srcIP == it->replyAddr.srcIP
						&& entry.addr.dstIP == it->replyAddr.dstIP && entry.addr.srcPort == it->replyAddr.srcPort
						&& entry.addr.dstPort == it->replyAddr.dstPort) {
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
				for (Vector<StateEntry>::iterator it = _stateEntryList.begin() ; it < _stateEntryList.end(); it++) {
					if (entry.protocol == it->protocol && entry.addr.srcIP == it->addr.srcIP
							&& entry.addr.dstIP == it->addr.dstIP && entry.type == it->type) {
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
				for (Vector<StateEntry>::iterator it = _stateEntryList.begin() ; it < _stateEntryList.end(); it++) {
					if (entry.protocol == it->protocol && entry.addr.srcIP == it->replyAddr.srcIP
							&& entry.addr.dstIP == it->replyAddr.dstIP && ICMP_ECHO == it->type) {
						//reply packet to connection initiator
						state = ESTABLISHED;
						break;
					}
				}
			} else if (entry.type == ICMP_TSTAMPREPLY || entry.type == ICMP_IREQREPLY || entry.type == ICMP_MASKREQREPLY) {
				for (Vector<StateEntry>::iterator it = _stateEntryList.begin() ; it < _stateEntryList.end(); it++) {
					if (entry.protocol == it->protocol && entry.addr.srcIP == it->replyAddr.srcIP
							&& entry.addr.dstIP == it->replyAddr.dstIP && entry.type - 1 == it->type) {
						//reply packet to connection initiator
						state = ESTABLISHED;
						break;
					}
				}
			} else {
				for (Vector<StateEntry>::iterator it = _stateEntryList.begin() ; it < _stateEntryList.end(); it++) {
					if (entry.protocol != it->protocol && (
							(entry.addr.srcIP == it->addr.srcIP && entry.addr.dstIP == it->addr.dstIP)
							|| (entry.addr.srcIP == it->replyAddr.srcIP && entry.addr.dstIP == it->replyAddr.dstIP))) {
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

void
StateMachine::doNat(WritablePacket* p, String type, in_addr addr, uint16_t port) {
	uint8_t proto = p->ip_header()->ip_p;
	in_addr src_ip = p->ip_header()->ip_src;
	in_addr dst_ip = p->ip_header()->ip_dst;
	uint16_t src_port = 0;
	uint16_t dst_port = 0;
	if (proto == TCP) {
		src_port = p->tcp_header()->th_sport;
		dst_port = p->tcp_header()->th_dport;
	} else if (proto == UDP) {
		src_port = p->udp_header()->uh_sport;
		dst_port = p->udp_header()->uh_dport;
	}

	if (addr.s_addr != 0 || port != 0) {
		//we will change the replyAddr to its new NAT mapping and do src or dst NAT
		for (Vector<StateEntry>::iterator it = _stateEntryList.begin() ; it < _stateEntryList.end(); it++) {
			if (type == "dst") {
				if (proto == it->protocol && src_ip == it->addr.srcIP && dst_ip == it->addr.dstIP
						&& src_port == it->addr.srcPort	&& dst_port == it->addr.dstPort) {
					//dst nat => change src of replyAddr & dst of packet
					if (addr.s_addr != 0) {
						it->replyAddr.srcIP = addr;
						p->ip_header()->ip_dst = addr;
					}
					if (port != 0) {
						it->replyAddr.srcPort = port;
						if (proto == TCP)
							p->tcp_header()->th_dport = port;
						else if (proto == UDP)
							p->udp_header()->uh_dport = port;
					}
					break;
				}
			} else {
				//src nat might have been preceded by dst nat => check for src addr/port of reply
				//instead of normal dst addr/port
				if (proto == it->protocol && src_ip == it->addr.srcIP && dst_ip == it->replyAddr.srcIP
						&& src_port == it->addr.srcPort	&& dst_port == it->replyAddr.srcPort) {
					//src nat => change dst of reply
					if (addr.s_addr != 0) {
						it->replyAddr.dstIP = addr;
						p->ip_header()->ip_src = addr;
					}
					if (port != 0) {
						it->replyAddr.dstPort = port;
						if (proto == TCP)
							p->tcp_header()->th_sport = port;
						else if (proto == UDP)
							p->udp_header()->uh_sport = port;
					}
					break;
				}
			}
		}
	} else {
		//the NAT mapping is already present: just do src or dst NAT (received packet may be originator or reply!)
		for (Vector<StateEntry>::iterator it = _stateEntryList.begin() ; it < _stateEntryList.end(); it++) {
			if (type == "dst") {
				if (proto == it->protocol && src_ip == it->addr.srcIP && dst_ip == it->addr.dstIP
						&& src_port == it->addr.srcPort	&& dst_port == it->addr.dstPort) {
					//packet from connection originator
					p->ip_header()->ip_dst = it->replyAddr.srcIP;
					if (proto == TCP)
						p->tcp_header()->th_dport = it->replyAddr.srcPort;
					else if (proto == UDP)
						p->udp_header()->uh_dport = it->replyAddr.srcPort;
					break;
				} else if (proto == it->protocol && src_ip == it->replyAddr.srcIP && dst_ip == it->replyAddr.dstIP
						&& src_port == it->replyAddr.srcPort && dst_port == it->replyAddr.dstPort) {
					//reply packet to connection originator
					p->ip_header()->ip_dst = it->addr.srcIP;
					if (proto == TCP)
						p->tcp_header()->th_dport = it->addr.srcPort;
					else if (proto == UDP)
						p->udp_header()->uh_dport = it->addr.srcPort;
					break;
				} else if (proto == ICMP && proto != it->protocol) {
					//related packet
					if (src_ip == it->addr.srcIP && dst_ip == it->addr.dstIP) {
						//related packet from connection originator
						p->ip_header()->ip_dst = it->replyAddr.srcIP;
						break;
					} else if (src_ip == it->replyAddr.srcIP && dst_ip == it->replyAddr.dstIP) {
						//related reply packet to connection originator
						p->ip_header()->ip_dst = it->addr.srcIP;
						break;
					}
				}
			} else {
				//src nat might have been preceded by dst nat => check for src addr/port of reply
				//instead of normal dst addr/port
				if (proto == it->protocol && src_ip == it->addr.srcIP && dst_ip == it->replyAddr.srcIP
						&& src_port == it->addr.srcPort	&& dst_port == it->replyAddr.srcPort) {
					//packet from connection originator
					p->ip_header()->ip_src = it->replyAddr.dstIP;
					if (proto == TCP)
						p->tcp_header()->th_sport = it->replyAddr.dstPort;
					else if (proto == UDP)
						p->udp_header()->uh_sport = it->replyAddr.dstPort;
					break;
				} else if (proto == it->protocol && src_ip == it->replyAddr.srcIP && dst_ip == it->addr.srcIP
						&& src_port == it->replyAddr.srcPort && dst_port == it->addr.srcPort) {
					//reply packet to connection originator
					p->ip_header()->ip_src = it->addr.dstIP;
					if (proto == TCP)
						p->tcp_header()->th_sport = it->addr.dstPort;
					else if (proto == UDP)
						p->udp_header()->uh_sport = it->addr.dstPort;
					break;
				} else if (proto == ICMP && proto != it->protocol) {
					//related packet
					if (src_ip == it->addr.srcIP && dst_ip == it->replyAddr.srcIP) {
						//related packet from connection originator
						p->ip_header()->ip_src = it->replyAddr.dstIP;
						break;
					} else if (src_ip == it->replyAddr.srcIP && dst_ip == it->addr.srcIP) {
						//related reply packet to connection originator
						p->ip_header()->ip_src = it->addr.srcIP;
						break;
					}
				}
			}
		}
	}
}

CLICK_ENDDECLS
EXPORT_ELEMENT(StateMachine)
