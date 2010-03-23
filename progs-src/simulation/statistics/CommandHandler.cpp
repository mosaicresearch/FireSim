/**
 * @file CommandHandler.cpp
 * @author Nico Van Looy (s0061909) & Jens De Wit (s0061864)
 * @brief Bachelor project of University of Antwerp - 2008-2009
 */

//Standard
#include <string>
#include <iostream>
#include <sstream>

//Network
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/NetException.h"
#include "Poco/StringTokenizer.h"

//Logger
#include "Poco/Logger.h"

//FireSim
#include "../util/Util.h"
#include "CommandHandler.h"

using std::string;
using std::endl;
using Poco::Logger;

CommandHandler::CommandHandler(std::string host, int port) {
	Logger::get("ConsoleLogger").debug("CommandHandler.cpp: Connecting to firewall...");
	//Connect to firewall
	bool connected = false;
	while (!connected) {
		try {
			Poco::Net::SocketAddress sa(host, port);
			_socket.connect(sa);
			Poco::Net::SocketStream socketStream(_socket);
			connected = true;
			Logger::get("ConsoleLogger").debug("CommandHandler.cpp: Connection to firewall established.");

			//Receive "Click::ControlSocket/1.1" this is always the first received line when the connection is established.
			string result;
			std::getline(socketStream, result);
			Logger::get("ConsoleLogger").debug("CommandHandler.cpp: " + result);

		} catch (Poco::Net::ConnectionRefusedException &e) {
			//can not yet connect
			Logger::get("ConsoleLogger").debug("CommandHandler.cpp: " + e.displayText());
		} //end try-catch
	} //end while
}

CommandHandler::~CommandHandler(){
	_socket.close();
}

std::string CommandHandler::execute(std::string clickCmd) {
	string clickReply = "";
	string result = "";
	int triesLeft = 3;
	do {
		Poco::Net::SocketStream socketStream(_socket);
		//Send command
		Logger::get("ConsoleLogger").debug("CommandHandler.cpp: send " + clickCmd + " to click simulation.");
		socketStream << clickCmd << endl;

		//Receive reply: 200 Read handler 'clickCmd' OK
		std::getline(socketStream, result);
		Logger::get("ConsoleLogger").debug("CommandHandler.cpp: " + result);

		//Receive reply: DATA 'number'
		std::getline(socketStream, result);
		Logger::get("ConsoleLogger").debug("CommandHandler.cpp: " + result);

		if (result != "") {
			Poco::StringTokenizer tokenizer(result, " ", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
			int outputLength = stringToInt(tokenizer[1]);
			Logger::get("ConsoleLogger").debug("CommandHandler.cpp: #chars in Click reply is " + std::string(tokenizer[1]));

			for (int i = 0; i < outputLength; i++) {
						char tmp;
						socketStream.get(tmp);
						clickReply.append(1, tmp);
					}
		}
	} while (result == "" && --triesLeft > 0);
	return clickReply;
}
