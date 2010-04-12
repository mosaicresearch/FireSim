/**
 * @file Chain.h
 * @author Nico Van Looy (s0061909) & Jens De Wit (s0061864)
 * @brief Bachelor project of University of Antwerp - 2008-2009
 */

#ifndef CHAIN_H_
#define CHAIN_H_

//Standard
#include <vector>
#include <string>
#include <iostream>
//FireSim
#include "Rule.h"

/**
 * @class This class represtents an ipfilter chain.
 */
class Chain {
public:

	/**
	 * Constructor
	 * @param name This is the name of the chain.
	 * @param finalChain This is an indication whether the chain has transitions or not.(default false)
	 */
	Chain(std::string name, bool finalChain = false, bool usedChain = false);

	/**
	 * Constructor
	 * @param name This is the name of the chain.
	 * @param policy This is the policy of the chain.
	 * @param finalChain This is an indication whether the chain has transitions or not.(default false)
	 */
	Chain(std::string name, std::string policy, bool finalChain = false, bool usedChain = false);

	/**
	 * Destructor
	 */
	~Chain();

	/**
	 * Returns the name of the chain.
	 * @return The name of the chain.
	 */
	std::string getName();

	/**
	 * Returns whether the chain is final or not.
	 * @return boolean
	 */
	bool isFinal();

	/**
	 * Indicate that this chain is used in a rule somewhere
	 */
	void setUsed();

	/**
	 * Add a rule.
	 * Attention, the order in which the rules are added plays a significant role!
	 * The first one has the largest priority.
	 * @param rule This is the rule that will be added to this chain.
	 */
	void add(Rule* rule);

	/**
	 * Print click classifiers
	 * @param ostream This is the output stream. The classifier block will be printed to this stream.
	 */
	void printClickClassifiers(std::ostream& ostream, std::string prefix);

	/**
	 * Print click simulation
	 * @param ostream This is the output stream. The simulation block will be printed to this stream.
	 */
	void printClickSimulation(std::ostream& ostream, std::string prefix, std::string postfix);

private:
	std::string _name;
	std::string _policy;
	bool _finalChain;
	bool _usedChain;
	std::vector<Rule*> _rules;

};

#endif /* CHAIN_H_ */
