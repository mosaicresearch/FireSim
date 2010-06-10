/**
 * @file StateCondition.h
 * @author Nico Van Looy (s0061909) & Jens De Wit (s0061864)
 * @brief Bachelor project of University of Antwerp - 2008-2009
 */

#ifndef STATECONDITION_H_
#define STATECONDITION_H_

//Standard
//Poco
//FireSim
#include "Condition.h"

class StateCondition : public Condition {
public:
	StateCondition(std::string values) {
		if(values.find("NEW") != std::string::npos)
			_values.push_back(true);
		else
			_values.push_back(false);
		if(values.find("ESTABLISHED") != std::string::npos)
			_values.push_back(true);
		else
			_values.push_back(false);
		if(values.find("RELATED") != std::string::npos)
			_values.push_back(true);
		else
			_values.push_back(false);
		if(values.find("INVALID") != std::string::npos)
			_values.push_back(true);
		else
			_values.push_back(false);
	}

	//don't call this methode but use print(std::ostream& stream, std::string name) instead
	void print(std::ostream& stream) {
		assert(false);
	}

	std::vector<bool> getAllowedStates() {
		return _values;
	}

private:
	std::vector<bool> _values;
};

#endif /* STATECONDITION_H_ */
