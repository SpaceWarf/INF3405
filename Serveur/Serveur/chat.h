#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <vector>


using namespace std;

class Chat {
private:
	static const string FILE_NAME;
	static const int MAX_N_MESSAGES;
	const char* _username;
	const char* _ip;
	const int _port;



public:
	//Constructor
	Chat(char* username, char* ip, int port);

	//Destructor
	~Chat() {};

	
	void addNewMessage(string msg);

	
	vector<string> getOldMessages();
	string formatMessage(string msg);

};