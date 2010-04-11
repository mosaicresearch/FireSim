/**
 * @file ClickGenerator.cpp
 * @author Nico Van Looy (s0061909) & Jens De Wit (s0061864)
 * @brief Bachelor project of University of Antwerp - 2008-2009
 */

//Standard
#include <iostream>
#include "assert.h"

//Poco
#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/Document.h"
#include "Poco/SAX/SAXException.h"
#include "Poco/Exception.h"

//FireSim
#include "ClickGenerator.h"
#include "../parser/shorewall-parser/ShorewallParser.h"
#include "../parser/network-parser/NetworkParser.h"
#include "../ApplicationConstants.h"

ClickGenerator::ClickGenerator(std::string config_path, bool isTestRun) {
	_filterTable = 0;
	_natTable = 0;
	_mangleTable = 0;
	_networkLayout = 0;
	_configParser = 0;

	std::string config_path2 = config_path;
	if (isTestRun)
		config_path2 += "../";

	Poco::XML::DOMParser DOMparser;

	//Parse network layout
	try {
		//Skip whitespace
		DOMparser.setFeature("http://www.appinf.com/features/no-whitespace-in-element-content", false);
		Poco::XML::Document* xmlDoc = DOMparser.parse(config_path2 + NETWORKLAYOUT_FILENAME);
		NetworkParser* networkParser = NetworkParser::getInstance();
		_networkLayout = networkParser->parse(xmlDoc);
	} catch (Poco::XML::SAXParseException e){
		std::cout << "Failure while parsing network_layout.xml file at line " << e.getLineNumber() << " and column " << e.getColumnNumber() << ": " << e.name() << std::endl;
		exit(1);
	}

	//Parse shorewall compiled
	ShorewallParser::createInstance(config_path2, _networkLayout);
	ShorewallParser* parser = ShorewallParser::getInstance();
	_filterTable = parser->parseFilterTable();
	_natTable = parser->parseNatTable();
	_mangleTable = parser->parseMangleTable();

	try {
		//Parse configuration
		Poco::XML::Document* xmlDoc2 = DOMparser.parse(config_path + CONFIG_FILENAME);
		ConfigParser::createInstance(config_path);
		_configParser = ConfigParser::getInstance();
		_numTrafficBlocks = _configParser->parse(xmlDoc2, _networkLayout);
	} catch (Poco::XML::SAXParseException e){
		std::cout << "Failure while parsing config.xml file at line " << e.getLineNumber() << " and column " << e.getColumnNumber() << ": " << e.name() << std::endl;
		exit(1);
	}
}

ClickGenerator::~ClickGenerator() {
	if (_filterTable)
		delete _filterTable;
	if (_natTable)
		delete _natTable;
	if (_mangleTable)
		delete _mangleTable;
	if (_networkLayout)
		delete _networkLayout;
	if (_configParser)
		delete _configParser;
}

