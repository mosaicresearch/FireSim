/*
 * backtrackpainter.{cc,hh}
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
#include "backtrackpainter.hh"
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <vector>
#include "assert.h"
CLICK_DECLS

BacktrackPainter::BacktrackPainter()
{
	_next = 0;
}

BacktrackPainter::~BacktrackPainter()
{
}

int
BacktrackPainter::configure(Vector<String> &conf, ErrorHandler *errh)
{
    if (cp_va_kparse(conf, this, errh,
    		"NEXT", cpkP+cpkM, cpElement, &_next,
    		cpEnd) < 0)
    {
			errh->error("Expecting next classifier.");
    		return -1;
    }
    return 0;
}

void
BacktrackPainter::push(int, Packet* p){
	assert(sizeof(std::vector<Element*>*) == sizeof(uint32_t));
	if (p->anno_u32(AGGREGATE_ANNO_OFFSET) != 0)
		((std::vector<Element*>*)p->anno_u32(AGGREGATE_ANNO_OFFSET))->push_back(_next);
	else {
		std::vector<Element*>* vector_ptr = new std::vector<Element*>();
		vector_ptr->push_back(_next);
		p->set_anno_u32(AGGREGATE_ANNO_OFFSET, uint32_t(vector_ptr));
	}
	output(0).push(p);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(BacktrackPainter)
