/**
 * @file Chain.cpp
 * @author Nico Van Looy (s0061909) & Jens De Wit (s0061864)
 * @brief Bachelor project of University of Antwerp - 2008-2009
 */

//Standard
#include <iostream>
#include "assert.h"

//FireSim
#include "Chain.h"

Chain::Chain(std::string name, bool finalChain, bool usedChain) :
	_name(name), _policy(""), _rules(), _finalChain(finalChain), _usedChain(usedChain) {
}

Chain::Chain(std::string name, std::string policy, bool finalChain, bool usedChain) :
	_name(name), _policy(policy), _rules(), _finalChain(finalChain), _usedChain(usedChain) {
}

Chain::~Chain() {
	for(std::vector<Rule*>::iterator it = _rules.begin(); it != _rules.end(); it++) {
		delete *it;
	}
}

std::string Chain::getName(){
	return _name;
}

bool Chain::isFinal(){
	return _finalChain;
}

void Chain::setUsed() {
	_usedChain = true;
}

void Chain::add(Rule* rule){
	_rules.push_back(rule);
}

void Chain::printClickClassifiers(std::ostream& ostream) {
	for (int index = 0; index < _rules.size(); index++) {
		ostream << "	Idle -> ";
		if (!_rules[index]->hasCondition() || _rules[index]->ignore()){
			ostream << this->getName() << (index + 1) << " :: IPClassifier(-);" << std::endl;
		} else if (_rules[index]->needsIPClassifier() && _rules[index]->needsClassifier()) {
			ostream << this->getName() << (index + 1) << " :: IPClassifier(";
			_rules[index]->printClickIPClassifier(ostream);
			ostream << "-);" << std::endl;
			ostream << "	Idle -> ";
			ostream  << this->getName() << (index + 1) << "B :: Classifier(";
			_rules[index]->printClickClassifier(ostream);
			ostream << "-);" << std::endl;
		} else if (_rules[index]->needsIPClassifier()) {
			//only ipclassifier is needed
			ostream << this->getName() << (index + 1) << " :: IPClassifier(";
			_rules[index]->printClickIPClassifier(ostream);
			ostream << "-);" << std::endl;
		} else if (_rules[index]->needsClassifier()){
			//only classifier is needed
			ostream  << this->getName() << (index + 1) << " :: Classifier(";
			_rules[index]->printClickClassifier(ostream);
			ostream << "-);" << std::endl;
		} else {
			assert(false);
		}
	}
	if((_rules.empty() && _usedChain) || (!_rules.empty())) {
		ostream << "	Idle -> " << this->getName() << (_rules.size() + 1) << " :: IPClassifier(-);" << std::endl;
	}
}