void ClickGenerator::printTables(std::ostream& output, bool trace) {
	assert(_filterTable);
	assert(_natTable);
	assert(_mangleTable);

	output << "elementclass MangleTable {" << std::endl;
	output << "	//Backtracker declaration" << std::endl;
	output << "	Idle -> bt :: Backtracker;" << std::endl;
	output << std::endl;
	output << "	//Translation of the Iptables rules to click syntax" << std::endl;
	_mangleTable->printClickClassifiers(output, "MANGLE_");
	output << std::endl;
	if (trace) {
		output << "	//Pretty printers" << std::endl;
		_mangleTable->printPrettyPrinters(output, "MANGLE_");
		output << std::endl;
	}
	output << "	//Element input & output" << std::endl;
	output << "	input[0] -> ps :: PaintSwitch(ANNO 19);" << std::endl;
	output << "	ps[0] -> MANGLE_FORWARD1" << (trace?"Print;":";") << std::endl;
	output << "	ps[1] -> MANGLE_INPUT1" << (trace?"Print;":";") << std::endl;
	output << "	ps[2] -> MANGLE_OUTPUT1" << (trace?"Print;":";") << std::endl;
	output << "	ps[3] -> MANGLE_POSTROUTING1" << (trace?"Print;":";") << std::endl;
	output << "	ps[4] -> MANGLE_PREROUTING1" << (trace?"Print;":";") << std::endl;
	output << "	Idle -> ACCEPT :: Null -> [0]output;" << std::endl;
	output << "	Idle -> REJECT :: Null -> [1]output;" << std::endl;
	output << "	Idle -> DROP :: Null -> [2]output;" << std::endl;
	output << std::endl;
	output << "	//Linking of the Iptables rules" << std::endl;
	if (trace)
		_mangleTable->printClickTraceSimulation(output, "MANGLE_");
	else
		_mangleTable->printClickSimulation(output, "MANGLE_");
	output << "}" << std::endl;
	output << std::endl;

	output << "elementclass NatTable {" << std::endl;
	output << "	//Backtracker declaration" << std::endl;
	output << "	Idle -> bt :: Backtracker;" << std::endl;
	output << std::endl;
	_networkLayout->printMasqueradeSwitch(output);
	output << "	//Translation of the Iptables rules to click syntax" << std::endl;
	_natTable->printClickClassifiers(output, "NAT_");
	output << std::endl;
	if (trace) {
		output << "	//Pretty printers" << std::endl;
		_natTable->printPrettyPrinters(output, "NAT_");
		output << std::endl;
	}
	output << "	//Element input & output" << std::endl;
	output << "	input[0] -> ps :: PaintSwitch(ANNO 19);" << std::endl;
	output << "	ps[0] -> NAT_OUTPUT1" << (trace?"Print;":";") << std::endl;
	output << "	ps[1] -> NAT_POSTROUTING1" << (trace?"Print;":";") << std::endl;
	output << "	ps[2] -> NAT_PREROUTING1" << (trace?"Print;":";") << std::endl;
	output << "	Idle -> ACCEPT :: Null -> [0]output;" << std::endl;
	output << "	Idle -> REJECT :: Null -> [1]output;" << std::endl;
	output << "	Idle -> DROP :: Null -> [2]output;" << std::endl;
	output << std::endl;
	output << "	//Linking of the Iptables rules" << std::endl;
	if (trace)
		_natTable->printClickTraceSimulation(output, "NAT_");
	else
		_natTable->printClickSimulation(output, "NAT_");
	output << "}" << std::endl;
	output << std::endl;

	output << "elementclass FilterTable {" << std::endl;
	output << "	//Backtracker declaration" << std::endl;
	output << "	Idle -> bt :: Backtracker;" << std::endl;
	output << std::endl;
	output << "	//Translation of the Iptables rules to click syntax" << std::endl;
	_filterTable->printClickClassifiers(output, "FILTER_");
	output << std::endl;
	if (trace) {
		output << "	//Pretty printers" << std::endl;
		_filterTable->printPrettyPrinters(output, "FILTER_");
		output << std::endl;
	}
	output << "	input[0] -> ps :: PaintSwitch(ANNO 19);" << std::endl;
	output << "	ps[0] -> FILTER_FORWARD1" << (trace?"Print;":";") << std::endl;
	output << "	ps[1] -> FILTER_INPUT1" << (trace?"Print;":";") << std::endl;
	output << "	ps[2] -> FILTER_OUTPUT1" << (trace?"Print;":";") << std::endl;
	output << "	Idle -> ACCEPT :: Null -> [0]output;" << std::endl;
	output << "	Idle -> REJECT :: Null -> [1]output;" << std::endl;
	output << "	Idle -> DROP :: Null -> [2]output;" << std::endl;
	output << std::endl;
	output << "	//Linking of the Iptables rules" << std::endl;
	if (trace)
		_filterTable->printClickTraceSimulation(output, "FILTER_");
	else
		_filterTable->printClickSimulation(output, "FILTER_");
	output << "}" << std::endl;
	output << std::endl;
}

