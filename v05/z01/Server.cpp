// UDP server that use non-blocking sockets
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

// Port numbers of server that will be used for communication with clients
#define SERVER_PORT1 15011
#define SERVER_PORT2 15012

// Size of buffer that will be used for sending and receiving messages to clients
#define BUFFER_SIZE 512

int main()
{
	// Server addresses
	sockaddr_in adresaServera1;
	sockaddr_in adresaServera2;

	// Buffer we will use to send and receive clients' messages
	char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		printf("WSAStartup neuspesan.\nGreska: %d\n", WSAGetLastError());
		return 1;
	}

	// Initialize adresaServera1 and adresaServera2 structures used by bind function
	memset((char*) &adresaServera1, 0, sizeof(adresaServera1));
	adresaServera1.sin_family = AF_INET;
	adresaServera1.sin_addr.s_addr = INADDR_ANY;
	adresaServera1.sin_port = htons(SERVER_PORT1);

	memset((char*) &adresaServera2, 0, sizeof(adresaServera2));
	adresaServera2.sin_family = AF_INET;
	adresaServera2.sin_addr.s_addr = inet_addr("127.0.0.1");
	adresaServera2.sin_port = htons(SERVER_PORT2);

	// Create first socket
	SOCKET serverSocket1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Check if socket creation succeeded
	if (serverSocket1 == INVALID_SOCKET)
	{
		printf("Otvaranje soketa za server neuspesno.\nGreska: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Create second socket
	SOCKET serverSocket2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Check if socket creation succeeded
	if (serverSocket2 == INVALID_SOCKET)
	{
		printf("Otvaranje soketa za server neuspesno.\nGreska: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Bind server address structure (type, port number and local address) to first socket
	int iResult = bind(serverSocket1,(SOCKADDR*) &adresaServera1, sizeof(adresaServera1));

	// Check if socket is succesfully binded to server address data
	if (iResult == SOCKET_ERROR)
	{
		printf("Soket bind neuspesan.\nGreska: %d\n", WSAGetLastError());
		closesocket(serverSocket1);
		WSACleanup();
		return 1;
	}

	// Bind server address structure (type, port number and local address) to second socket
	iResult = bind(serverSocket2,(SOCKADDR*) &adresaServera2, sizeof(adresaServera2));

	// Check if socket is succesfully binded to server address data
	if (iResult == SOCKET_ERROR)
	{
		printf("Soket bind neuspesan.\nGreska: %d\n", WSAGetLastError());
		closesocket(serverSocket2);
		WSACleanup();
		return 1;
	}

	// Setting non-blocking mode to both sockets (mode = 1)
	unsigned long mode = 1;
	if (ioctlsocket(serverSocket1, FIONBIO, &mode) != 0 
	||  ioctlsocket(serverSocket2, FIONBIO, &mode) != 0)
	{
		printf("ioctlsocket neuspesan.\nGreska: %d\n", WSAGetLastError());
		closesocket(serverSocket1);
		closesocket(serverSocket2);
		WSACleanup();
		return 1;
	}

	printf("UDP server je pokrenut. Cekaju se poruke od klijenata.\n");

	// Main server loop
	while (true)
	{
		// Declare and initialize client address that will be set from recvfrom
		sockaddr_in adresaKlijenta;
		memset(&adresaKlijenta, 0, sizeof(adresaKlijenta));

		// Set whole buffer to zero
		memset(dataBuffer, 0, BUFFER_SIZE);

		// Size of client address
		int duzinaAdreseKlijenta = sizeof(adresaKlijenta);

		// Declare and initialize set of socket descriptor for buffer reading
		fd_set fdCitanje;
		FD_ZERO(&fdCitanje);

		// Add socket to set fdCitanje
		FD_SET(serverSocket1, &fdCitanje);
		FD_SET(serverSocket2, &fdCitanje);

		// Declare and initialize set of socket descriptor for exceptions
		fd_set fdIzuzeci;
		FD_ZERO(&fdIzuzeci);

		// Add socket to set fdIzuzeci
		FD_SET(serverSocket1, &fdIzuzeci);
		FD_SET(serverSocket2, &fdIzuzeci);

		// Wait new messages to arrive in one of the sockets in fdCitanje set and wait for possible exception on sockets in fdIzuzeci set
		int sResult = select(0, &fdCitanje , NULL , &fdIzuzeci , NULL);

		// Check whether the error occurred
		if (sResult == SOCKET_ERROR)
		{
			printf("select nije uspeo.\nGreska: %d\n", WSAGetLastError());
			break;
		}
		// If result is positive, that is indicator that packet has arrived to one or more socket or there is exception on some socket
		// Result number corresonds to number of sockets that have an event
		else if (sResult > 0)
		{
			unsigned long brojPorta = 0;

			// We will check if packet is arrived to one of our sockets (if two or more sockets are used, we have to check each socket)
			if (FD_ISSET(serverSocket1, &fdCitanje))
			{
				// Receive client message
				iResult = recvfrom
					(serverSocket1,					// Socket with port 15001
					dataBuffer,						// Buffer that will be used for receiving message
					BUFFER_SIZE,					// Maximal size of buffer
					0,								// No flags
					(SOCKADDR *)&adresaKlijenta,	// Client information from received message (ip address and port)
					&duzinaAdreseKlijenta);			// Size of sockadd_in structure
				
				brojPorta = SERVER_PORT1;
				
				if (iResult != SOCKET_ERROR)
				{
					// Set end of string
					dataBuffer[iResult] = '\0';

					printf("Server je primio poruku na %d portu. Klijent je poslao: %s.\n", brojPorta,dataBuffer);
				}
				else
				{
					printf("Primanje poruke od klijenta neuspesno.\nGreska: %d\n", WSAGetLastError());
					continue;
				}
			}

			if (FD_ISSET(serverSocket2, &fdCitanje))
			{
				// Receive client message
				iResult = recvfrom
					(serverSocket2,					// Socket with port 15002
					dataBuffer,						// Buffer that will be used for receiving message
					BUFFER_SIZE,					// Maximal size of buffer
					0,								// No flags
					(SOCKADDR *)&adresaKlijenta,	// Client information from received message (ip address and port)
					&duzinaAdreseKlijenta);			// Size of sockadd_in structure

				brojPorta = SERVER_PORT2;

				if (iResult != SOCKET_ERROR)
				{
					// Set end of string
					dataBuffer[iResult] = '\0';

					printf("Server je primio poruku na %d portu. Klijent je poslao: %s.\n", brojPorta,dataBuffer);
				}
				else
				{
					printf("Primanje poruke od klijenta neuspesno.\nGreska: %d\n", WSAGetLastError());
					continue;
				}
			}

			// We will check if an exception occurred on one of our sockets. Close server application if error occured
			if (FD_ISSET(serverSocket1, &fdIzuzeci))
			{
				break;
			}
			if (FD_ISSET(serverSocket2, &fdIzuzeci))
			{
				break;
			}
			// Neither of two socket is in these two sets (some error has occurred)
			else
			{
				continue;
			}

			// Check if message is succesfully received
		}
	}

	// Close server application
	if (SOCKET_ERROR == closesocket(serverSocket1) 
	||  SOCKET_ERROR == closesocket(serverSocket2))
	{
		printf("Zatvaranje serverskog soketa nije uspelo.\nGreska: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	printf("Server je uspesno ugasen.\n");

	// Close Winsock library
	WSACleanup();
	return 0;
}
