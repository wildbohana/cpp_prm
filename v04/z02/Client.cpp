// UDP klijent, blokirajuce uticnice
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
    sockaddr_in adresaServera;
    int velicinaAdreseServera = sizeof(adresaServera);

    char dataBuffer[BUFFER_SIZE];

    int iResult;

    WSADATA wsaData;
	
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
		printf("Pokretanje winsock bibilioteke neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        return 1;
    }

    memset((char*) &adresaServera, 0, velicinaAdreseServera);

	// Učitati iz komandne linije port servera ka kome će se slati poruke
    printf("Unesite port servera (17010 ili 17011): ");
    gets_s(dataBuffer, BUFFER_SIZE);
    unsigned short serverPort = atoi(dataBuffer);

    adresaServera.sin_family = AF_INET;	
    adresaServera.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    adresaServera.sin_port = htons(serverPort);

    SOCKET klijentskaUticnica = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (klijentskaUticnica == INVALID_SOCKET)
    {
		printf("Stvaranje klijentske uticnice neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    while (true)
    {
        printf("Unesi poruku za slanje:\n");
        gets_s(dataBuffer, BUFFER_SIZE);

        if (strcmp(dataBuffer, "kraj") == 0)
        {
            printf("Klijent odustaje od daljeg slanja i zatvara se.\n");
            break;
        }

        iResult = sendto(klijentskaUticnica, dataBuffer, strlen(dataBuffer), 0, (SOCKADDR*) &adresaServera, velicinaAdreseServera);

        if (iResult == SOCKET_ERROR)
        {
            printf("Slanje poruke serveru neuspesno.\nGreska: %d\n", WSAGetLastError());
            closesocket(klijentskaUticnica);
            WSACleanup();
            return 1;
        }
    }

	printf("Pritisnite bilo koji taster za izlaz iz programa...");
    _getch();

    iResult = closesocket(klijentskaUticnica);
    if (iResult == SOCKET_ERROR)
    {
		printf("Zatvaranje klijentske uticnice neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    WSACleanup();

    return 0;
}