void ClickGenerator::printTrafficTypes(std::ostream& output, bool trace) {
	output << "elementclass ForwardTraffic {" << std::endl;
	output << "	input[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"Entering the Forward chain of the Mangle table.\")":"")
		<< "-> Paint(ANNO 19, COLOR 0) -> mangle_forward :: MangleTable;" << std::endl;
	output << "	mangle_forward[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"Entering the Forward chain of the Filter table.\")":"")
		<< "-> Paint(ANNO 19, COLOR 0) -> filter_forward :: FilterTable;" << std::endl;
	output << "	mangle_forward[1] -> [1]output;" << std::endl;
	output << "	mangle_forward[2] -> [2]output;" << std::endl;
	output << "	filter_forward[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"Entering the Postrouting chain of the Mangle table.\")":"")
		<< "-> Paint(ANNO 19, COLOR 3) -> mangle_postrouting :: MangleTable;" << std::endl;
	output << "	filter_forward[1] -> [1]output;" << std::endl;
	output << "	filter_forward[2] -> [2]output;" << std::endl;
	output << "	mangle_postrouting[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"Entering the Postrouting chain of the Nat table.\")":"")
		<< "-> Paint(ANNO 19, COLOR 1) -> nat_postrouting :: NatTable;" << std::endl;
	output << "	mangle_postrouting[1] -> [1]output;" << std::endl;
	output << "	mangle_postrouting[2] -> [2]output;" << std::endl;
	output << "	nat_postrouting[0] -> [0]output;" << std::endl;
	output << "	nat_postrouting[1] -> [1]output;" << std::endl;
	output << "	nat_postrouting[2] -> [2]output;" << std::endl;
	output << "}" << std::endl;
	output << std::endl;

	output << "elementclass InputTraffic {" << std::endl;
	output << "	input[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"Entering the Input chain of the Mangle table.\")":"")
		<< "-> Paint(ANNO 19, COLOR 1) -> mangle_input :: MangleTable;" << std::endl;
	output << "	mangle_input[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"Entering the Input chain of the Filter table.\")":"")
		<< "-> Paint(ANNO 19, COLOR 1) -> filter_input :: FilterTable;" << std::endl;
	output << "	mangle_input[1] -> [1]output;" << std::endl;
	output << "	mangle_input[2] -> [2]output;" << std::endl;
	output << "	filter_input[0] -> [0]output;" << std::endl;
	output << "	filter_input[1] -> [1]output;" << std::endl;
	output << "	filter_input[2] -> [2]output;" << std::endl;
	output << "}" << std::endl;
	output << std::endl;

	output << "elementclass OutputTraffic {" << std::endl;
	output << "	input[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"Entering the Output chain of the Mangle table.\")":"")
		<< " -> Paint(ANNO 19, COLOR 2) -> mangle_output :: MangleTable;" << std::endl;
	output << "	mangle_output[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"Entering the Output chain of the Nat table.\")":"")
		<< "-> Paint(ANNO 19, COLOR 0) -> nat_output :: NatTable;" << std::endl;
	output << "	mangle_output[1] -> [1]output;" << std::endl;
	output << "	mangle_output[2] -> [2]output;" << std::endl;
	output << "	nat_output[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"Entering the Output chain of the Filter table.\")":"")
		<< " -> Paint(ANNO 19, COLOR 2) -> filter_output :: FilterTable;" << std::endl;
	output << "	nat_output[1] -> [1]output;" << std::endl;
	output << "	nat_output[2] -> [2]output;" << std::endl;
	output << "	filter_output[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"Entering the Postrouting chain of the Mangle table.\")":"")
		<< " -> Paint(ANNO 19, COLOR 3) -> mangle_postrouting :: MangleTable;" << std::endl;
	output << "	filter_output[1] -> [1]output;" << std::endl;
	output << "	filter_output[2] -> [2]output;" << std::endl;
	output << "	mangle_postrouting[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"Entering the Postrouting chain of the Nat table.\")":"")
		<< " -> Paint(ANNO 19, COLOR 1) -> nat_postrouting :: NatTable;" << std::endl;
	output << "	mangle_postrouting[1] -> [1]output;" << std::endl;
	output << "	mangle_postrouting[2] -> [2]output;" << std::endl;
	output << "	nat_postrouting[0] -> [0]output;" << std::endl;
	output << "	nat_postrouting[1] -> [1]output;" << std::endl;
	output << "	nat_postrouting[2] -> [2]output;" << std::endl;
	output << "}" << std::endl;
	output << std::endl;

	output << "elementclass ArrivingTraffic {" << std::endl;
	output << "	input[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"Entering the Prerouting chain of the Mangle table.\")":"")
		<< " -> Paint(ANNO 19, COLOR 4) -> mangle_prerouting :: MangleTable;" << std::endl;
	output << "	mangle_prerouting[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"Entering the Prerouting chain of the Nat table.\")":"")
		<< "-> Paint(ANNO 19, COLOR 2) -> nat_prerouting :: NatTable;" << std::endl;
	output << "	mangle_prerouting[1] -> [1]output;" << std::endl;
	output << "	mangle_prerouting[2] -> [2]output;" << std::endl;
	output << "	nat_prerouting[0] -> ";
	_networkLayout->printTrafficTypeClassifier(output, "in_or_fwd", false);
	output << "	nat_prerouting[1] -> [1]output;" << std::endl;
	output << "	nat_prerouting[2] -> [2]output;" << std::endl;
	output << "	in_or_fwd[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"Following the INPUT traffic chains.\")":"")
		<< " -> in :: InputTraffic;" << std::endl;
	output << "	in_or_fwd[1]"
		<< (trace?" -> Script(TYPE PACKET, print \"Following the FORWARD traffic chains.\")":"")
		<< " -> fwd :: ForwardTraffic;" << std::endl;
	output << "	in[0] -> [0]output;" << std::endl;
	output << "	in[1] -> [1]output;" << std::endl;
	output << "	in[2] -> [2]output;" << std::endl;
	output << "	fwd[0] -> [0]output;" << std::endl;
	output << "	fwd[1] -> [1]output;" << std::endl;
	output << "	fwd[2] -> [2]output;" << std::endl;
	output << "}" << std::endl;
	output << std::endl;

	output << "elementclass OutgoingTraffic {" << std::endl;
	output << "	input[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"Following the OUTPUT traffic chains.\")":"")
		<< " -> out :: OutputTraffic;" << std::endl;
	output << "	out[0] -> ";
	_networkLayout->printTrafficTypeClassifier(output, "local_or_out", false);
	output << "	out[1] -> [1]output;" << std::endl;
	output << "	out[2] -> [2]output;" << std::endl;
	output << "	local_or_out[0] -> arriving :: ArrivingTraffic;" << std::endl;
	output << "	local_or_out[1] -> [0]output;" << std::endl;
	output << "	arriving[0] -> [0]output;" << std::endl;
	output << "	arriving[1] -> [1]output;" << std::endl;
	output << "	arriving[2] -> [2]output;" << std::endl;
	output << "}" << std::endl;
	output << std::endl;
}

