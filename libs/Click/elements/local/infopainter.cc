/*
 * infopainter.{cc,hh}
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
#include "infopainter.hh"
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <click/handlercall.hh>
#include <vector>
#include "assert.h"
CLICK_DECLS

InfoPainter::InfoPainter()
{
}

InfoPainter::~InfoPainter()
{
}

int
InfoPainter::configure(Vector<String> &conf, ErrorHandler *errh)
{
    if (cp_va_kparse(conf, this, errh,
    		"LABEL", cpkP+cpkM, cpString, &_label,
    		"READHANDLER", cpkP, cpString, &_readhandler,
    		cpEnd) < 0)
    {
    		return -1;
    }
    return 0;
}

void
InfoPainter::push(int, Packet* p)
{
	String info = _label;
	if (_readhandler.compare(String("")) != 0) {
		HandlerCall hc(_readhandler);
		ErrorHandler *d_errh = ErrorHandler::default_handler();
		if (hc.initialize_read(this, d_errh) >= 0) {
			ContextErrorHandler c_errh(d_errh, "While calling %<%s%>:", hc.unparse().c_str());
			info += hc.call_read(&c_errh);
		}
	}

	assert(sizeof(std::vector<String>*) == sizeof(uint32_t));
	if (p->anno_u32(EXTRA_PACKETS_ANNO_OFFSET) != 0)
		((std::vector<String>*)p->anno_u32(EXTRA_PACKETS_ANNO_OFFSET))->push_back(info);
	else {
		std::vector<String>* vector_ptr = new std::vector<String>();
		vector_ptr->push_back(info);
		p->set_anno_u32(EXTRA_PACKETS_ANNO_OFFSET, uint32_t(vector_ptr));
	}
	output(0).push(p);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(InfoPainter)
