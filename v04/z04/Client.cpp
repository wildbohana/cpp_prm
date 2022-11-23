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
#define SERVER_PORT 19010
#define BUFFER_SIZE 256

// TCP client that use blocking sockets
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
    klijentSoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (klijentSoket == INVALID_SOCKET)
    {
        printf("Otvaranje soketa neuspesno.\nGreska: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Create and initialize address structure
    sockaddr_in adresaServera;
    adresaServera.sin_family = AF_INET;
    adresaServera.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    adresaServera.sin_port = htons(SERVER_PORT);

    // Connect to server specified in adresaServera and socket klijentSoket
    if (connect(klijentSoket, (SOCKADDR*) &adresaServera, sizeof(adresaServera)) == SOCKET_ERROR)
    {
        printf("Spajanje sa serverom neuspesno.\n");
        closesocket(klijentSoket);
        WSACleanup();
        return 1;
    }
	
	// Niz brojeva koji saljemo
    int nizBrojeva[3];

    while (true)
    {
        // Ucitavanje niza od 3 broja
        for (int i = 0; i < 3; i++)
        {
            printf("\nUnesite broj za slanje: ");
            gets_s(dataBuffer, BUFFER_SIZE);
            nizBrojeva[i] = atoi(dataBuffer);
            nizBrojeva[i] = htonl(nizBrojeva[i]);
        }
        
        // Saljemo nizBrojeva prosledjujuci njegovu adresu i duzinu poruke (niza) u bajtima
        iResult = send
			(klijentSoket, 
			(char*) nizBrojeva, 
			3 * sizeof(int), 
			0);

        // Check result of send function
        if (iResult == SOCKET_ERROR)
        {
            printf("Slanje poruke serveru neuspesno.\nGreska: %d\n", WSAGetLastError());
            closesocket(klijentSoket);
            WSACleanup();
            return 1;
        }

        printf("Poruka je uspesno poslata serveru. Ukupno bajtova: %ld\n", iResult);

        // Prijem poruke koju salje server
        iResult = recv
			(klijentSoket, 
			dataBuffer, 
			BUFFER_SIZE, 
			0);
	
		// Check if message is successfully received
        if (iResult > 0)
        {
            dataBuffer[iResult] = '\0';
            printf("\nServer salje: %s\n", dataBuffer);
        }
		// Check if shutdown command is received
        else if (iResult == 0)	
        {
            // Connection was closed successfully
            printf("Veza sa serverom je uspesno zatvorena.\n");
            break;
        }
		// Greska pri prijemu
        else	
        {
            printf("Prijem poruke od servera je neuspesan.\nGreska: %d\n", WSAGetLastError());
            break;
        }

		// PROVERA DA LI JE DOŠLO DO PREKIDA (TJ DA LI JE POZVANO EXIT)
        printf("\nZa prekid slanja unite 'exit', a za nastavak pritisnite bilo koji taster:\n");
        gets_s(dataBuffer, BUFFER_SIZE);
        if (!strcmp(dataBuffer, "exit"))
		{
            break;
		}
    }

    // Shutdown the connection since we're done
    iResult = shutdown(klijentSoket, SD_BOTH);

    // Check if connection is succesfully shut down.
    if (iResult == SOCKET_ERROR)
    {
        printf("Shutdown neuspesan.\nGreska: %d\n", WSAGetLastError());
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
