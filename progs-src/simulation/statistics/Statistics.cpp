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

Statistics::Statistics(){
}

Statistics::~Statistics(){
}

void Statistics::getUserReport() {
	Logger::get("ConsoleLogger").information("Simulate firewall with CLICK.");
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
			report += " (see faulty_accept.{dump,txt} in the output folder)";
			Logger::get("ConsoleLogger").information(report);
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
			report += " (see faulty_reject.{dump,txt} in the output folder)";
			Logger::get("ConsoleLogger").information(report);
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
			report += " (see faulty_drop.{dump,txt} in the output folder)";
			Logger::get("ConsoleLogger").information(report);
		} else {
			Logger::get("ConsoleLogger").information(report);
			removeFile("faulty_drop.dump", OUTPUT_PATH);
		}

		if (numLostPackets > 0)
			Logger::get("ConsoleLogger").warning("WARNING: " + intToString(numLostPackets) + " packets were lost.");

		file.close();
		removeFile(STATISTICS_FILENAME, "");

		//check if there are unused rules and inform the user or remove the empty file
		std::ifstream file((OUTPUT_PATH + UNUSED_RULES_FILENAME).c_str());
		file.seekg(0, std::ios::end);
		int length = file.tellg();
		file.close();

		if (length == 0)
			removeFile(UNUSED_RULES_FILENAME, OUTPUT_PATH);
		else
			Logger::get("ConsoleLogger").information("Unused rules detected: see " + UNUSED_RULES_FILENAME + " in the output folder.");

	} else {
		Logger::get("ConsoleLogger").fatal("Failed to open " + STATISTICS_FILENAME + " created by the click script " + SIMULATION_SCRIPT);
		exit(1);
	}
}
