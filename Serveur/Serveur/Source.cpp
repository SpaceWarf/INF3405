//Server
#undef UNICODE

#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <strstream>
#include <locale>
#include <list>

#include <string>
#include "chat.h"
#include <list>


using namespace std;

// link with Ws2_32.lib
#pragma comment( lib, "ws2_32.lib" )

//To make sure there are no errors
#pragma warning(disable : 4996)

// External functions
extern DWORD WINAPI ConnectionHandler(void *sd_);
extern int Authenticate(void *sd_, char *username, char *password);

//Liste globale des sockets connect�es
list<SOCKET> Connections;

void broadcast(string FormattedMsg) {

	sockaddr_in socket_info = { 0 };
	int sl = sizeof(socket_info);

	for (std::list<SOCKET>::iterator i = Connections.begin(); i != Connections.end(); i++)
	{
		SOCKET sd = (*i);
		getpeername(sd, (sockaddr*)&socket_info, &sl);
		send(sd, FormattedMsg.c_str(), 300, 0);
	}	
}


// List of Winsock error constants mapped to an interpretation string.
// Note that this list must remain sorted by the error constants'
// values, because we do a binary search on the list when looking up
// items.
static struct ErrorEntry {
	int nID;
	const char* pcMessage;

	ErrorEntry(int id, const char* pc = 0) :
		nID(id),
		pcMessage(pc)
	{
	}

	bool operator<(const ErrorEntry& rhs) const
	{
		return nID < rhs.nID;
	}
} gaErrorList[] = {
	ErrorEntry(0,                  "No error"),
	ErrorEntry(WSAEINTR,           "Interrupted system call"),
	ErrorEntry(WSAEBADF,           "Bad file number"),
	ErrorEntry(WSAEACCES,          "Permission denied"),
	ErrorEntry(WSAEFAULT,          "Bad address"),
	ErrorEntry(WSAEINVAL,          "Invalid argument"),
	ErrorEntry(WSAEMFILE,          "Too many open sockets"),
	ErrorEntry(WSAEWOULDBLOCK,     "Operation would block"),
	ErrorEntry(WSAEINPROGRESS,     "Operation now in progress"),
	ErrorEntry(WSAEALREADY,        "Operation already in progress"),
	ErrorEntry(WSAENOTSOCK,        "Socket operation on non-socket"),
	ErrorEntry(WSAEDESTADDRREQ,    "Destination address required"),
	ErrorEntry(WSAEMSGSIZE,        "Message too long"),
	ErrorEntry(WSAEPROTOTYPE,      "Protocol wrong type for socket"),
	ErrorEntry(WSAENOPROTOOPT,     "Bad protocol option"),
	ErrorEntry(WSAEPROTONOSUPPORT, "Protocol not supported"),
	ErrorEntry(WSAESOCKTNOSUPPORT, "Socket type not supported"),
	ErrorEntry(WSAEOPNOTSUPP,      "Operation not supported on socket"),
	ErrorEntry(WSAEPFNOSUPPORT,    "Protocol family not supported"),
	ErrorEntry(WSAEAFNOSUPPORT,    "Address family not supported"),
	ErrorEntry(WSAEADDRINUSE,      "Address already in use"),
	ErrorEntry(WSAEADDRNOTAVAIL,   "Can't assign requested address"),
	ErrorEntry(WSAENETDOWN,        "Network is down"),
	ErrorEntry(WSAENETUNREACH,     "Network is unreachable"),
	ErrorEntry(WSAENETRESET,       "Net connection reset"),
	ErrorEntry(WSAECONNABORTED,    "Software caused connection abort"),
	ErrorEntry(WSAECONNRESET,      "Connection reset by peer"),
	ErrorEntry(WSAENOBUFS,         "No buffer space available"),
	ErrorEntry(WSAEISCONN,         "Socket is already connected"),
	ErrorEntry(WSAENOTCONN,        "Socket is not connected"),
	ErrorEntry(WSAESHUTDOWN,       "Can't send after socket shutdown"),
	ErrorEntry(WSAETOOMANYREFS,    "Too many references, can't splice"),
	ErrorEntry(WSAETIMEDOUT,       "Connection timed out"),
	ErrorEntry(WSAECONNREFUSED,    "Connection refused"),
	ErrorEntry(WSAELOOP,           "Too many levels of symbolic links"),
	ErrorEntry(WSAENAMETOOLONG,    "File name too long"),
	ErrorEntry(WSAEHOSTDOWN,       "Host is down"),
	ErrorEntry(WSAEHOSTUNREACH,    "No route to host"),
	ErrorEntry(WSAENOTEMPTY,       "Directory not empty"),
	ErrorEntry(WSAEPROCLIM,        "Too many processes"),
	ErrorEntry(WSAEUSERS,          "Too many users"),
	ErrorEntry(WSAEDQUOT,          "Disc quota exceeded"),
	ErrorEntry(WSAESTALE,          "Stale NFS file handle"),
	ErrorEntry(WSAEREMOTE,         "Too many levels of remote in path"),
	ErrorEntry(WSASYSNOTREADY,     "Network system is unavailable"),
	ErrorEntry(WSAVERNOTSUPPORTED, "Winsock version out of range"),
	ErrorEntry(WSANOTINITIALISED,  "WSAStartup not yet called"),
	ErrorEntry(WSAEDISCON,         "Graceful shutdown in progress"),
	ErrorEntry(WSAHOST_NOT_FOUND,  "Host not found"),
	ErrorEntry(WSANO_DATA,         "No host data of that type was found")
};
const int kNumMessages = sizeof(gaErrorList) / sizeof(ErrorEntry);


