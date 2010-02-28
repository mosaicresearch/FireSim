/**
 * @file tablelinker.cc
 * @author Nico Van Looy (s0061909) & Jens De Wit (s0061864)
 * @brief Bachelor project of University of Antwerp - 2008-2009
 */

/*
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
#include "tablelinker.hh"
#include <click/confparse.hh>
#include <click/error.hh>
#include <iostream>
#include "assert.h"
CLICK_DECLS

TableLinker::TableLinker()
{
	  _accept = 0;
	  _from = 0;
	  _manglePrerouting = 0;
	  _mangleInput = 0;
	  _mangleForward = 0;
	  _mangleOutput = 0;
	  _manglePostrouting = 0;
	  _natPrerouting = 0;
	  _natPostrouting = 0;
	  _natOutput = 0;
	  _filterInput = 0;
	  _filterForward = 0;
	  _filterOutput = 0;
	  _chain = "";
}

TableLinker::~TableLinker()
{
}

int
TableLinker::configure(Vector<String> &conf, ErrorHandler *errh)
{
	//	INPUT
	//	prerouting mangle
	//	prerouting nat
	//	input mangle
	//	input filter
	//
	//	FORWARD
	//	prerouting mangle
	//	prerouting nat
	//	forward mangle
	//	forward filter
	// 	postrouting mangle
	// 	postrouting nat
	//
	//	OUTPUT
	//	output mangle
	//	output nat
	//	output filter
	//	postrouting mangle
	//	postrouting nat
	if (cp_va_kparse(conf, this, errh,
			     "ACCEPT", cpkP+cpkM, cpElement, &_accept,
			     "MANGLE_PREROUTING", cpkP+cpkM, cpElement, &_manglePrerouting,
			     "MANGLE_INPUT", cpkP+cpkM, cpElement, &_mangleInput,
			     "MANGLE_FORWARD", cpkP+cpkM, cpElement, &_mangleForward,
			     "MANGLE_OUTPUT", cpkP+cpkM, cpElement, &_mangleOutput,
			     "MANGLE_POSTROUTING", cpkP+cpkM, cpElement, &_manglePostrouting,
			     "NAT_PREROUTING", cpkP+cpkM, cpElement, &_natPrerouting,
			     "NAT_POSTROUTING", cpkP+cpkM, cpElement, &_natPostrouting,
			     "NAT_OUTPUT", cpkP+cpkM, cpElement, &_natOutput,
			     "FILTER_INPUT", cpkP+cpkM, cpElement, &_filterInput,
			     "FILTER_FORWARD", cpkP+cpkM, cpElement, &_filterForward,
			     "FILTER_OUTPUT", cpkP+cpkM, cpElement, &_filterOutput,
			     cpEnd) < 0)
		return -1;
    return 0;
}

void
TableLinker::onFromEvent(Element* fromElement){
	if ((fromElement == _natPrerouting) || (fromElement == _natPostrouting) || (fromElement == _natOutput)
			|| (fromElement == _filterInput) || (fromElement == _filterForward) || (fromElement == _filterOutput)
			|| (fromElement == _manglePrerouting) || (fromElement == _mangleInput) || (fromElement == _mangleForward) || (fromElement == _mangleOutput) || (fromElement == _manglePostrouting)){
		_from = fromElement;
	}
}

int
TableLinker::from_handler(const String& conf , Element* e, void *, ErrorHandler *errh)
{
	TableLinker* tableLinker = (TableLinker*)e;
	Element* fromElement;
	if(cp_va_kparse(conf, tableLinker, errh,
			"FROM", cpkM+cpkP, cpElement, &fromElement
			, cpEnd) < 0)
		return -1;
	tableLinker->onFromEvent(fromElement);
	return true;
}

void
TableLinker::onChainEvent(String chain){
	assert((chain=="INPUT")  || (chain=="OUTPUT") || (chain=="FORWARD") || (chain=="INTERN"));
	_chain = chain;
}

int
TableLinker::chain_handler(const String& conf, Element* e, void*, ErrorHandler* errh)
{
	TableLinker* tableLinker = (TableLinker*)e;
	String chain;
	if(cp_va_kparse(conf, tableLinker, errh,
			"CHAIN", cpkM+cpkP, cpString, &chain
			, cpEnd) < 0)
		return -1;
	tableLinker->onChainEvent(chain);
	return true;
}


void
TableLinker::add_handlers()
{
	add_write_handler("from", from_handler, (void *)0);
	add_write_handler("chain", chain_handler, (void *)0);
}

void
TableLinker::push(int port, Packet* packet){
//	INPUT
//	prerouting mangle
//	prerouting nat
//	input mangle
//	input filter
//
//	FORWARD
//	prerouting mangle
//	prerouting nat
//	forward mangle
//	forward filter
// 	postrouting mangle
// 	postrouting nat
//
//	OUTPUT
//	output mangle
//	output nat
//	output filter
//	postrouting mangle
//	postrouting nat

	assert((_chain=="INPUT")  || (_chain=="OUTPUT") || (_chain=="FORWARD") || (_chain=="INTERN"));
	if (_chain=="INPUT"){
		assert((_from==_manglePrerouting) || (_from==_natPrerouting) || (_from==_mangleInput) || (_from==_filterInput));
		if (_from==_manglePrerouting){
			std::cout << "MANGLE Table (Prerouting Chain) -> NAT Table (Prerouting Chain)" << std::endl;
			_natPrerouting->push(port, packet);
		} else if (_from==_natPrerouting){
			std::cout << "NAT Table (Prerouting Chain) -> MANGLE Table (Input Chain)" << std::endl;
			_mangleInput->push(port, packet);
		} else if (_from==_mangleInput){
			std::cout << "MANGLE Table (Input Chain) -> FILTER Table (Input Chain)" << std::endl;
			_filterInput->push(port, packet);
		} else if (_from==_filterInput) {
			_accept->push(port, packet);
		}
	} else if (_chain=="FORWARD") {
		assert((_from==_manglePrerouting) || (_from==_natPrerouting) || (_from==_mangleForward) || (_from==_filterForward) || (_from==_manglePostrouting) || (_from==_natPostrouting));
		if (_from==_manglePrerouting){
			std::cout << "MANGLE Table (Prerouting Chain) -> NAT Table (Prerouting Chain)" << std::endl;
			_natPrerouting->push(port, packet);
		} else if (_from==_natPrerouting){
			std::cout << "NAT Table (Prerouting Chain) -> MANGLE Table (Forward Chain)" << std::endl;
			_mangleForward->push(port, packet);
		} else if (_from==_mangleForward){
			std::cout << "MANGLE Table (Forward Chain) -> FILTER Table (Forward Chain)" << std::endl;
			_filterForward->push(port, packet);
		} else if (_from==_filterForward) {
			std::cout << "FILTER Table (Forward Chain) -> MANGLE Table (Postrouting Chain)" << std::endl;
			_manglePostrouting->push(port, packet);
		} else if (_from==_manglePostrouting) {
			std::cout << "MANGLE Table (PostRouting Chain) -> NAT Table (Postrouting Chain)" << std::endl;
			_natPostrouting->push(port, packet);
		} else if (_from==_natPostrouting) {
			_accept->push(port, packet);
		}
	} else if (_chain=="OUTPUT") {
		assert((_from==_mangleOutput) || (_from==_natOutput) || (_from==_filterOutput) || (_from==_manglePostrouting) || (_from==_natPostrouting));
		if (_from==_mangleOutput){
			std::cout << "MANGLE Table (Output Chain) -> NAT Table (Output Chain)" << std::endl;
			_natOutput->push(port, packet);
		} else if (_from==_natOutput){
			std::cout << "NAT Table (Output Chain) -> FILTER Table (Output Chain)" << std::endl;
			_filterOutput->push(port, packet);
		} else if (_from==_filterOutput) {
			std::cout << "FILTER Table (Output Chain) -> MANGLE Table (Postrouting Chain)" << std::endl;
			_manglePostrouting->push(port, packet);
		} else if (_from==_manglePostrouting) {
			std::cout << "MANGLE Table (Postrouting Chain) -> NAT Table (Postrouting Chain)" << std::endl;
			_natPostrouting->push(port, packet);
		} else if (_from==_natPostrouting) {
			_accept->push(port, packet);
		}
	} else if (_chain=="INTERN") {
		//first output chain, afterwards input chain
		assert((_from==_manglePrerouting) || (_from==_natPrerouting) || (_from==_mangleInput) || (_from==_filterInput) || (_from==_mangleOutput) || (_from==_natOutput) || (_from==_filterOutput) || (_from==_manglePostrouting) || (_from==_natPostrouting));
		if (_from==_manglePrerouting){
			std::cout << "MANGLE Table (Prerouting Chain) -> NAT Table (Prerouting Chain)" << std::endl;
			_natPrerouting->push(port, packet);
		} else if (_from==_natPrerouting){
			std::cout << "NAT Table (Prerouting Chain) -> MANGLE Table (Input Chain)" << std::endl;
			_mangleInput->push(port, packet);
		} else if (_from==_mangleInput){
			std::cout << "MANGLE Table (Input Chain) -> FILTER Table (Input Chain)" << std::endl;
			_filterInput->push(port, packet);
		} else if (_from==_filterInput) {
			_accept->push(port, packet);
		} else if (_from==_mangleOutput){
			std::cout << "MANGLE Table (Output Chain) -> NAT Table (Output Chain)" << std::endl;
			_natOutput->push(port, packet);
		} else if (_from==_natOutput){
			std::cout << "NAT Table (Output Chain) -> FILTER Table (Output Chain)" << std::endl;
			_filterOutput->push(port, packet);
		} else if (_from==_filterOutput) {
			std::cout << "FILTER Table (Output Chain) -> MANGLE Table (Postrouting Chain)" << std::endl;
			_manglePostrouting->push(port, packet);
		} else if (_from==_manglePostrouting) {
			std::cout << "MANGLE Table (Postrouting Chain) -> NAT Table (Postrouting Chain)" << std::endl;
			_natPostrouting->push(port, packet);
		} else if (_from==_natPostrouting) {
			std::cout << "This packet is classified as INPUT." << std::endl;
			std::cout << "Start -> MANGLE Table (Prerouting Chain)" << std::endl;
			_manglePrerouting->push(port, packet);
		}
	}
}

CLICK_ENDDECLS
EXPORT_ELEMENT(TableLinker)
