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

#define SERVER_IP_ADDRESS "127.0.0.1"		// IPv4 address of server
#define BUFFER_SIZE 512						// Size of buffer that will be used for sending and receiving messages to client

int main()
{
    // Server address structure
    sockaddr_in adresaServera;

    // Size of server address structure
	int duzinaAdreseServera = sizeof(adresaServera);

	// Buffer that will be used for sending and receiving messages to client
    char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is used to receive details of the Windows Sockets implementation
    WSADATA wsaData;
    
	// Initialize windows sockets for this process
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    
	// Check if library is succesfully initialized
	if (iResult != 0)
    {
        printf("WSAStartup neuspesan.\nGreska: %d\n", iResult);
        return 1;
    }

	// Initialize memory for address structure
    memset((char*) &adresaServera, 0, sizeof(adresaServera));		
    
	// Read server's port number
	printf("Unesi broj porta servera (15011 or 15012):\n");
    gets_s(dataBuffer, BUFFER_SIZE);
	unsigned long serverPort = atoi(dataBuffer);

	 // Initialize address structure of server
	adresaServera.sin_family = AF_INET;								// IPv4 address famly
    adresaServera.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// Set server IP address using string
    adresaServera.sin_port = htons(serverPort);						// Set server port

	// Create a socket
    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Check if socket creation succeeded
    if (clientSocket == INVALID_SOCKET)
    {
        printf("Neuspelo otvaranje soketa.\nGreska: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

   	while (true)
   	{	
		printf("Unesite poruku za slanje:\n");

		// Read string from user into outgoing buffer
    	gets_s(dataBuffer, BUFFER_SIZE);

		// Close client if end  message is entered
    	if (!strcmp(dataBuffer, "end"))
    	{
       		printf("Klijent je zavrsio. Zatvara se.\n");
	    	break;
    	}
	
		// Send message to server
   		iResult = sendto
			(clientSocket,					// Own socket
			dataBuffer,						// Text of message
			strlen(dataBuffer),				// Message size
			0,								// No flags
			(SOCKADDR*) &adresaServera,		// Address structure of server (type, IP address and port)
			sizeof(adresaServera));			// Size of sockadr_in structure

		// Check if message is succesfully sent. If not, close client application
		if (iResult == SOCKET_ERROR)
    	{
	        printf("Neuspelo slanje poruke serveru.\nGreska: %d\n", WSAGetLastError());
	        closesocket(clientSocket);
	        WSACleanup();
	        return 1;
    	}
   	}

	// Only for demonstration purpose
	printf("Press any key to exit: ");
	_getch();

	// Close client application
    iResult = closesocket(clientSocket);
    if (iResult == SOCKET_ERROR)
    {
        printf("closesocket nije uspeo.\nGreska: %d\n", WSAGetLastError());
		WSACleanup();
        return 1;
    }

	// Close Winsock library
    WSACleanup();

	// Client has succesfully sent a message
    return 0;
}
