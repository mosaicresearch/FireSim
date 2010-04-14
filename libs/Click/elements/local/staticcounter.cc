/*
 * staticcounter.{cc,hh}
 * Jens De Wit
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2008 Regents of the University of California
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
#include "staticcounter.hh"
#include <click/error.hh>
#include <click/confparse.hh>
#include <click/glue.hh>
CLICK_DECLS

StaticCounter::StaticCounter()
{
}

StaticCounter::~StaticCounter()
{
}

std::map<String,int> StaticCounter::_nat;
std::map<String,int> StaticCounter::_mangle;
std::map<String,int> StaticCounter::_filter;

int
StaticCounter::configure(Vector<String> &conf, ErrorHandler *errh)
{
	if (cp_va_kparse(conf, this, errh,
		   "TABLE", cpkP, cpString, &_table,
		   "RULE", cpkP, cpString, &_rule,
		   cpEnd) < 0)
		return -1;

	if (_rule != "") {
		if (_table == "mangle")
			_mangle[_rule] = 0;
		else if (_table == "nat")
			_nat[_rule] = 0;
		else if (_table == "filter")
			_filter[_rule] = 0;
	}
	return 0;
}

void
StaticCounter::push(int, Packet* p)
{
	if (_rule == "") {
		click_chatter("Error: expecting RULE argument in configuration string.");
		return;
	}
	if (_table == "mangle")
		_mangle[_rule] = _mangle[_rule] + 1;
	else if (_table == "nat")
		_nat[_rule] = _nat[_rule] + 1;
	else if (_table == "filter")
		_filter[_rule] = _filter[_rule] + 1;
	else {
		click_chatter("Error: expecting TABLE argument in configuration string.");
		return;
	}

	output(0).push(p);
}

String
StaticCounter::unused_handler(Element *, void *)
{
	String output = String();
	bool first = true;
	for (std::map<String,int>::const_iterator it = _mangle.begin(); it != _mangle.end(); it++) {
		if (it->second == 0) {
			if (first) {
				output += "****************\n";
				output += "* MANGLE TABLE *\n";
				output += "****************\n";
				first = false;
			}
			output += (it->first + "\n");
		}
	}
	first = true;
	for (std::map<String,int>::iterator it = _nat.begin(); it != _nat.end(); it++) {
		if (it->second == 0) {
			if (first) {
				output += "*************\n";
				output += "* NAT TABLE *\n";
				output += "*************\n";
				first = false;
			}
			output += (it->first + "\n");
		}
	}
	first = true;
	for (std::map<String,int>::iterator it = _filter.begin(); it != _filter.end(); it++) {
		if (it->second == 0) {
			if (first) {
				output += "****************\n";
				output += "* FILTER TABLE *\n";
				output += "****************\n";
				first = false;
			}
			output += (it->first + "\n");
		}
	}
	return output;
}

void
StaticCounter::add_handlers()
{
    add_read_handler("unused", unused_handler, (void *)0);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(StaticCounter)
