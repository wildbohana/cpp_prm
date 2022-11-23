#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27016
#define BUFFER_SIZE 256

// TCP client that uses blocking sockets
int main()
{
	// Socket used to communicate with server
	SOCKET klijentSoket = INVALID_SOCKET;

	// Variable used to store function return value
	int iResult;

	// Buffer we will use to store message
	char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup neuspesan.\nGreska: %d\n", WSAGetLastError());
		return 1;
	}

	// Create a socket
	klijentSoket = socket(AF_INET, SOCK_STREAM,	IPPROTO_TCP);

	if (klijentSoket == INVALID_SOCKET)
	{
		printf("Stvaranje klijentskog soketa neuspešno.\nGreska: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
	serverAddress.sin_port = htons(SERVER_PORT);

	// Connect to server specified in serverAddress and socket klijentSoket
	if (connect(klijentSoket, (SOCKADDR*) &serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Nemoguce spajanje sa serverom.\n");
		closesocket(klijentSoket);
		WSACleanup();
		return 1;
	}

	// DODATO - provera da li je došlo do kraja (koja stiže od servera)
	bool kraj = false;

	// DODATO - kako bi smo mogli da primamo/šaljemo više poruka zaredom
	do {
		// Receive message from server
		iResult = recv(klijentSoket, dataBuffer, BUFFER_SIZE, 0);
		
		// Check if message is successfully received
		if (iResult > 0)	
		{
			dataBuffer[iResult] = '\0';

			// Log message text
			printf("Server je poslao: %s.\n", dataBuffer);

			if (strstr(dataBuffer, "POBEDILI") 
			||  strstr(dataBuffer, "IZGUBILI"))
			{
				kraj = true;
			}
		}
		// Check if shutdown command is received
		else if (iResult == 0)	
		{
			// Connection was closed successfully
			printf("Veza sa serverom je zatvorena.\n");
			closesocket(klijentSoket);
			WSACleanup();
			return 0;
		}
		// There was an error during recv
		else	
		{
			printf("Neuspesno primanje poruke od servera.\nGreska: %d\n", WSAGetLastError());
			closesocket(klijentSoket);
			WSACleanup();
			return 1;
		}

		if (kraj)
		{
			break;
		}

		// Read string from user into outgoing buffer
		printf("Posalji rec na odabrano slovo za slanje: ");
		gets_s(dataBuffer, BUFFER_SIZE);

		// Send message to server using connected socket
		iResult = send(klijentSoket, dataBuffer, (int) strlen(dataBuffer), 0);

		// Check result of send function
		if (iResult == SOCKET_ERROR)
		{
			printf("Slanje poruke serveru nije bilo uspesno.\nGreska: %d\n", WSAGetLastError());
			closesocket(klijentSoket);
			WSACleanup();
			return 1;
		}
	} while (true);

	// Shutdown the connection since we're done
	iResult = shutdown(klijentSoket, SD_BOTH);

	// Check if connection is succesfully shut down.
	if (iResult == SOCKET_ERROR)
	{
		printf("Gasenje neuspesno.\nGreska: %d\n", WSAGetLastError());
		closesocket(klijentSoket);
		WSACleanup();
		return 1;
	}

	// For demonstration purpose
	printf("\nPress any key to exit: ");
	_getch();

	// Close connected socket
	closesocket(klijentSoket);

	// Deinitialize WSA library
	WSACleanup();

	return 0;
}
