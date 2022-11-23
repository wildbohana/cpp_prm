// UDP server that use blocking sockets
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

// DVE SERVERSKE UTIČNICE UMESTO JEDNE
#define SERVER_PORT1 17010 
#define SERVER_PORT2 17011

// Size of buffer that will be used for sending and receiving messages to clients
#define BUFFER_SIZE 512		

int main()
{
    // Dve adresne strukture za svaku serversku uticnicu
    sockaddr_in serverAddress1, serverAddress2;

    // Buffer we will use to send and receive clients' messages
    char dataBuffer[BUFFER_SIZE];

    // WSADATA data structure that is to receive details of the Windows Sockets implementation
    WSADATA wsaData;

    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup neuspesan.\nGreska: %d\n", WSAGetLastError());
        return 1;
    }

	// Popunjavanje adresnih struktura, kreiranje dve uticnice i
	// njihovo povezivanje sa adresnim podacima u tim strukturama

    // Initialize serverAddress1 and serverAddress2 structures used by bind function
    memset((char*) &serverAddress1, 0, sizeof(serverAddress1));
    serverAddress1.sin_family = AF_INET; 
    serverAddress1.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress1.sin_port = htons(SERVER_PORT1);	// 17010 port

    memset((char*) &serverAddress2, 0, sizeof(serverAddress2));
    serverAddress2.sin_family = AF_INET;
    serverAddress2.sin_addr.s_addr = INADDR_ANY;
    serverAddress2.sin_port = htons(SERVER_PORT2);	// 17011 port

    // Create first UDP socket
    SOCKET serverSoket1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Check if socket creation succeeded
    if (serverSoket1 == INVALID_SOCKET)
    {
        printf("Neuspesno stvaranje soketa.\nGreska: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Create second UDP socket
    SOCKET serverSoket2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Check if socket creation succeeded
    if (serverSoket2 == INVALID_SOCKET)
    {
        printf("Neuspesno stvaranje soketa.\nGreska: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Povezujemo prvu uticnicu sa njenom adresnom strukturom 
    int iResult = bind(serverSoket1, (SOCKADDR*) &serverAddress1, sizeof(serverAddress1));

    // Check if socket is succesfully binded to server datas
    if (iResult == SOCKET_ERROR)
    {
        printf("bind uticnice neuspesan.\nGreska: %d\n", WSAGetLastError());
        closesocket(serverSoket1);
        WSACleanup();
        return 1;
    }

    // Povezujemo drugu uticnicu sa njenom adresnom strukturom 
    iResult = bind(serverSoket2, (SOCKADDR*) &serverAddress2, sizeof(serverAddress2));

    // Check if socket is succesfully binded to server address data
    if (iResult == SOCKET_ERROR)
    {
        printf("bind uticnice neuspesan.\nGreska: %d\n", WSAGetLastError());
        closesocket(serverSoket2);
        WSACleanup();
        return 1;
    }

	printf("UDP server je pokrenut i ceka na poruke od klijenata.\n");

    // Declare client address that will be set from recvfrom
    sockaddr_in adresaKlijenta;
    // size of client address structure
    int duzinaAdreseKlijenta = sizeof(adresaKlijenta);

	// Stavljamo uticnice u neblokirajuci rezim (mode = 1)
    unsigned long mode = 1;
    if (ioctlsocket(serverSoket1, FIONBIO, &mode) != 0 
	||  ioctlsocket(serverSoket2, FIONBIO, &mode) != 0)
    {
        printf("ioctlsocket neuspesan.\nGreska: %d\n", WSAGetLastError());
        closesocket(serverSoket1);
        closesocket(serverSoket2);
        WSACleanup();
        return 1;
    }

	// Pomocne promenljive - detekcija pristigle poruke, brojac poena za prvog i drugog klijenta
    bool primljen1 = false, primljen2 = false; 
    int poeni1 = poeni2 = 0; 

	// Polling model prijema poruka na ove dve uticnice
    while (1)
    {
        // Initialize client address structure in memory
        memset(&adresaKlijenta, 0, sizeof(adresaKlijenta));

        // Set whole buffer to zero
        memset(dataBuffer, 0, BUFFER_SIZE);

        // Prijem poruke sa prve uticnice
        iResult = recvfrom
			(serverSoket1,				// Own socket
            dataBuffer,					// Buffer that will be used for receiving message
            BUFFER_SIZE,				// Maximal size of buffer
            0,							// No flags
            (SOCKADDR*)&adresaKlijenta,	// Client information from received message (ip address and port)
            &duzinaAdreseKlijenta);		// Size of sockadd_in structure

        // Provera da li je uspesno primljena poruka
        if (iResult != SOCKET_ERROR)
        {
            // Set end of string
            dataBuffer[iResult] = '\0';

            // Ispis primljene poruke na ekran
            printf("Serverska uticnica br. 1 je primila: %s.\n", dataBuffer);
			
			// Prvi klijent je poslao poruku i ona je primljena
            primljen1 = true; 
            
			// Ako je stigla poruka od 1. klijenta, ali ne i od 2. klijenta
            if (primljen1 && !primljen2)
            {
                poeni1++;
                printf("Prvi klijent je prvi poslao poruku i ima %d poena.\n", poeni1);
            }
        }
		// iResult == SOCKET_ERROR tj. desila se greska
        else  
        {
            // Obavezno proveriti da li je operacija vratila WSAEWOULDBLOCK
            // To oznacava da bi operacija recvfrom() blokirala programa
			// jer jos nije spremna da se izvrsi tj. poruka nije stigla
            if (WSAGetLastError() == WSAEWOULDBLOCK) 
			{
                // Pusticemo da program tece dalje jer imamo pokusaj 
                // prijema poruke i na 2. uticnici, tamo cemo pozvati Sleep
            }
            else 
			{
                // Desila se neka druga greska prilikom prijema poruke
                // izaci cemo iz petlje, zatvoriti uticnice i ugasiti program
                break;
            }
        }

        // Prijem poruke sa druge uticnice
        iResult = recvfrom
			(serverSoket2,					// Own socket
            dataBuffer,						// Buffer that will be used for receiving message
            BUFFER_SIZE,					// Maximal size of buffer
            0,								// No flags
            (SOCKADDR*) &adresaKlijenta,	// Client information from received message (ip address and port)
            &duzinaAdreseKlijenta);			// Size of sockadd_in structure

        // Provera da li je uspesno primljena poruka
        if (iResult != SOCKET_ERROR)
        {
            // Set end of string
            dataBuffer[iResult] = '\0';

            // Ispis primljene poruke na ekran
            printf("Serverska uticnica br. 2 je primila: %s.\n", dataBuffer);
			
			// Drugi klijent je poslao poruku i ona je primljena
            primljen2 = true; 
            
			// Ako je stigla poruka od 2. klijenta, ali ne i od 1. klijenta
            if (!primljen1 && primljen2) 
            {
                poeni2++;
                printf("Drugi klijent je prvi poslao poruku i ima %d poena.\n", poeni2);
            }
        }
		// iResult == SOCKET_ERROR tj. desila se greska
        else  
        {
            // Obavezno proveriti da li je operacija vratila WSAEWOULDBLOCK
            // To oznacava da bi operacija recvfrom() blokirala programa
            // jer jos nije spremna da se izvrsi tj. poruka nije stigla
            if (WSAGetLastError() == WSAEWOULDBLOCK) 
			{
				// Cekamo do narednog pokusaja prijema poruke 1.5s
                Sleep(1500); 
            }
            else 
			{
                // Desila se neka druga greska prilikom prijema poruke
                // Izaci cemo iz petlje, zatvoriti uticnice i ugasiti program
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

    // Close server application - za oba soketa
    iResult = closesocket(serverSoket1);
    if (iResult == SOCKET_ERROR)
    {
        printf("closesocket neuspesan.\nGreska: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    iResult = closesocket(serverSoket2);
    if (iResult == SOCKET_ERROR)
    {
        printf("closesocket neuspesan.\nGreska: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    printf("Server je uspesno ugasen.\n");

    // Close Winsock library
    WSACleanup();

    return 0;
}
