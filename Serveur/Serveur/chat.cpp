#include "chat.h"

using namespace std;

#pragma warning(disable : 4996)

const string Chat::FILE_NAME = "chat.txt";
const int MAX_N_MESSAGES = 15;

//Constructor
Chat::Chat(char* username, char* ip, int port) :
	_username(username), _ip(ip), _port(port) 
{
	
}

//Add a new message to the chat file
void Chat::addNewMessage(char* msg)
{

}

//Returns all the old 15 messages (or less than 15)
char** Chat::getOldMessages()
{
	char* results[15];
	for (int i = 0; i < 15; i++) {
		char* msg = "iqutrye"; //Aller chercher les 15 derniers messages
		results[i] = msg;
	}
	return results;
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