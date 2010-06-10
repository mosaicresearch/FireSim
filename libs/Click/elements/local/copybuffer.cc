/*
 * copybuffer.{cc,hh}
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
#include "copybuffer.hh"
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <click/vector.hh>
CLICK_DECLS

CopyBuffer::CopyBuffer()
{
}

CopyBuffer::~CopyBuffer()
{
}

int
CopyBuffer::configure(Vector<String> &conf, ErrorHandler *errh)
{
    return cp_va_kparse(conf, this, errh, cpEnd);
}

void
CopyBuffer::push(int port, Packet* p)
{
	if (port == 0) {
		_packet = p->clone();
		output(0).push(p);
	} else if (port == 1) {
		// copy the information added by the infopainters
		_packet->set_anno_u32(EXTRA_PACKETS_ANNO_OFFSET, p->anno_u32(EXTRA_PACKETS_ANNO_OFFSET));

		// delete the information added by the backtrackpainters before killing the incoming packet
		Vector<Element*>* vector_ptr = (Vector<Element*>*) p->anno_u32(AGGREGATE_ANNO_OFFSET);
		if (vector_ptr != 0)
			delete vector_ptr;
		p->kill();

		output(1).push(_packet);
	}
}

CLICK_ENDDECLS
EXPORT_ELEMENT(CopyBuffer)
