#include "chat.h"

using namespace std;

#pragma warning(disable : 4996)

const string Chat::FILE_NAME = "logs.txt";
const int MAX_N_MESSAGES = 15;

//Constructor
Chat::Chat(char* username, char* ip, int port) :
	_username(username), _ip(ip), _port(port) 
{
	
}

//Add a new message to the chat file
void Chat::addNewMessage(string msg)
{
	ofstream outFile("logs/logs.txt", ios_base::app);
	outFile << msg << "\n";
}

int getFileLength(string file) {
	ifstream inFile(file);
	string line;
	int nLines = 0;

	while (getline(inFile, line))
		++nLines;

	return nLines;
}

//Returns all the old 15 messages (or less than 15)
vector<string> Chat::getOldMessages()
{
	int nLines = getFileLength("logs/logs.txt");
	string line;
	vector<string> msgs;
	ifstream inFile("logs/logs.txt");

	for (int i = 0; i < nLines && getline(inFile, line); ++i) {
		if (i >= nLines - 15) {
			msgs.push_back(line);
		}
	}
		
	return msgs;
}
//Returns the full fomrmatted str message 
string Chat::formatMessage(string msg)
{
	time_t now = time(0);
	char* dt = ctime(&now);

	string str_time(dt);
	str_time = str_time.substr(0, str_time.size() - 1); // remove char de fin de ligne a la fin qui est mis auto

	string str_username(_username);
	string str_ip(_ip);

	string str = "[" + str_username + " - " + str_ip + ":" + to_string(_port) + " - " + str_time + "]: " + msg + "\n";
	
	return str;
}