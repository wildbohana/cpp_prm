// UDP server
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

#define SERVER_PORT 15001
#define BUFFER_SIZE 512		

int main()
{
	// Server address
	sockaddr_in adresaServera;

	// Size of sockaddr_in structure
	int velicinaAdreseServera = sizeof(adresaServera);

	// Buffer we will use to receive client message
	char dataBuffer[BUFFER_SIZE];

	// Variable used to store function return value
	int iResult;

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup neuspesan.\nGreska: %d\n", WSAGetLastError());
		return 1;
	}

	// Initialize adresaServera structure used by bind function
	memset((char*) &adresaServera, 0, velicinaAdreseServera);
	// Use all available addresses of server - INADDR_ANY
	adresaServera.sin_addr.s_addr = INADDR_ANY; 
	adresaServera.sin_family = AF_INET;
	adresaServera.sin_port = htons(SERVER_PORT);

	// Create a socket
	SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Check if socket creation succeeded
	if (serverSocket == INVALID_SOCKET)
	{
		printf("Stvaranje soketa za server neuspesno.\nGreska: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Bind port number and local address to socket
	iResult = bind(serverSocket, (SOCKADDR*) &adresaServera, velicinaAdreseServera);
    // Check if socket is succesfully binded to server datas
	if (iResult == SOCKET_ERROR)
	{
		printf("Neuspesno povezivanje serverskog soketa.\nGreska: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	printf("UDP server je pokrenut i ceka poruke od klijenata.\n");

	int velika_slova, mala_slova, ostali_karakteri;
	char prethodni[BUFFER_SIZE];

	// Main server loop
	while (1)
	{
		// ClientAddress will be set from recvfrom
		sockaddr_in clientAddress;
		memset(&clientAddress, 0, velicinaAdreseServera);

		// Set whole buffer to zero
		memset(dataBuffer, 0, BUFFER_SIZE);

		// Receive client message
		iResult = recvfrom(serverSocket, dataBuffer, BUFFER_SIZE, 0, (SOCKADDR*) &clientAddress, &velicinaAdreseServera);

        // Check if message is succesfully received
		if (iResult == SOCKET_ERROR)
		{
			printf("Server nije uspeo da preuzme poruku od klijenta.\nGreska: %d\n", WSAGetLastError());
			continue;
		}

		// Set end of string
		dataBuffer[iResult] = '\0';
		
		// 15 spaces for decimal notation (for example: "192.168.100.200") + '\0'
		char ipAdresa[16]; 

		// Copy client ip to local char[]
		strcpy_s(ipAdresa, sizeof(ipAdresa), inet_ntoa(clientAddress.sin_addr));

		// Convert port number from network byte order to host byte order
		unsigned short clientPort = ntohs(clientAddress.sin_port);

		// 8. Koristeći debugging mode (prekidne tačke i watch window) saznati ASCII vrednost slova „a“ , „z“ , „A“ i „Z“. 

		// Check big, small letters and other characters in message
		velika_slova = mala_slova = ostali_karakteri = 0;
		for (int i = 0; i < strlen(dataBuffer); i++)
		{
			// 65 = A, 90 = Z
			if (65 <= dataBuffer[i] && dataBuffer[i] <= 90)
			{
				++velika_slova;
			}
			// 97 = a, 122 = z
			else if (97 <= dataBuffer[i] && dataBuffer[i] <= 122)
			{
				++mala_slova;
			}
			else
			{
				++ostali_karakteri;
			}
		}

		// Log client ip address, port and message text
		printf("Klijent spojen sa IP: %s, port: %d, poslao: %s.\n", ipAdresa, clientPort, dataBuffer);

		/*
		9. Nakon prijema svake poruke, na serveru ispisati: 
		- dužinu primljene poruke, 
		- broj malih slova u toj poruci, 
		- broj velikih slova u poruci,
		- broj drugih znakova (karaktera) u toj poruci.
		*/

		// Log number of big, small letters and other characters in message
		printf("Poruka ima %d karaktera.\n", iResult);
		printf("Poruka ima %d velikih slova.\n", velika_slova);
		printf("Poruka ima %d malih slova.\n", mala_slova);
		printf("Poruka ima %d ostalih karaktera.\n", ostali_karakteri);

		// 7. Omogućiti gašenje servera ukoliko sa klijenta uzastopno stignu dve identične poruke.

		// Check if client sent two identical messages consecutively
		// compare current and prethodni message
		if (!strcmp(prethodni, dataBuffer))
		{
			printf("Klijent je poslao dve identicne poruke zaredom. Server se gasi.\n");
			break;
		}

		// Store current message for next comparison
		strcpy(prethodni, dataBuffer);

		// 5. Primljenu poruku sa servera proslediti (poslati nazad) klijentu.
		
		// Send message to client
		iResult = sendto(serverSocket, dataBuffer, strlen(dataBuffer), 0, (SOCKADDR*) &clientAddress, velicinaAdreseServera);

		if (iResult == SOCKET_ERROR)
		{
			printf("Slanje poruke od servera ka klijentu neuspesan.\nGreska: %d\n", WSAGetLastError());
			continue;
		}
	}

	// Close server application
	iResult = closesocket(serverSocket);
	if (iResult == SOCKET_ERROR)
	{
		printf("Zatvaranje serverske uticnice neuspesno.\nGreska: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	printf("Server je uspesno ugasen.\n");

    // Close Winsock library
	WSACleanup();

	return 0;
}
