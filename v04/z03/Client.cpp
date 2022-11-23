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
#define SERVER_PORT 18010
#define BUFFER_SIZE 256

// TCP client that use blocking sockets
int main()
{
	// Variable used to store function return value
    int iResult;

    // Buffer we will use to store message
    char dataBuffer[BUFFER_SIZE];

    // WSADATA data structure that is to receive details of the Windows Sockets implementation
    WSADATA wsaData;

    // Initialize windows sockets library for this process
    // Check if library is succesfully initialized
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup neuspesan.\nGreska: %d\n", WSAGetLastError());
        return 1;
    }

	// MALO DRUGAČIJA INICIJALIZACIJA
    // Socket used to communicate with server
    SOCKET klijentskiSoket = INVALID_SOCKET;
    
	// Create a socket
    klijentskiSoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (klijentskiSoket == INVALID_SOCKET)
    {
        printf("Stvaranje socketa neuspesno.\nGreska: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Create and initialize address structure
    sockaddr_in adresaServera;
    adresaServera.sin_family = AF_INET;
    adresaServera.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    adresaServera.sin_port = htons(SERVER_PORT);

    // Connect to server specified in adresaServera and socket klijentskiSoket
    if (connect(klijentskiSoket, (SOCKADDR*) &adresaServera, sizeof(adresaServera)) == SOCKET_ERROR)
    {
        printf("Nije moguce spajanje sa serverom.\n");
        closesocket(klijentskiSoket);
        WSACleanup();
        return 1;
    }

	// DODATO - NIZ BROJEVA I WHILE PETLJA
    int nizBrojeva[3];

    while (true)
    {
        // Ucitavanje niza od 3 broja
        for (int i = 0; i < 3; i++)
        {
            printf("\nUnesite broj za slanje: ");
            gets_s(dataBuffer, BUFFER_SIZE);
            nizBrojeva[i] = atoi(dataBuffer);

			// Priprema podatka za slanje, 
			// treba nam konverzija iz host u network zapis tj. htonl()
            nizBrojeva[i] = htonl(nizBrojeva[i]); 
        }

        // Saljemo nizBrojeva prosledjujuci njegovu adresu i duzinu poruke (niza) u bajtima
        iResult = send(klijentskiSoket, (char*) nizBrojeva, 3 * sizeof(int), 0);

        // Check result of send function
        if (iResult == SOCKET_ERROR)
        {
            printf("Slanje poruke serveru nesupesno.\nGreska: %d\n", WSAGetLastError());
            closesocket(klijentskiSoket);
            WSACleanup();
            return 1;
        }

        printf("Poruka uspesno poslata serveru. Ukupno bajta: %ld\n", iResult);

		// Prijem poruke koju salje server
        iResult = recv(klijentskiSoket, dataBuffer, BUFFER_SIZE, 0);
	
		// Check if message is successfully received
        if (iResult > 0)	
        {
			// Pristup u memoriji sadrzaju poruke i adekvatno kastovanje pokazivaca
            int* primljenaVrednost = (int*) dataBuffer; 

			// Primena funkcije ntohl() jer je poruka u mreznom redosledu 
            int zbir = ntohl(*primljenaVrednost); 
            printf("\nIzracunata suma je: %d\n", zbir);
        }
		// Check if shutdown command is received
        else if (iResult == 0)	
        {
            // Connection was closed successfully
            printf("Veza sa serverom je zatvorena.\n");
            break;
        }
		// Greska pri prijemu
        else	
        {
            printf("Prijem poruke od servera neuspesan.\nGreska: %d\n", WSAGetLastError());
            break;
        }

        printf("\nZa prekid slanja unite 'exit', a za nastavak pritisnite bilo koji taster.\n");
        gets_s(dataBuffer, BUFFER_SIZE);
        if (!strcmp(dataBuffer, "exit"))
		{
            break;
		}
    }

    // Shutdown the connection since we're done
    iResult = shutdown(klijentskiSoket, SD_BOTH);

    // Check if connection is succesfully shut down.
    if (iResult == SOCKET_ERROR)
    {
        printf("Shutdown klijenta neuspesan.\nGreska: %d\n", WSAGetLastError());
        closesocket(klijentskiSoket);
        WSACleanup();
        return 1;
    }

    // For demonstration purpose
    printf("\nPritisni bilo koji taster za izlaz: ");
    _getch();

    // Close connected socket
    closesocket(klijentskiSoket);

    // Deinitialize WSA library
    WSACleanup();

    return 0;
}