//// WSAGetLastErrorMessage ////////////////////////////////////////////
// A function similar in spirit to Unix's perror() that tacks a canned 
// interpretation of the value of WSAGetLastError() onto the end of a
// passed string, separated by a ": ".  Generally, you should implement
// smarter error handling than this, but for default cases and simple
// programs, this function is sufficient.
//
// This function returns a pointer to an internal static buffer, so you
// must copy the data from this function before you call it again.  It
// follows that this function is also not thread-safe.
const char* WSAGetLastErrorMessage(const char* pcMessagePrefix, int nErrorID = 0)
{
	// Build basic error string
	static char acErrorBuffer[256];
	ostrstream outs(acErrorBuffer, sizeof(acErrorBuffer));
	outs << pcMessagePrefix << ": ";

	// Tack appropriate canned message onto end of supplied message 
	// prefix. Note that we do a binary search here: gaErrorList must be
	// sorted by the error constant's value.
	ErrorEntry* pEnd = gaErrorList + kNumMessages;
	ErrorEntry Target(nErrorID ? nErrorID : WSAGetLastError());
	ErrorEntry* it = lower_bound(gaErrorList, pEnd, Target);
	if ((it != pEnd) && (it->nID == Target.nID)) {
		outs << it->pcMessage;
	}
	else {
		// Didn't find error in list, so make up a generic one
		outs << "unknown error";
	}
	outs << " (" << Target.nID << ")";

	// Finish error message off and return it.
	outs << ends;
	acErrorBuffer[sizeof(acErrorBuffer) - 1] = '\0';
	return acErrorBuffer;
}

int main(void)
{
	//----------------------
	// Initialize Winsock.
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		cerr << "Error at WSAStartup()\n" << endl;
		return 1;
	}

	//----------------------
	// Create a SOCKET for listening for
	// incoming connection requests.
	SOCKET ServerSocket;
	ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ServerSocket == INVALID_SOCKET) {
		cerr << WSAGetLastErrorMessage("Error at socket()") << endl;
		WSACleanup();
		return 1;
	}
	char option[] = "1";
	setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, option, sizeof(option));

	char addr[256];
	int port;

	printf("Entrez l'addresse du serveur:\n");
	cin >> addr;
	cin.clear();
	cin.ignore(10000, '\n');

	do {
		printf("Entrez le port du serveur (5000-5050):\n");
		cin >> port;
		cin.clear();
		cin.ignore(10000, '\n');
	} while (port < 5000 || port > 5050);

	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(addr);
	service.sin_port = htons(port);

	if (bind(ServerSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
		printf("L'adresse ip entr�e est invalide.");
		cerr << WSAGetLastErrorMessage("bind() failed.") << endl;
		closesocket(ServerSocket);
		WSACleanup();
		return 1;
	}

	//----------------------
	// Listen for incoming connection requests.
	// on the created socket
	if (listen(ServerSocket, 30) == SOCKET_ERROR) {
		cerr << WSAGetLastErrorMessage("Error listening on socket.") << endl;
		closesocket(ServerSocket);
		WSACleanup();
		return 1;
	}

	printf("En attente des connections des clients sur le port %d...\n\n", ntohs(service.sin_port));

	while (true) {

		sockaddr_in sinRemote;
		int nAddrSize = sizeof(sinRemote);

		// Create a SOCKET for accepting incoming requests.
		// Accept the connection.
		SOCKET sd = accept(ServerSocket, (sockaddr*)&sinRemote, &nAddrSize);
		Connections.push_back(sd);

		if (sd != INVALID_SOCKET) {
			DWORD nThreadID;
			CreateThread(0, 0, ConnectionHandler, (void*)sd, 0, &nThreadID);
		}
		else {
			cerr << WSAGetLastErrorMessage("Echec d'une connection.") <<
				endl;
			// return 1;
		}
	}
}

