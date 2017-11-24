#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>


using namespace std;

class Chat {
private:
	static const string FILE_NAME;
	static const int MAX_N_MESSAGES;

	//static int _n_messages; //probleme: le programme doit le savoir meme s'il se ferme (persistent data) 

	const char* _username;
	const char* _ip;
	const int _port;



public:
	//Constructor
	Chat(char* username, char* ip, int port);

	//Destructor
	~Chat() {};

	
	void addNewMessage(char* msg);

	
	char** getOldMessages();
	string formatMessage(string msg);

};