void Chain::printClickSimulation(std::ostream& ostream, std::string tableName) {
	int nrOfRules = _rules.size();
	for (int index = 0; index < nrOfRules; index++) {
		Chain* target = _rules[index]->getJumpChain();
		if (_rules[index]->ignore()) {
			//this rule must be ignored ==> jump to next rule
			ostream << "\t" << this->getName() << (index + 1) << "[0]"
			<< " -> InfoPainter(\"*** Rule ignored: " << _rules[index]->getIpTablesText().substr(3, std::string::npos) << " ***\")"
			<< " -> " << this->getName() << (index + 2) << ";" << std::endl;
		} else if (!_rules[index]->hasCondition()){
			//this rule has no conditions ==> jump to target
			ostream << "\t" << this->getName() << (index + 1) << "[0]"
			<< " -> StaticCounter(TABLE " << tableName << ", RULE \"" <<_rules[index]->getIpTablesText().substr(3, std::string::npos) << "\")"
			<< " -> InfoPainter(\"" << _rules[index]->getIpTablesText().substr(3, std::string::npos) << " (succeeded)\")";
			if (target->getName() == "RETURN") {
				ostream << " -> bt;" << std::endl;
			} else if (target->getName() == "MASQUERADE") {
				ostream << " -> BacktrackPainter(" << this->getName() << (index + 2) << ")"
				<< " -> masq;" << std::endl;
			} else if ((target->getName() == "SNAT") || (target->getName() == "DNAT")) {
				_rules[index]->printIPRewriter(ostream);
				ostream	<< " -> " << this->getName() << (index + 2) << ";" <<std::endl;
			} else if (target->isFinal()) {
				ostream << " -> " << target->getName() << ";" <<std::endl;
			} else {
				ostream << " -> BacktrackPainter(" << this->getName() << (index + 2) << ")"
				<< " -> " << target->getName() << "1" << ";" << std::endl;
			}
		} else if (_rules[index]->needsIPClassifier() && _rules[index]->needsClassifier()) {
			//this rule has conditions that must be simulated by an IPClassifier and a general Classifier
			ostream << "\t" << this->getName() << (index + 1) << "[0]"
			<< " -> " << this->getName() << (index + 1) << "B;" << std::endl;

			ostream << "\t" << this->getName() << (index + 1) << "[1]"
			<< " -> InfoPainter(\"" << _rules[index]->getIpTablesText().substr(3, std::string::npos) << " (failed)\")"
			<< " -> " << this->getName() << (index + 2) << ";" << std::endl;

			ostream << "\t" << this->getName() << (index + 1) << "B[0]"
			<< " -> StaticCounter(TABLE " << tableName << ", RULE \"" <<_rules[index]->getIpTablesText().substr(3, std::string::npos) << "\")"
			<< " -> InfoPainter(\"" << _rules[index]->getIpTablesText().substr(3, std::string::npos) << " (succeeded)\")";
			if (target->getName() == "RETURN") {
				ostream << " -> bt;" << std::endl;
			} else if (target->getName() == "MASQUERADE") {
				ostream << " -> BacktrackPainter(" << this->getName() << (index + 2) << ")"
				<< " -> masq;" << std::endl;
			} else if ((target->getName() == "SNAT") || (target->getName() == "DNAT")) {
				_rules[index]->printIPRewriter(ostream);
				ostream	<< " -> " << this->getName() << (index + 2) << ";" <<std::endl;
			} else if (target->isFinal()) {
				ostream << " -> " << target->getName() << ";" <<std::endl;
			} else {
				ostream << " -> BacktrackPainter(" << this->getName() << (index + 2) << ")"
				<< " -> " << target->getName() << "1" << ";" <<std::endl;
			}
			ostream << "\t" << this->getName() << (index + 1) << "B[1]"
			<< " -> InfoPainter(\"" << _rules[index]->getIpTablesText().substr(3, std::string::npos) << " (failed)\")"
			<< " -> " << this->getName() << (index + 2) << ";" << std::endl;
		} else if (_rules[index]->needsIPClassifier() || _rules[index]->needsClassifier()) {
			//only 1 classifier is needed
			ostream << "\t" << this->getName() << (index + 1) << "[0]"
			<< " -> StaticCounter(TABLE " << tableName << ", RULE \"" <<_rules[index]->getIpTablesText().substr(3, std::string::npos) << "\")"
			<< " -> InfoPainter(\"" << _rules[index]->getIpTablesText().substr(3, std::string::npos) << " (succeeded)\")";
			if (target->getName() == "RETURN") {
				ostream << " -> bt;" << std::endl;
			} else if (target->getName() == "MASQUERADE") {
				ostream << " -> BacktrackPainter(" << this->getName() << (index + 2) << ")"
				<< " -> masq;" << std::endl;
			} else if ((target->getName() == "SNAT") || (target->getName() == "DNAT")) {
				_rules[index]->printIPRewriter(ostream);
				ostream	<< " -> " << this->getName() << (index + 2) << ";" <<std::endl;
			} else if (target->isFinal()) {
				ostream << " -> " << target->getName() << ";" <<std::endl;
			} else {
				ostream << " -> BacktrackPainter(" << this->getName() << (index + 2) << ")"
				<< " -> " << target->getName() << "1" << ";" << std::endl;
			}
			ostream << "\t" << this->getName() << (index + 1) << "[1]"
			<< " -> InfoPainter(\"" << _rules[index]->getIpTablesText().substr(3, std::string::npos) << " (failed)\")"
			<< " -> " << this->getName() << (index + 2)  << ";" << std::endl;
		} else {
			assert(false);
		}
	}
	if (_usedChain) {
		if (_policy == "") {
			//no policy set ==> backtrace
			ostream << "\t" << this->getName() << (_rules.size() + 1) << "[0] -> InfoPainter(\"*** Backtracking: leaving "
			<< this->getName() << " chain ***\") -> bt;" << std::endl;
		} else {
			//jump to policy
			ostream << "\t" << this->getName() << (_rules.size() + 1) << "[0] -> InfoPainter(\"*** Policy of " << this->getName()
			<< " chain triggered: " << _policy << " ***\") -> " << _policy << ";" << std::endl;
		}
	}
}
