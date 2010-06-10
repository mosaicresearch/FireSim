/*
 * backtracker.{cc,hh}
 * Nico Van Looy & Jens De Wit
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
#include "backtracker.hh"
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <click/vector.hh>
#include "assert.h"
CLICK_DECLS

Backtracker::Backtracker()
{
}

Backtracker::~Backtracker()
{
}

int
Backtracker::configure(Vector<String> &conf, ErrorHandler *errh)
{
    return cp_va_kparse(conf, this, errh, cpEnd);
}

void
Backtracker::push(int port, Packet* p){
	Vector<Element*>* vector_ptr = (Vector<Element*>*) p->anno_u32(AGGREGATE_ANNO_OFFSET);
	assert(p->anno_u32(AGGREGATE_ANNO_OFFSET) != 0);

	Element* tmp = vector_ptr->back();
	vector_ptr->pop_back();

	tmp->push(port, p);

}

CLICK_ENDDECLS
EXPORT_ELEMENT(Backtracker)
