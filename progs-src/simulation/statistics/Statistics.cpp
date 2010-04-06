/**
 * @file Statistics.cpp
 * @author Nico Van Looy (s0061909) & Jens De Wit (s0061864)
 * @brief Bachelor project of University of Antwerp - 2008-2009
 */

//Standard
#include <string>
#include <fstream>
#include <stdlib.h>

//Logger
#include "Poco/Logger.h"

//FireSim
#include "Statistics.h"
#include "../ApplicationConstants.h"
#include "../util/Util.h"

using Poco::Logger;

Statistics::Statistics(bool isTestRun){
	_isTestRun = isTestRun;
}

Statistics::~Statistics(){

}

void Statistics::askForTrace(std::string filename) {
	if (_isTestRun) {
		_traceVector.push_back(filename);
	} else {
		Logger::get("ConsoleLogger").information("Do you want a trace for these packets? (y/n)");
		std::cout << "> ";
		std::cin.clear();
		std::string x;

		while(true) {
			getline(std::cin, x);
			if ((x != "y") && (x != "n") && (x != "Y") && (x != "N")
					&& (x != "yes") && (x != "no")) {
				Logger::get("ConsoleLogger").information("Wrong input. Do you want a trace for these packets? (y/n)");
				std::cout << "> ";
			} else {
				break;
			}
		}
		if((x == "y") || (x == "yes") || (x == "Y")){
			//Add filename to pending traces
			_traceVector.push_back(filename);
		}
	}

}

void Statistics::doTraces() {
	for (std::vector<std::string>::const_iterator it = _traceVector.begin(); it < _traceVector.end(); it++) {
		std::string shellCommand = "click " + SCRIPT_DIR + TRACE_SCRIPT + " FILENAME=" + OUTPUT_PATH + *it +
			".dump >> " + OUTPUT_PATH + *it + ".txt";

		if (system(shellCommand.c_str())) {
			Logger::get("ConsoleLogger").fatal("shell command '" + shellCommand + "' has failed to execute. Aborting...");
			exit(1);
		}

		Logger::get("ConsoleLogger").information("Finished tracing " + *it + ".dump: see " + *it + ".txt in output folder");
	}
}

void Statistics::getUserReport() {
	std::string shellCommand = "click " + SCRIPT_DIR + SIMULATION_SCRIPT;
	if (system(shellCommand.c_str()) == -1) {
		Logger::get("ConsoleLogger").fatal("shell command '" + shellCommand + "' has failed to execute. Aborting...");
		exit(1);
	}

	int numLostPackets;
	std::string line;
	std::ifstream file(STATISTICS_FILENAME.c_str());

	if (file.is_open()) {
		getline(file, line);
		numLostPackets = stringToInt(line);
		Logger::get("ConsoleLogger").information("Total packets generated: " + line);

		getline(file, line);
		numLostPackets -= stringToInt(line);
		Logger::get("ConsoleLogger").information("Total packets correctly accepted: " + line);

		getline(file, line);
		numLostPackets -= stringToInt(line);
		std::string report = "Total packets incorrectly accepted: " + line;
		if (stringToInt(line) > 0) {
			report += " (see faulty_accept.dump in output folder)";
			Logger::get("ConsoleLogger").information(report);
			askForTrace("faulty_accept");
		} else {
			Logger::get("ConsoleLogger").information(report);
			removeFile("faulty_accept.dump", OUTPUT_PATH);
		}

		getline(file, line);
		numLostPackets -= stringToInt(line);
		Logger::get("ConsoleLogger").information("Total packets correctly rejected: " + line);

		getline(file, line);
		numLostPackets -= stringToInt(line);
		report = "Total packets incorrectly rejected: " + line;
		if (stringToInt(line) > 0) {
			report += " (see faulty_reject.dump in output folder)";
			Logger::get("ConsoleLogger").information(report);
			askForTrace("faulty_reject");
		} else {
			Logger::get("ConsoleLogger").information(report);
			removeFile("faulty_reject.dump", OUTPUT_PATH);
		}

		getline(file, line);
		numLostPackets -= stringToInt(line);
		Logger::get("ConsoleLogger").information("Total packets correctly dropped: " + line);

		getline(file, line);
		numLostPackets -= stringToInt(line);
		report = "Total packets incorrectly dropped: " + line;
		if (stringToInt(line) > 0) {
			report += " (see faulty_drop.dump in output folder)";
			Logger::get("ConsoleLogger").information(report);
			askForTrace("faulty_drop");
		} else {
			Logger::get("ConsoleLogger").information(report);
			removeFile("faulty_drop.dump", OUTPUT_PATH);
		}

		if (numLostPackets > 0)
			Logger::get("ConsoleLogger").warning("WARNING: " + intToString(numLostPackets) + " packets were lost.");

		file.close();
		removeFile(STATISTICS_FILENAME, "");
	} else {
		Logger::get("ConsoleLogger").fatal("Failed to open " + STATISTICS_FILENAME + "created by the click script " + SIMULATION_SCRIPT);
		exit(1);
	}
}