void listenToMessages(void *sd_, char* username) {
	int readBytes;
	char msg[300];
	SOCKET sd = (SOCKET)sd_;
	sockaddr_in socket_info = {0};
	int sl = sizeof(socket_info);
	getpeername(sd, (sockaddr*)&socket_info, &sl);

	Chat chat(username, inet_ntoa(socket_info.sin_addr), ntohs(socket_info.sin_port));
	vector<string> oldMsgs = chat.getOldMessages();

	for (int i = 0; i < oldMsgs.size(); ++i) {
		send(sd, oldMsgs.at(i).c_str(), 300, 0);
	}

	while (true) {
		readBytes = recv(sd, msg, 300, 0);
		if (readBytes > 0) {
			chat.addNewMessage(chat.formatMessage(msg));
			cout << chat.formatMessage(msg) << endl;
			broadcast(chat.formatMessage(msg));
		}
		else {
			cout << string(username) + " disconnected" << endl;
			closesocket(sd);
			Connections.erase(remove(Connections.begin(), Connections.end(), sd), Connections.end());
			broadcast(string(username) + " disconnected");
			return;
		}
	}
}

void closeSocket(void* sd_) {
	SOCKET sd = (SOCKET)sd_;
	closesocket(sd);
	Connections.erase(remove(Connections.begin(), Connections.end(), sd), Connections.end());
}

DWORD WINAPI ConnectionHandler(void* sd_)
{
	SOCKET sd = (SOCKET)sd_;
	char username[200];
	char password[200];
	int readBytes, exitCode;

	readBytes = recv(sd, username, 200, 0);
	if (readBytes > 0) {
	}

	readBytes = recv(sd, password, 200, 0);
	if (readBytes > 0) {
		exitCode = Authenticate(sd_, username, password);
		if (exitCode == 0) {
			closeSocket((void*)sd);
		}
		else {
			listenToMessages((void*)sd, username);
		}
	}
	else if (readBytes == SOCKET_ERROR) {
		cout << WSAGetLastErrorMessage("Echec de la reception !") << endl;
	}
	closeSocket((void*)sd);
	return 0;
}

void sendAllMessages(void* sd_) {
	SOCKET sd = (SOCKET)sd_;
	ifstream is_msg("chat.txt");



}

int Authenticate(void *sd_, char *username, char *password)
{
	SOCKET sd = (SOCKET)sd_;
	ifstream inFile(string(username) + ".txt");
	int exitCode;

	if (inFile.good()) {
		string readPassword;
		getline(inFile, readPassword);
		if (readPassword.compare(string(password)) == 0) {
			cout << username << " a rejoint le chat." << endl;
			send(sd, "1", 4, 0);
			exitCode = 1;
		}
		else {
			cout << "Mot de passe invalide." << endl;
			send(sd, "0", 4, 0);
			exitCode = 0;
		}
	}
	else {
		ofstream outFile(string(username) + ".txt");
		cout << "Cr�ation d'un nouvel utilisateur\n" << "  Username: " << username << "\n  Password: " << password << endl;
		outFile << password;
		outFile.close();
		send(sd, "2", 4, 0);
		exitCode = 2;
	}
	inFile.close();
	return exitCode;
}

