/**
 * @file Table.cpp
 * @author Nico Van Looy (s0061909) & Jens De Wit (s0061864)
 * @brief Bachelor project of University of Antwerp - 2008-2009
 */

#include "Table.h"
#include "assert.h"

Table::Table(std::string name): _name(name) {
	//Add standard targets
	this->add(new Chain("ACCEPT",true,true));
	this->add(new Chain("REJECT",true,true));
	this->add(new Chain("DROP",true,true));
	this->add(new Chain("RETURN",false));
	this->add(new Chain("ULOG",false));
	this->add(new Chain("DNAT",false));
	this->add(new Chain("SNAT",false));
	this->add(new Chain("MASQUERADE",false));
}

Table::~Table() {
	for(std::map<std::string,Chain*>::iterator it = _map.begin(); it != _map.end(); it++) {
		delete it->second;
	}
}

void Table::add(Chain* chain){
	_map.insert(std::pair<std::string, Chain*>(chain->getName(), chain));
}

Chain* Table::get(std::string name){
	assert((_map.find(name) != _map.end()));
	return _map.find(name)->second;
}

void Table::printClickClassifiers(std::ostream& ostream){
	for(std::map<std::string, Chain*>::iterator it = _map.begin(); it != _map.end(); it++){
		if (!it->second->isFinal()){
			it->second->printClickClassifiers(ostream);
		}
	}
}

void Table::printClickSimulation(std::ostream& ostream){
	for(std::map<std::string, Chain*>::iterator it = _map.begin(); it != _map.end(); it++){
		if (!it->second->isFinal()){
			it->second->printClickSimulation(ostream, _name);
		}
	}
}
