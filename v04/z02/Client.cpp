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
#define BUFFER_SIZE 512	

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
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Check if library is succesfully initialized
    if (iResult != 0)
    {
        printf("WSAStartup neuspesan.\nGreska: %d\n", iResult);
        return 1;
    }

    // Initialize memory for address structure
    memset((char*) &adresaServera, 0, sizeof(adresaServera));

    // Initialize address structure of server
    adresaServera.sin_family = AF_INET;	
    adresaServera.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    
	// Učitati iz komandne linije port servera ka kome će se slati poruke
    printf("Unesite port servera (17010 ili 17011): ");
    gets_s(dataBuffer, BUFFER_SIZE);
    unsigned short serverPort = atoi(dataBuffer);

	// Učitanu vrednost porta smestamo u polje sin_port serverske adresne strukture
    adresaServera.sin_port = htons(serverPort);

    // Create a UDP socket
    SOCKET klijentskiSoket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Check if socket creation succeeded
    if (klijentskiSoket == INVALID_SOCKET)
    {
        printf("Stvaranje klijentskog soketa neuspesno.\nGreska: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

	// DODATO - KAKO BI MOGLI SLATI VIŠE PORUKA
    while (true)
    {
        printf("Unesi poruku za slanje:\n");

        // Read string from user into outgoing buffer
        gets_s(dataBuffer, BUFFER_SIZE);

		// Gasimo klijenta ako je uneto "kraj"
        if (strcmp(dataBuffer, "kraj") == 0)
        {
            printf("Klijent odustaje od daljeg slanja i zatvara se.\n");
            break;
        }

        // Send message to server
        iResult = sendto
			(klijentskiSoket,			// Own socket
            dataBuffer,					// Text of message
            strlen(dataBuffer),			// Message size
            0,							// No flags
            (SOCKADDR*) &adresaServera,	// Address structure of server (type, IP address and port)
            sizeof(adresaServera));		// Size of sockadr_in structure

        // Check if message is succesfully sent. If not, close client application
        if (iResult == SOCKET_ERROR)
        {
            printf("Slanje poruke od klijenta ka serveru neuspesno.\nGreska: %d\n", WSAGetLastError());
            closesocket(klijentskiSoket);
            WSACleanup();
            return 1;
        }
    }

    // Only for demonstration purpose
    printf("Press any key to exit: ");
    _getch();

    // Close client application
    iResult = closesocket(klijentskiSoket);
    if (iResult == SOCKET_ERROR)
    {
        printf("closesocket neuspesan.\nGreska: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Close Winsock library
    WSACleanup();

    // Client has succesfully sent a message
    return 0;
}
