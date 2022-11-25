// UDP server, blokirajuce uticnice
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

#define SERVER_PORT1 17010 
#define SERVER_PORT2 17011

#define BUFFER_SIZE 512		

int main()
{
    // Dve adresne strukture za svaku serversku uticnicu
    sockaddr_in adresaServera1, adresaServera2;

    char dataBuffer[BUFFER_SIZE];

	int iResult;

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
		printf("Pokretanje winsock biblioteke neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        return 1;
    }

    memset((char*) &adresaServera1, 0, sizeof(adresaServera1));
    adresaServera1.sin_family = AF_INET; 
    adresaServera1.sin_addr.s_addr = inet_addr("127.0.0.1");
    adresaServera1.sin_port = htons(SERVER_PORT1);		// 17010 port

    memset((char*) &adresaServera2, 0, sizeof(adresaServera2));
    adresaServera2.sin_family = AF_INET;
    adresaServera2.sin_addr.s_addr = INADDR_ANY;
    adresaServera2.sin_port = htons(SERVER_PORT2);		// 17011 port

    SOCKET serverskaUticnica1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverskaUticnica1 == INVALID_SOCKET)
    {
		printf("Otvaranje serverske uticnice neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    SOCKET serverskaUticnica2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverskaUticnica2 == INVALID_SOCKET)
    {
		printf("Otvaranje serverske uticnice neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Povezujemo prvu uticnicu sa njenom adresnom strukturom 
    iResult = bind(serverskaUticnica1, (SOCKADDR*) &adresaServera1, sizeof(adresaServera1));
    if (iResult == SOCKET_ERROR)
    {
		printf("Povezivanje serverske uticnice sa adresom neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        closesocket(serverskaUticnica1);
        WSACleanup();
        return 1;
    }

    // Povezujemo drugu uticnicu sa njenom adresnom strukturom 
    iResult = bind(serverskaUticnica2, (SOCKADDR*) &adresaServera2, sizeof(adresaServera2));
    if (iResult == SOCKET_ERROR)
    {
        printf("bind uticnice neuspesan.\nGreska: %d\n", WSAGetLastError());
        closesocket(serverskaUticnica2);
        WSACleanup();
        return 1;
    }

	printf("UDP server je pokrenut i ceka na poruke od klijenata.\n");

    sockaddr_in adresaKlijenta;
    int duzinaAdreseKlijenta = sizeof(adresaKlijenta);

	// Stavljamo uticnice u neblokirajuci rezim (mode = 1)
    unsigned long mode = 1;
    if (ioctlsocket(serverskaUticnica1, FIONBIO, &mode) != 0 
	||  ioctlsocket(serverskaUticnica2, FIONBIO, &mode) != 0)
    {
		printf("Prebacivanje servera u neblokirajuci rezim neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        closesocket(serverskaUticnica1);
        closesocket(serverskaUticnica2);
        WSACleanup();
        return 1;
    }

	// Pomocne promenljive - detekcija pristigle poruke, brojac poena za prvog i drugog klijenta
    bool primljen1 = false, primljen2 = false; 
    int poeni1 = poeni2 = 0; 

	// Polling model prijema poruka na ove dve uticnice
    while (true)
    {
        memset(&adresaKlijenta, 0, sizeof(adresaKlijenta));

        memset(dataBuffer, 0, BUFFER_SIZE);

        // Prijem poruke sa prve uticnice
    	iResult = recvfrom(serverskaUticnica1, dataBuffer, BUFFER_SIZE, 0, (SOCKADDR*) &adresaKlijenta, &duzinaAdreseKlijenta);
	    if (iResult != SOCKET_ERROR)
        {
            dataBuffer[iResult] = '\0';
            printf("Serverska uticnica 1 je primila: %s.\n", dataBuffer);
			
            primljen1 = true; 

            if (primljen1 && !primljen2)
            {
                poeni1++;
                printf("Prvi klijent je prvi poslao poruku i ima %d poena.\n", poeni1);
            }
        }
        else  
        {
            // Obavezno proveriti da li je operacija vratila WSAEWOULDBLOCK
            if (WSAGetLastError() == WSAEWOULDBLOCK) 
			{
                // Pusticemo da program tece dalje u uticnici 1, ali cemo u uticnici 2 pozvari sleep
            }
            else 
			{
                // Desila se neka druga greska prilikom prijema poruke, gasimo ceo server (zato ide break)
                break;
            }
        }

        // Prijem poruke sa druge uticnice
        iResult = recvfrom(serverskaUticnica2, dazaBuffer, BUFFER_SIZE, 0, (SOCKADDR*) &adresaKlijenta, duzinaAdreseKlijenta);
        if (iResult != SOCKET_ERROR)
        {
            dataBuffer[iResult] = '\0';
            printf("Serverska uticnica 2 je primila: %s.\n", dataBuffer);
			
            primljen2 = true; 
            
            if (!primljen1 && primljen2) 
            {
                poeni2++;
                printf("Drugi klijent je prvi poslao poruku i ima %d poena.\n", poeni2);
            }
        }
        else  
        {
            // Obavezno proveriti da li je operacija vratila WSAEWOULDBLOCK
            if (WSAGetLastError() == WSAEWOULDBLOCK) 
			{
				// Cekamo do narednog pokusaja prijema poruke 1.5s
                Sleep(1500); 
            }
            else 
			{
                // Desila se neka druga greska prilikom prijema poruke, gasimo server
                break;
            }
        }
		
		// Ako su stigle poruke od oba klijenta, za naredni ciklus provere resetujemo promenjlive 
        if (primljen1 && primljen2) 
        {
            primljen1 = false;
            primljen2 = false;
        }
    }

	// Zatvaramo prvu uticnicu
    iResult = closesocket(serverskaUticnica1);
    if (iResult == SOCKET_ERROR)
    {
		printf("Zatvaranje serverske uticnice neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

	// Zatvaramo drugu uticnicu
    iResult = closesocket(serverskaUticnica2);
    if (iResult == SOCKET_ERROR)
    {
		printf("Zatvaranje serverske uticnice neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    printf("Server je uspesno ugasen.\n");

    WSACleanup();

    return 0;
}