void ClickGenerator::printTrafficSwitch(std::ostream& output, bool trace) {
	output << "//Traffic type check" << std::endl;
	_networkLayout->printTrafficTypeClassifier(output, "trafficType", true);
	output << std::endl;
	output << "//Output or \"local\" traffic" << std::endl;
	output << "trafficType[0]"
		<< (trace?" -> Script(TYPE PACKET, print \"This packet is classified as outgoing traffic (OUTPUT or \"local\".\")":"")
		<< " -> outgoing :: OutgoingTraffic;" << std::endl;
	output << "outgoing[0] -> ACCEPT;" << std::endl;
	output << "outgoing[1] -> REJECT;" << std::endl;
	output << "outgoing[2] -> DROP;" << std::endl;
	output << std::endl;
	output << "//Input or forward traffic" << std::endl;
	output << "trafficType[1]"
		<< (trace?" -> Script(TYPE PACKET, print \"This packet is classified as arriving traffic (INPUT or FORWARD).\")":"")
		<< " -> arriving :: ArrivingTraffic;" << std::endl;
	output << "arriving[0] -> ACCEPT;" << std::endl;
	output << "arriving[1] -> REJECT;" << std::endl;
	output << "arriving[2] -> DROP;" << std::endl;
	output << std::endl;
}

