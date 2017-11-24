#undef UNICODE

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>


using namespace std;

// Link avec ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

//To make sure there are no errors
#pragma warning(disable : 4996)

// External functions
extern DWORD WINAPI ListenForChat(void* sd_);
extern void DoSomething(char *src, char *dest);
extern void InputForChat(SOCKET socket);

int __cdecl main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET leSocket;// = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;

	int iResult;
	char Authenticated[10];
	char slctdAddr[15];

	//--------------------------------------------
	// InitialisATION de Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("Erreur de WSAStartup: %d\n", iResult);
		return 1;
	}
	// On va creer le socket pour communiquer avec le serveur
	leSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (leSocket == INVALID_SOCKET) {
		printf("Erreur de socket(): %ld\n\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return 1;
	}
	//--------------------------------------------
	// On va chercher l'adresse du serveur en utilisant la fonction getaddrinfo.
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;        // Famille d'adresses
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;  // Protocole utilisé par le serveur

	//Demander adresse ip et port
	string host;
	string port;

	printf("Entrez l'adresse ip du serveur\n");
	getline(cin, host);
	//cout << "Ce que inet_adrr retourne: " << inet_addr(ip) << endl;
	while (!inet_addr(host.c_str())) {
		printf("Cette adresse est invalide. Veuillez entrer une adresse valide\n");
		//cout << "Ce que inet_adrr retourne: " << inet_addr(ip) << endl;
		getline(cin, host);
	}

	printf("Entrez le port du serveur\n");
	getline(cin, port);

	//----------------------------
	// Demander à l'usager nom et mot de passe
	string username;
	string password;

	printf("Veuillez entrer votre nom d'utilisateur:\n");
	getline(cin, username);

	printf("Veuillez entrer votre mot de passe:\n");
	getline(cin, password);

	// getaddrinfo obtient l'adresse IP du host donné
	iResult = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
	if (iResult != 0) {
		printf("Erreur de getaddrinfo: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	//---------------------------------------------------------------------		
	//On parcours les adresses retournees jusqu'a trouver la premiere adresse IPV4
	while ((result != NULL) && (result->ai_family != AF_INET))
		result = result->ai_next;

	//	if ((result != NULL) &&(result->ai_family==AF_INET)) result = result->ai_next;  

	//-----------------------------------------
	if (((result == NULL) || (result->ai_family != AF_INET))) {
		freeaddrinfo(result);
		printf("Impossible de recuperer la bonne adresse\n\n");
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return 1;
	}

	sockaddr_in *adresse;
	adresse = (struct sockaddr_in *) result->ai_addr;

	// On va se connecter au serveur en utilisant l'adresse qui se trouve dans
	// la variable result.
	iResult = connect(leSocket, result->ai_addr, (int)(result->ai_addrlen));
	if (iResult == SOCKET_ERROR) {
		printf("Impossible de se connecter au serveur %s sur le port %s\n\n", inet_ntoa(adresse->sin_addr), port);
		freeaddrinfo(result);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return 1;
	}

	freeaddrinfo(result);

	//-----------------------------
	// Envoyer les informations au serveur
	iResult = send(leSocket, username.c_str(), 200, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Erreur du send: %d\n", WSAGetLastError());
		closesocket(leSocket);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();

		return 1;
	}

	iResult = send(leSocket, password.c_str(), 200, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Erreur du send: %d\n", WSAGetLastError());
		closesocket(leSocket);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();

		return 1;
	}

	//------------------------------
	// Recevoir le resultat de la demande d'authentification
	string auth;
	iResult = recv(leSocket, Authenticated, 4, 0);
	if (iResult > 0) {
		auth = string(Authenticated);
		//printf("Le mot recu est %*s\n", iResult, auth);

		if (auth.compare("0") == 0) {
			cout << "Mot de passe invalide. \n";
			printf("Appuyez une touche pour finir\n");
			getchar();
			return 1;
		}
		else if (auth.compare("1") == 0) {
			cout << "Connecté au chat en tant que " << username << endl;
		}
		else {
			cout << "Connecté au chat en tant que nouvel utilisateur " << username << endl;
		}
	}
	else {
		printf("Erreur de reception : %d\n", WSAGetLastError());
	}

	DWORD nThreadID;
	CreateThread(0, 0, ListenForChat, (void*)leSocket, 0, &nThreadID);

	InputForChat(leSocket);

	// cleanup
	closesocket(leSocket);
	WSACleanup();

	printf("Appuyez une touche pour finir\n");
	getchar();
	return 0;
}

DWORD WINAPI ListenForChat(void* sd_)
{
	int readBytes;
	SOCKET sd = (SOCKET)sd_;
	char serverMsg[200];

	while (true) {
		readBytes = recv(sd, serverMsg, 200, 0);
		if (readBytes > 0) {
			cout << serverMsg << endl;
		}
	}
	return 0;
}

void InputForChat(SOCKET socket) {
	int iResult;
	string msg;
	while (true) {
		getline(cin, msg);
		iResult = send(socket, msg.c_str(), 200, 0);
		if (iResult == SOCKET_ERROR) {
			printf("Erreur du send du message: %d\n", WSAGetLastError());
			closesocket(socket);
			WSACleanup();
			printf("Appuyez une touche pour quitter le chat\n");
			getchar();

			break;
		}
	}
}

// Do Something with the information
void DoSomething(char *src, char *dest)
{

}
