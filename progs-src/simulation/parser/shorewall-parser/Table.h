/**
 * @file Table.h
 * @author Nico Van Looy (s0061909) & Jens De Wit (s0061864)
 * @brief Bachelor project of University of Antwerp - 2008-2009
 */

#ifndef TABLE_H_
#define TABLE_H_

//Standard
#include <string>
#include <map>
#include <iostream>
//FireSim
#include "Chain.h"

/**
 * @class This class represents a collections of ipfilter chains.
 */
class Table {
public:
	/**
	 * Constructor
	 */
	Table();

	/**
	 * Destructor
	 */
	~Table();

	/**
	 * Add a chain to the collection.
	 * @param chain This is the chain.
	 */
	void add(Chain* chain);

	/**
	 * Get the chain with the specified name.
	 * @param name This is the name of the chain.
	 */
	Chain* get(std::string name);

	/**
	 * Print click classifier block
	 * @param ostream This is the output stream. The classifier block will be printed to this stream.
	 */
	void printClickClassifiers(std::ostream& ostream);

	/**
	 * Print click simulation block
	 * @param ostream This is the output stream. The simulation block will be printed to this stream.
	 */
	void printClickSimulation(std::ostream& ostream);

private:
	std::map<std::string, Chain*> _map;
};

#endif /* TABLE_H_ */
