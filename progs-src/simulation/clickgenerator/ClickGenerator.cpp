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

ClickGenerator::ClickGenerator(std::string config_path, bool testRun) {
	_filterTable = 0;
	_natTable = 0;
	_mangleTable = 0;
	_networkLayout = 0;
	_configParser = 0;

	std::string config_path2 = config_path;
	if (_testRun = testRun)
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
		std::cout << "Failure while parsing network_layout.xml file at line " << e.getLineNumber()
			<< " and column " << e.getColumnNumber() << ": " << e.name() << std::endl;
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
		std::cout << "Failure while parsing config.xml file at line " << e.getLineNumber()
			<< " and column " << e.getColumnNumber() << ": " << e.name() << std::endl;
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

void ClickGenerator::printTables(std::ostream& output) {
	assert(_filterTable);
	assert(_natTable);
	assert(_mangleTable);

	output << "elementclass MangleTable {" << std::endl;
	output << "	//Backtracker declaration" << std::endl;
	output << "	Idle -> bt :: Backtracker;" << std::endl;
	output << std::endl;
	output << "	//Translation of the Iptables rules to click syntax" << std::endl;
	_mangleTable->printClickClassifiers(output);
	output << std::endl;
	output << "	//Element input & output" << std::endl;
	output << "	input[0] -> ps :: PaintSwitch(ANNO 19);" << std::endl;
	output << "	ps[0] -> FORWARD1;" << std::endl;
	output << "	ps[1] -> INPUT1;" << std::endl;
	output << "	ps[2] -> OUTPUT1;" << std::endl;
	output << "	ps[3] -> POSTROUTING1;" << std::endl;
	output << "	ps[4] -> PREROUTING1;" << std::endl;
	output << "	Idle -> ACCEPT :: Null -> [0]output;" << std::endl;
	output << "	Idle -> REJECT :: Null -> [1]output;" << std::endl;
	output << "	Idle -> DROP :: Null -> [2]output;" << std::endl;
	output << std::endl;
	output << "	//Linking of the Iptables rules" << std::endl;
	_mangleTable->printClickSimulation(output);
	output << "}" << std::endl;
	output << std::endl;

	output << "elementclass NatTable {" << std::endl;
	output << "	//Backtracker declaration" << std::endl;
	output << "	Idle -> bt :: Backtracker;" << std::endl;
	output << std::endl;
	_networkLayout->printMasqueradeSwitch(output);
	output << "	//Translation of the Iptables rules to click syntax" << std::endl;
	_natTable->printClickClassifiers(output);
	output << std::endl;
	output << "	//Element input & output" << std::endl;
	output << "	input[0] -> ps :: PaintSwitch(ANNO 19);" << std::endl;
	output << "	ps[0] -> OUTPUT1;" << std::endl;
	output << "	ps[1] -> POSTROUTING1;" << std::endl;
	output << "	ps[2] -> PREROUTING1;" << std::endl;
	output << "	Idle -> ACCEPT :: Null -> [0]output;" << std::endl;
	output << "	Idle -> REJECT :: Null -> [1]output;" << std::endl;
	output << "	Idle -> DROP :: Null -> [2]output;" << std::endl;
	output << std::endl;
	output << "	//Linking of the Iptables rules" << std::endl;
	_natTable->printClickSimulation(output);
	output << "}" << std::endl;
	output << std::endl;

	output << "elementclass FilterTable {" << std::endl;
	output << "	//Backtracker declaration" << std::endl;
	output << "	Idle -> bt :: Backtracker;" << std::endl;
	output << std::endl;
	output << "	//Translation of the Iptables rules to click syntax" << std::endl;
	_filterTable->printClickClassifiers(output);
	output << std::endl;
	output << "	input[0] -> ps :: PaintSwitch(ANNO 19);" << std::endl;
	output << "	ps[0] -> FORWARD1;" << std::endl;
	output << "	ps[1] -> INPUT1;" << std::endl;
	output << "	ps[2] -> OUTPUT1;" << std::endl;
	output << "	Idle -> ACCEPT :: Null -> [0]output;" << std::endl;
	output << "	Idle -> REJECT :: Null -> [1]output;" << std::endl;
	output << "	Idle -> DROP :: Null -> [2]output;" << std::endl;
	output << std::endl;
	output << "	//Linking of the Iptables rules" << std::endl;
	_filterTable->printClickSimulation(output);
	output << "}" << std::endl;
	output << std::endl;
}

void ClickGenerator::printTrafficTypes(std::ostream& output) {
	output << "elementclass ForwardTraffic {" << std::endl;
	output << "	input[0]"
		<< " -> InfoPainter(\"Entering the Forward chain of the Mangle table.\")"
		<< " -> Paint(ANNO 19, COLOR 0) -> mangle_forward :: MangleTable;" << std::endl;
	output << "	mangle_forward[0]"
		<< " -> InfoPainter(\"Entering the Forward chain of the Filter table.\")"
		<< " -> Paint(ANNO 19, COLOR 0) -> filter_forward :: FilterTable;" << std::endl;
	output << "	mangle_forward[1] -> [1]output;" << std::endl;
	output << "	mangle_forward[2] -> [2]output;" << std::endl;
	output << "	filter_forward[0]"
		<< " -> InfoPainter(\"Entering the Postrouting chain of the Mangle table.\")"
		<< " -> Paint(ANNO 19, COLOR 3) -> mangle_postrouting :: MangleTable;" << std::endl;
	output << "	filter_forward[1] -> [1]output;" << std::endl;
	output << "	filter_forward[2] -> [2]output;" << std::endl;
	output << "	mangle_postrouting[0] -> newConnection :: CheckPaint(ANNO 17, COLOR 0)" << std::endl;
	output << "	mangle_postrouting[1] -> [1]output;" << std::endl;
	output << "	mangle_postrouting[2] -> [2]output;" << std::endl;
	output << "	newConnection[0] -> InfoPainter(\"Entering the Postrouting chain of the Nat table.\")"
		<< " -> Paint(ANNO 19, COLOR 1) -> nat_postrouting :: NatTable;" << std::endl;
	output << "	newConnection[1] -> StaticNat(TYPE src)"
		<< " -> InfoPainter(\"Skipping the Postrouting chain of the Nat table.\") -> [0]output;";
	output << "	nat_postrouting[0] -> [0]output;" << std::endl;
	output << "	nat_postrouting[1] -> [1]output;" << std::endl;
	output << "	nat_postrouting[2] -> [2]output;" << std::endl;
	output << "}" << std::endl;
	output << std::endl;

	output << "elementclass InputTraffic {" << std::endl;
	output << "	input[0]"
		<< " -> InfoPainter(\"Entering the Input chain of the Mangle table.\")"
		<< " -> Paint(ANNO 19, COLOR 1) -> mangle_input :: MangleTable;" << std::endl;
	output << "	mangle_input[0]"
		<< " -> InfoPainter(\"Entering the Input chain of the Filter table.\")"
		<< " -> Paint(ANNO 19, COLOR 1) -> filter_input :: FilterTable;" << std::endl;
	output << "	mangle_input[1] -> [1]output;" << std::endl;
	output << "	mangle_input[2] -> [2]output;" << std::endl;
	output << "	filter_input[0] -> [0]output;" << std::endl;
	output << "	filter_input[1] -> [1]output;" << std::endl;
	output << "	filter_input[2] -> [2]output;" << std::endl;
	output << "}" << std::endl;
	output << std::endl;

	output << "elementclass OutputTraffic {" << std::endl;
	output << "	input[0]"
		<< " -> InfoPainter(\"Entering the Output chain of the Mangle table.\")"
		<< " -> Paint(ANNO 19, COLOR 2) -> mangle_output :: MangleTable;" << std::endl;
	output << "	mangle_output[0] -> newConnection :: CheckPaint(ANNO 17, COLOR 0)" << std::endl;
	output << "	mangle_output[1] -> [1]output;" << std::endl;
	output << "	mangle_output[2] -> [2]output;" << std::endl;
	output << "	newConnection[0] -> InfoPainter(\"Entering the Output chain of the Nat table.\")"
		<< " -> Paint(ANNO 19, COLOR 0) -> nat_output :: NatTable;" << std::endl;
	output << "	newConnection[1] -> StaticNat(TYPE dst) -> InfoPainter(\"Skipping the Output chain of the Nat table.\")"
		<< " -> toFilter :: InfoPainter(\"Entering the Output chain of the Filter table.\")"
		<< " -> Paint(ANNO 19, COLOR 2) -> filter_output :: FilterTable;" << std::endl;
	output << "	nat_output[0] -> toFilter" << std::endl;
	output << "	nat_output[1] -> [1]output;" << std::endl;
	output << "	nat_output[2] -> [2]output;" << std::endl;
	output << "	filter_output[0]"
		<< " -> InfoPainter(\"Entering the Postrouting chain of the Mangle table.\")"
		<< " -> Paint(ANNO 19, COLOR 3) -> mangle_postrouting :: MangleTable;" << std::endl;
	output << "	filter_output[1] -> [1]output;" << std::endl;
	output << "	filter_output[2] -> [2]output;" << std::endl;
	output << "	mangle_postrouting[0] -> newConnection2 :: CheckPaint(ANNO 17, COLOR 0)" << std::endl;
	output << "	mangle_postrouting[1] -> [1]output;" << std::endl;
	output << "	mangle_postrouting[2] -> [2]output;" << std::endl;
	output << "	newConnection2[0] -> InfoPainter(\"Entering the Postrouting chain of the Nat table.\")"
		<< " -> Paint(ANNO 19, COLOR 1) -> nat_postrouting :: NatTable;" << std::endl;
	output << "	newConnection2[1] -> StaticNat(TYPE src)"
		<< " -> InfoPainter(\"Skipping the Postrouting chain of the Nat table.\") -> [0]output;" << std::endl;
	output << "	nat_postrouting[0] -> [0]output;" << std::endl;
	output << "	nat_postrouting[1] -> [1]output;" << std::endl;
	output << "	nat_postrouting[2] -> [2]output;" << std::endl;
	output << "}" << std::endl;
	output << std::endl;

	output << "elementclass ArrivingTraffic {" << std::endl;
	output << "	input[0]"
		<< " -> InfoPainter(\"Entering the Prerouting chain of the Mangle table.\")"
		<< " -> Paint(ANNO 19, COLOR 4) -> mangle_prerouting :: MangleTable;" << std::endl;
	output << "	mangle_prerouting[0] -> newConnection :: CheckPaint(ANNO 17, COLOR 0)" << std::endl;
	output << "	mangle_prerouting[1] -> [1]output;" << std::endl;
	output << "	mangle_prerouting[2] -> [2]output;" << std::endl;
	output << "	newConnection[0] -> InfoPainter(\"Entering the Prerouting chain of the Nat table.\")"
		<< " -> Paint(ANNO 19, COLOR 2) -> nat_prerouting :: NatTable;" << std::endl;
	output << "	newConnection[1] -> StaticNat(TYPE dst)"
		<< " -> InfoPainter(\"Skipping the Prerouting chain of the Nat table.\") -> ";
	_networkLayout->printTrafficTypeClassifier(output, "in_or_fwd", false);
	output << "	nat_prerouting[0] -> in_or_fwd";
	output << "	nat_prerouting[1] -> [1]output;" << std::endl;
	output << "	nat_prerouting[2] -> [2]output;" << std::endl;
	output << "	in_or_fwd[0]"
		<< " -> InfoPainter(\"Following the INPUT traffic chains.\")"
		<< " -> in :: InputTraffic;" << std::endl;
	output << "	in_or_fwd[1]"
		<< " -> InfoPainter(\"Following the FORWARD traffic chains.\")"
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
		<< " -> InfoPainter(\"Following the OUTPUT traffic chains.\")"
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

void ClickGenerator::printTrafficSwitch(std::ostream& output) {
	output << "//Traffic type check" << std::endl;
	_networkLayout->printTrafficTypeClassifier(output, "trafficType", true);
	output << std::endl;
	output << "//Output or \"local\" traffic" << std::endl;
	output << "trafficType[0]"
		<< " -> InfoPainter(\"This packet is classified as outgoing traffic (OUTPUT or \"local\").\")"
		<< " -> outgoing :: OutgoingTraffic;" << std::endl;
	output << "outgoing[0] -> ACCEPT;" << std::endl;
	output << "outgoing[1] -> REJECT;" << std::endl;
	output << "outgoing[2] -> DROP;" << std::endl;
	output << std::endl;
	output << "//Input or forward traffic" << std::endl;
	output << "trafficType[1]"
		<< " -> InfoPainter(\"This packet is classified as arriving traffic (INPUT or FORWARD).\")"
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
	output << "	printn >" << OUTPUT_PATH + UNUSED_RULES_FILENAME << " rulesPassedCounter.unused, " << std::endl;
	output << "	stop);" << std::endl;
	output << std::endl;

	output << "//Counter to track which rules are actually used" << std::endl;
	output << "rulesPassedCounter :: StaticCounter;" << std::endl;
	output << std::endl;

	printTables(output);

	printTrafficTypes(output);

	output << "//Painters for statistics" << std::endl;
	output << "Idle -> ACCEPT :: CheckPaint(COLOR 0);" << std::endl;
	output << "Idle -> REJECT :: CheckPaint(COLOR 1);" << std::endl;
	output << "Idle -> DROP :: CheckPaint(COLOR 2);" << std::endl;
	output << std::endl;

	printTrafficSwitch(output);

	output << "//Counters for statistics" << std::endl;
	output << "inputCounter :: Counter;" << std::endl;
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
	output << "output[0] -> InfoPrinter(\"" << (_testRun?"":"output/faulty_accept.txt")
		<< "\") -> ToDump(\"output/faulty_accept.dump\");" << std::endl;
	output << "output[1] -> InfoPrinter(\"" << (_testRun?"":"output/faulty_reject.txt")
		<< "\") -> ToDump(\"output/faulty_reject.dump\");" << std::endl;
	output << "output[2] -> InfoPrinter(\"" << (_testRun?"":"output/faulty_drop.txt")
		<< "\") -> ToDump(\"output/faulty_drop.dump\");" << std::endl;
	output << std::endl;

	output << "//Do simulation with input traffic" << std::endl;
	output << "copy[0] -> MarkIPHeader(14)" << std::endl;
	output << "	-> inputCounter" << std::endl;
	output << "	-> InfoPainter(\"PACKET \", inputCounter.count) " << std::endl;
	output << "	-> StateMachine(ANNO 17)" << std::endl;
	output << "	-> trafficType;" << std::endl;
	output << std::endl;

	output << "//Send faulty packet to correct dump" << std::endl;
	output << "copy[1] -> InfoPainter(\"\") -> output;" << std::endl;
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
