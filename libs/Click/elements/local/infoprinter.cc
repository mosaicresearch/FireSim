/*
 * infoprinter.{cc,hh}
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
#include "infoprinter.hh"
#include <click/confparse.hh>
#include <click/error.hh>
#include <click/packet_anno.hh>
#include <vector>
#include <iostream>
#include <fstream>
#include "assert.h"
CLICK_DECLS

InfoPrinter::InfoPrinter()
{
}

InfoPrinter::~InfoPrinter()
{
}

int
InfoPrinter::configure(Vector<String> &conf, ErrorHandler *errh)
{
    return cp_va_kparse(conf, this, errh,
    		"FILE", cpkP+cpkM, cpString, &_file,
    		cpEnd);
}

void
InfoPrinter::push(int, Packet* p)
{
	std::vector<String>* vector_ptr = (std::vector<String>*) p->anno_u32(EXTRA_PACKETS_ANNO_OFFSET);
	assert(p->anno_u32(EXTRA_PACKETS_ANNO_OFFSET) != 0);

	if(_file.compare(String("")) != 0) {
		std::ofstream file(_file.c_str(), std::ios::app);
		std::vector<String>::iterator it;
		for (it = vector_ptr->begin(); it < vector_ptr->end(); it++) {
			file << it->c_str() << std::endl;
		}
		file.close();
	}

	delete vector_ptr;
	output(0).push(p);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(InfoPrinter)
