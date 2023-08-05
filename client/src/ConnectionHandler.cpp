#include "../include/ConnectionHandler.h"

using boost::asio::ip::tcp;

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

ConnectionHandler::ConnectionHandler(string host, short port) : host_(host), port_(port), io_service_(),
	socket_(io_service_), connected(false), loggedIn(false), name("") {}
																
ConnectionHandler::ConnectionHandler() : host_("0"), port_(0), io_service_(),
	socket_(io_service_), connected(false), loggedIn(false), name("") {}

ConnectionHandler::~ConnectionHandler() {
	close();
}



bool ConnectionHandler::connect() {
	std::cout << "Connecting to " << host_ << ":" << port_ << std::endl;
	try {
		tcp::endpoint endpoint(boost::asio::ip::address::from_string(host_), port_); // the server endpoint
		boost::system::error_code error;
		socket_.connect(endpoint, error);
		if (error)
			throw boost::system::system_error(error);
		else{
			connected = true;
		}
	}
	catch (std::exception &e) {
		std::cerr << "Connection failed (Error: " << e.what() << ')' << std::endl;
		return false;
	}
	std::cout << "Connected" << std::endl;
	return true;
}

bool ConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead) {
	size_t tmp = 0;
	boost::system::error_code error;
	try {
		while (!error && bytesToRead > tmp) {
			tmp += socket_.read_some(boost::asio::buffer(bytes + tmp, bytesToRead - tmp), error);
		}
		if (error)
			throw boost::system::system_error(error);
	} catch (std::exception &e) {
		std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
		return false;
	}
	return true;
}

bool ConnectionHandler::sendBytes(const char bytes[], int bytesToWrite) {
	int tmp = 0;
	boost::system::error_code error;
	try {
		while (!error && bytesToWrite > tmp) {
			tmp += socket_.write_some(boost::asio::buffer(bytes + tmp, bytesToWrite - tmp), error);
		}
		if (error)
			throw boost::system::system_error(error);
	} catch (std::exception &e) {
		std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
		return false;
	}
	return true;
}

bool ConnectionHandler::getLine(std::string &line) {
	return getFrameAscii(line, '\n');
}

bool ConnectionHandler::sendLine(std::string &line) {
	return sendFrameAscii(line, '\n');
}


bool ConnectionHandler::getFrameAscii(std::string &frame, char delimiter) {
	char ch;
	// Stop when we encounter the null character.
	// Notice that the null character is not appended to the frame string.
	try {
		do {
			if (!getBytes(&ch, 1)) {
				return false;
			}
			if (ch != '\0')
				frame.append(1, ch);
		} while (delimiter != ch);
	} catch (std::exception &e) {
		std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
		return false;
	}
	return true;
}

bool ConnectionHandler::sendFrameAscii(const std::string &frame, char delimiter) {
	bool result = sendBytes(frame.c_str(), frame.length());
	if (!result) return false;
	return sendBytes(&delimiter, 1);
}

bool ConnectionHandler::sendFrame(const StompFrame& frame) {
	std::string str = frame.toString();
	return sendFrameAscii(str, '\0');
}

// Close down the connection properly.
void ConnectionHandler::close() {
	try {
		
		socket_.close();
	} catch (...) {
		std::cout << "closing failed: connection already closed" << std::endl;
	}
}

void ConnectionHandler::disconnect(){
	connected = false;
	setLoggedIn(false);
    close();
}

bool ConnectionHandler::isConnected() const{
	return connected;
}

bool ConnectionHandler::isLoggedIn() const{
	return loggedIn;
}

void ConnectionHandler::setHost(string h){
	this->host_ = std::move(h);
}

void ConnectionHandler::setPort(short p){
	this->port_ = p;
}

void ConnectionHandler::setLoggedIn(bool b){
    this->loggedIn = b;
}

void ConnectionHandler::setName(std::basic_string<char> &basicString) {
    name=basicString;
}

const std::string& ConnectionHandler::getName() const{
	return name;
}