void ClickGenerator::generateSimulation(std::ostream& output) {
	output << "//Please don't alter the content of this file generated by FireSim" << std::endl;
	output << "//authors: Nico Van Looy (2008-2009) & Jens De Wit (2008-2010)" << std::endl;
	output << std::endl;

	output << "//Wait for all traffic to end, then collect statistics and shut down" << std::endl;
	output << "DriverManager(";
	for (int i = 1; i <= _numTrafficBlocks; i++) {
		output << "pause, ";
		if (i % 5 == 0)
			output << std::endl << "\t";
	}
	output << std::endl;
	output << "	print >" << STATISTICS_FILENAME << " inputCounter.count," << std::endl;
	output << "	print >>" << STATISTICS_FILENAME << " ACCEPT_true.count," << std::endl;
	output << "	print >>" << STATISTICS_FILENAME << " ACCEPT_false.count," << std::endl;
	output << "	print >>" << STATISTICS_FILENAME << " REJECT_true.count," << std::endl;
	output << "	print >>" << STATISTICS_FILENAME << " REJECT_false.count," << std::endl;
	output << "	print >>" << STATISTICS_FILENAME << " DROP_true.count," << std::endl;
	output << "	print >>" << STATISTICS_FILENAME << " DROP_false.count," << std::endl;
	output << "	stop);" << std::endl;
	output << std::endl;

	printTables(output, false);

	printTrafficTypes(output, false);

	output << "//Painters for statistics" << std::endl;
	output << "Idle -> ACCEPT :: CheckPaint(COLOR 0);" << std::endl;
	output << "Idle -> REJECT :: CheckPaint(COLOR 1);" << std::endl;
	output << "Idle -> DROP :: CheckPaint(COLOR 2);" << std::endl;
	output << std::endl;

	printTrafficSwitch(output, false);

	output << "//Counters for statistics" << std::endl;
	output << "inputCounter :: Counter -> trafficType;" << std::endl;
	output << "ACCEPT_true :: Counter;" << std::endl;
	output << "ACCEPT_false :: Counter;" << std::endl;
	output << "REJECT_true :: Counter;" << std::endl;
	output << "REJECT_false :: Counter;" << std::endl;
	output << "DROP_true :: Counter;" << std::endl;
	output << "DROP_false :: Counter;" << std::endl;
	output << std::endl;

	output << "//Traffic copy" << std::endl;
	output << "copy :: CopyBuffer;" << std::endl;
	output << std::endl;

	output << "//Output switch" << std::endl;
	output << "output :: Switch;" << std::endl;
	output << "output[0] -> ToDump(\"output/faulty_accept.dump\");" << std::endl;
	output << "output[1] -> ToDump(\"output/faulty_reject.dump\");" << std::endl;
	output << "output[2] ->ToDump(\"output/faulty_drop.dump\");" << std::endl;
	output << std::endl;

	output << "//Do simulation with input traffic" << std::endl;
	output << "copy[0] -> MarkIPHeader(14)" << std::endl;
	output << "	-> inputCounter;" << std::endl;
	output << std::endl;

	output << "//Send faulty packet to correct dump" << std::endl;
	output << "copy[1] -> output;" << std::endl;
	output << std::endl;

	output << "//Feedback" << std::endl;
	output << "ACCEPT[0] -> ACCEPT_true -> Script(TYPE PACKET, write output.switch -1) -> [1]copy" << std::endl;
	output << "ACCEPT[1] -> ACCEPT_false -> Script(TYPE PACKET, write output.switch 0) -> [1]copy" << std::endl;
	output << "REJECT[0] -> REJECT_true -> Script(TYPE PACKET, write output.switch -1) -> [1]copy" << std::endl;
	output << "REJECT[1] -> REJECT_false -> Script(TYPE PACKET, write output.switch 1) -> [1]copy" << std::endl;
	output << "DROP[0] -> DROP_true -> Script(TYPE PACKET, write output.switch -1) -> [1]copy" << std::endl;
	output << "DROP[1] -> DROP_false -> Script(TYPE PACKET, write output.switch 2) -> [1]copy" << std::endl;
	output << std::endl;

	_configParser->printClickTraffic(output, _networkLayout);
}

void ClickGenerator::generateTraces(std::ostream& output) {
	assert(_filterTable);
	assert(_natTable);
	assert(_mangleTable);

	output << "//Please don't alter the content of this file generated by FireSim" << std::endl;
	output << "//authors: Nico Van Looy (2008-2009) & Jens De Wit (2008-2010)" << std::endl;
	output << std::endl;

	printTables(output, true);

	printTrafficTypes(output, true);

	output << "//Policies" << std::endl;
	output << "Idle -> ACCEPT :: Discard;" << std::endl;
	output << "Idle -> REJECT :: Discard;" << std::endl;
	output << "Idle -> DROP :: Discard;" << std::endl;
	output << std::endl;

	printTrafficSwitch(output, true);

	output << "// Replay specific dump file" << std::endl;
	output << "FromDump($FILENAME, STOP true)" << std::endl;
	output << "	-> MarkIPHeader(14)" << std::endl;
	output << "	-> counter :: Counter" << std::endl;
	output << "	-> Script(TYPE PACKET," << std::endl;
	output << "		set packetnr $(counter.count)," << std::endl;
	output << "		print \"\nPacket\" $packetnr)" << std::endl;
	output << "	-> trafficType;" << std::endl;
	output << std::endl;
}
