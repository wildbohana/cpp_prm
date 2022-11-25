// UDP klijent, neblokirajuce uticnice
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

    char dataBuffer[BUFFER_SIZE];

    WSADATA wsaData;
    
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    	if (iResult != 0)
    {
        printf("Pokretanje winsock biblioteke neuspesno.\nGRESKA: %d\n", iResult);
        return 1;
    }

    memset((char*) &adresaServera, 0, sizeof(adresaServera));		
    
	// Izbor broja porta za server
	printf("Unesi broj porta servera (15011 or 15012):\n");
    gets_s(dataBuffer, BUFFER_SIZE);
	unsigned long serverPort = atoi(dataBuffer);

	adresaServera.sin_family = AF_INET;	
    adresaServera.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    adresaServera.sin_port = htons(serverPort);	

    SOCKET klijentskaUticnica = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (klijentskaUticnica == INVALID_SOCKET)
    {
		printf("Otvaranje klijentske uticnice neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

   	while (true)
   	{	
		printf("Unesite poruku za slanje:\n");
    	gets_s(dataBuffer, BUFFER_SIZE);

		// Zatvori klijenta ako je poruka za kraj uneta
    	if (!strcmp(dataBuffer, "end"))
    	{
       		printf("Klijent je zavrsio. Zatvara se.\n");
	    	break;
    	}
	
		// Slanje poruke serveru
		iResult = sendto(klijentskaUticnica, dataBuffer, strlen(dataBuffer), 0, (SOCKADDR*) &adresaServera, sizeof(adresaServera));
		if (iResult == SOCKET_ERROR)
    	{
			printf("Slanje poruke serveru neuspesno.\nGRESKA: %d\n", WSAGetLastError());
	        closesocket(klijentskaUticnica);
	        WSACleanup();
	        return 1;
    	}
   	}

	printf("Pritisni bilo koji taster za kraj... ");
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
