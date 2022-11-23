// UDP client that uses blocking sockets
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
#define SERVER_PORT 15001
#define BUFFER_SIZE 512

int main()
{
	// Server address
	sockaddr_in adresaServera;

	// Size of sockaddr_in structure
	int duzinaAdreseServera = sizeof(adresaServera);

	// Buffer we will use to store message
	char dataBuffer[BUFFER_SIZE];

	// Port on server that will be used for communication with client
	unsigned short serverPort = SERVER_PORT;

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets for this process
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup neuspesan.\nGreska: %d\n", iResult);
		return 1;
	}

	// Initialize adresaServera structure
	memset((char*) &adresaServera, 0, duzinaAdreseServera);
	adresaServera.sin_family = AF_INET;
	adresaServera.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
	adresaServera.sin_port = htons(serverPort);

	// Create a socket
	SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Check if socket creation succeeded
	if (clientSocket == INVALID_SOCKET)
	{
		printf("Neuspesno stvaranje soketa.\nGreska: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	
	// 3. Obezbediti klijentu da pošalje više od jedne poruke serveru.
	// Dodajemo prehodna_poruka[], i celu while petlju
	char prehodna_poruka[BUFFER_SIZE];

	// Slanje poruka stavljamo u beskonačnu petlju
	while (true)
	{
		// Read string from user into outgoing buffer
		printf("Unesi poruku za slanje: ");
		gets_s(dataBuffer, BUFFER_SIZE);

		//  4. Omogućiti završetak rada klijenta korišćenjem naredbe "stop client".
		// Check if client closing command is entered
		if (!strcmp(dataBuffer, "stop client"))
		{
			break;
		}

		// Send message to server
		iResult = sendto
			(clientSocket,				// Own socket
			dataBuffer,					// Text of message
			strlen(dataBuffer),			// Message size
			0,							// No flags
			(SOCKADDR*) &adresaServera,	// Address structure of server (type, IP address and port)
			sizeof(adresaServera));		// Size of sockadr_in structure

		// Check result of sendto function
		if (iResult == SOCKET_ERROR)
		{
			printf("Slanje od klijenta ka serveru neuspeno.\nGreska: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

		//  7. Omogućiti gašenje servera ukoliko sa klijenta uzastopno stignu dve identične poruke.

		// Compare current and previous message	
		if (!strcmp(prehodna_poruka, dataBuffer))
		{
			printf("Dve identicne poruke su se poslale uzastopno. Server se gasi.\n");
			break;
		}

		// Store current message for next comparison
		strcpy(prehodna_poruka, dataBuffer);

		// Receive server message
		iResult = recvfrom(clientSocket, dataBuffer, BUFFER_SIZE, 0, (SOCKADDR*) &adresaServera, &duzinaAdreseServera);

		if (iResult == SOCKET_ERROR)
		{
			printf("Primanje poruke od servera neuspesno.\nGreska: %d\n", WSAGetLastError());
			continue;
		}

		//  6. Na klijentu ispisati odgovor dobijen od servera.
		
		// Set end of string
		dataBuffer[iResult] = '\0';
		// Log message from server
		printf("Poruka od servera: %s.\n\n", dataBuffer);
	}

	// Close client application
	iResult = closesocket(clientSocket);
	if (iResult == SOCKET_ERROR)
	{
		printf("Neuspesno zatvaranje soketa.\nGreska: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

    // Close Winsock library
	WSACleanup();

	return 0;
}
