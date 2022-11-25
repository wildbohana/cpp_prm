// TCP klijent, blokirajuce uticnice
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

int main()
{
    int iResult;

    char dataBuffer[BUFFER_SIZE];

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
		printf("Pokretanje winsock biblioteke neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        return 1;
    }

    SOCKET klijentskaUticnica = INVALID_SOCKET;

    klijentskaUticnica = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (klijentskaUticnica == INVALID_SOCKET)
    {
		printf("Otvaranje klijentske uticnice neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    sockaddr_in adresaServera;
    adresaServera.sin_family = AF_INET;
    adresaServera.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    adresaServera.sin_port = htons(SERVER_PORT);

    if (connect(klijentskaUticnica, (SOCKADDR*) &adresaServera, sizeof(adresaServera)) == SOCKET_ERROR)
    {
		printf("Spajanje korisnicke uticnice sa adresom servera neuspesno.\n");
        closesocket(klijentskaUticnica);
        WSACleanup();
        return 1;
    }

    int nizBrojeva[3];

    while (true)
    {
        // Ucitavanje niza od 3 broja
        for (int i = 0; i < 3; i++)
        {
            printf("\nUnesite broj za slanje: ");
            gets_s(dataBuffer, BUFFER_SIZE);
            nizBrojeva[i] = atoi(dataBuffer);

			// Priprema podatka za slanje - treba nam htonl()
            nizBrojeva[i] = htonl(nizBrojeva[i]); 
        }

        // Saljemo nizBrojeva prosledjujuci njegovu adresu i duzinu poruke (niza) u bajtima
        iResult = send(klijentskaUticnica, (char*) nizBrojeva, 3 * sizeof(int), 0);
        if (iResult == SOCKET_ERROR)
        {
			printf("Slanje poruke serveru neuspesno.\nGRESKA: %d\n", WSAGetLastError());
            closesocket(klijentskaUticnica);
            WSACleanup();
            return 1;
        }

        printf("Poruka uspesno poslata serveru. Ukupno bajta: %ld\n", iResult);

		// Prijem poruke koju salje server
        iResult = recv(klijentskaUticnica, dataBuffer, BUFFER_SIZE, 0);
        if (iResult > 0)	
        {
            int* primljenaVrednost = (int*) dataBuffer; 

			// Primena funkcije ntohl() jer je poruka u mreznom redosledu 
            int zbir = ntohl(*primljenaVrednost); 
            printf("\nIzracunata suma je: %d\n", zbir);
        }
		// Check if shutdown command is received
        else if (iResult == 0)	
        {
            printf("Veza sa serverom je zatvorena.\n");
            break;
        }
		// Greska pri prijemu
        else	
        {
			printf("Prijem poruke od servera neuspesan.\nGRESKA: %d\n", WSAGetLastError());
            break;
        }

		// Provera da li klijent zeli da ugasi program
        printf("\nZa prekid slanja unesite 'exit', a za nastavak pritisnite bilo koji taster.\n");
        gets_s(dataBuffer, BUFFER_SIZE);
        if (!strcmp(dataBuffer, "exit"))
		{
            break;
		}
    }

    // Gašenje veze - preko shutdown
    iResult = shutdown(klijentskaUticnica, SD_BOTH);
    if (iResult == SOCKET_ERROR)
    {
		printf("Gasenje klijentske uticnice neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        closesocket(klijentskaUticnica);
        WSACleanup();
        return 1;
    }

	printf("Pritisnite bilo koji taster za izlaz...");
    _getch();

    closesocket(klijentskaUticnica);

    WSACleanup();

    return 0;
}
