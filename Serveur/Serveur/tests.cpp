//Server
#undef UNICODE

#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <strstream>
#include <locale>
#include "chat.h"
#include <string>

using namespace std;

// link with Ws2_32.lib
#pragma comment( lib, "ws2_32.lib" )

//To make sure there are no errors
#pragma warning(disable : 4996)



int main(void)
{
	char* username = "Utilisateur 1";
	char* ip = "123.123.123.123";
	int port = 55555;

	Chat chat = Chat(username, ip, port);
	cout << chat.formatMessage("bonjour la police");

	cout << "entrez quelquechose pour terminer le programme" << endl;
	getchar();

	return 0;
}

