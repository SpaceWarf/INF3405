#undef UNICODE

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>


using namespace std;

// Link avec ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

//To make sure there are no errors
#pragma warning(disable : 4996)

// External functions
extern DWORD WINAPI InputForChat(void* sd_);
extern void DoSomething(char *src, char *dest);

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
	char host[256];
	char port[256];

	printf("Entrez l'adresse ip du serveur\n");
	cin >> host;
	cin.clear();
	cin.ignore(10000, '\n');
	//cout << "Ce que inet_adrr retourne: " << inet_addr(ip) << endl;
	while (!inet_addr(host)) {
		printf("Cette adresse est invalide. Veuillez entrer une adresse valide\n");
		//cout << "Ce que inet_adrr retourne: " << inet_addr(ip) << endl;
		cin >> host;
		cin.clear();
		cin.ignore(10000, '\n');
	}

	printf("Entrez le port du serveur\n");
	cin >> port;
	cin.clear();
	cin.ignore(10000, '\n');

	//----------------------------
	// Demander à l'usager nom et mot de passe
	char username[256];
	char password[256];

	printf("Veuillez entrer votre nom d'utilisateur:\n");
	cin >> username;
	cin.clear();
	cin.ignore(10000, '\n');

	printf("Veuillez entrer votre mot de passe:\n");
	cin >> password;
	cin.clear();
	cin.ignore(10000, '\n');

	// getaddrinfo obtient l'adresse IP du host donné
	iResult = getaddrinfo(host, port, &hints, &result);
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
	//----------------------------------------------------
	printf("Adresse trouvee pour le serveur %s : %s\n\n", host, inet_ntoa(adresse->sin_addr));
	printf("Tentative de connexion au serveur %s avec le port %s\n\n", inet_ntoa(adresse->sin_addr), port);

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

	printf("Connecte au serveur %s:%s\n\n", host, port);
	freeaddrinfo(result);

	//-----------------------------
	// Envoyer les informations au serveur
	iResult = send(leSocket, username, 200, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Erreur du send: %d\n", WSAGetLastError());
		closesocket(leSocket);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();

		return 1;
	}

	iResult = send(leSocket, password, 200, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Erreur du send: %d\n", WSAGetLastError());
		closesocket(leSocket);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();

		return 1;
	}

	printf("Nombre d'octets envoyes : %ld\n", iResult);

	//------------------------------
	// Recevoir le resultat de la demande d'authentification
	string auth;
	iResult = recv(leSocket, Authenticated, 7, 0);
	if (iResult > 0) {
		printf("Nombre d'octets recus: %d\n", iResult);
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

	//Thread en parrallele pour ecouter le keyboard de l'utilisateur sans empecher les nouveaux messages d'etre affiches
	DWORD nThreadID;
	CreateThread(0, 0, InputForChat, 0, leSocket, &nThreadID);

	//boucle inifinie du chat (écoute du serveur pour les messages)
	while (true) {
		//Test pour le thread
		//Sleep(5000);
		//cout << "nouveau message du server" << endl;

	}

	// cleanup
	closesocket(leSocket);
	WSACleanup();

	printf("Appuyez une touche pour finir\n");
	getchar();
	return 0;
}

//// EchoHandler ///////////////////////////////////////////////////////
// Handles the incoming data by reflecting it back to the sender.
DWORD WINAPI InputForChat(void* sd_)
{
	SOCKET socket = (SOCKET)sd_;
	int iResult;
	char msg[200];
	while (true) {
		cin >> msg;
		cin.clear();
		cin.ignore(10000, '\n');

		cout << "[tentative d'envoi de ce message]" << msg << endl;
		iResult = send(socket, msg, 200, 0);
		if (iResult == SOCKET_ERROR) {
			printf("Erreur du send du message: %d\n", WSAGetLastError());
			closesocket(socket);
			WSACleanup();
			printf("Appuyez une touche pour finir\n");
			getchar();

			return 1;
		}

	}
}
// Do Something with the information
void DoSomething(char *src, char *dest)
{

}
