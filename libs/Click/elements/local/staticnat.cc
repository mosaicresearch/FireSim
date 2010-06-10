/*
 * staticnat.{cc,hh}
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
#include "staticnat.hh"
#include <click/confparse.hh>
#include <click/error.hh>
CLICK_DECLS

StaticNat::StaticNat()
{
	_stateMachine = new StateMachine();
	_type = 0;
	_addr.s_addr = 0;
	_port = 0;
}

StaticNat::~StaticNat()
{
}

int
StaticNat::configure(Vector<String> &conf, ErrorHandler *errh)
{
    if (cp_va_kparse(conf, this, errh,
    		"TYPE", cpkM+cpkP, cpString, &_type,
    		"IP", 0, cpIPAddress, &_addr,
    		"PORT", 0, cpUnsigned, &_port,
    		cpEnd) < 0) {
    	return -1;
    }
    _port = htons(_port);
    if (_type == "src" || _type == "dst")
    	return 0;
    return -1;
}

void
StaticNat::push(int, Packet* p){
	WritablePacket* packet = p->uniqueify();
	_stateMachine->doNat(packet, _type, _addr, _port);
	output(0).push(packet);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(StaticNat)
