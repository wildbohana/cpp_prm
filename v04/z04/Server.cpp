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

#define SERVER_PORT 19010
#define BUFFER_SIZE 256

// TCP server that use blocking sockets
int main()
{
    // Socket used for listening for new clients 
    SOCKET serverSoket = INVALID_SOCKET;

    // Socket used for communication with client
    SOCKET prihvaceniSoket[2];
    prihvaceniSoket[0] = INVALID_SOCKET;
    prihvaceniSoket[1] = INVALID_SOCKET;

    // Variable used to store function return value
    int iResult;

    // Buffer used for storing incoming data
    char dataBuffer[BUFFER_SIZE];

    // WSADATA data structure that is to receive details of the Windows Sockets implementation
    WSADATA wsaData;

    // Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup neuspesan.\nGreska: %d\n", WSAGetLastError());
        return 1;
    }

    // Initialize adresaServera structure used by bind
    sockaddr_in adresaServera;
    memset((char*)&adresaServera, 0, sizeof(adresaServera));
    adresaServera.sin_family = AF_INET;	
    adresaServera.sin_addr.s_addr = INADDR_ANY;	
    adresaServera.sin_port = htons(SERVER_PORT);

    // Create a SOCKET for connecting to server
    serverSoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Check if socket is successfully created
    if (serverSoket == INVALID_SOCKET)
    {
        printf("Otvaranje server soketa neuspesno.\nGreska: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket - bind port number and local address to socket
    iResult = bind(serverSoket, (struct sockaddr*) &adresaServera, sizeof(adresaServera));

    // Check if socket is successfully binded to address and port from sockaddr_in structure
    if (iResult == SOCKET_ERROR)
    {
        printf("bind utcnice za server nesupesno.\nGreska: %d\n", WSAGetLastError());
        closesocket(serverSoket);
        WSACleanup();
        return 1;
    }

    // Set serverSoket in listening mode
    iResult = listen(serverSoket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen neuspesan.\nGreska: %d\n", WSAGetLastError());
        closesocket(serverSoket);
        WSACleanup();
        return 1;
    }

    // Stavljanje uticnice serverSoket u neblokirajuci rezim (mode = 1)
	unsigned long mode = 1; 
    iResult = ioctlsocket(serverSoket, FIONBIO, &mode);
    if (iResult != NO_ERROR)
	{
        printf("ioctlsocket neuspesan.\nGreska: %ld\n", iResult);
	}

	printf("Serverska uticnica je u rezimu prisluskivanja. Cekanje na nove zahteve za povezivanje.\n");
	
	// Brojac koliko klijenata je ostvarilo vezu sa serverom
    int povezano = 0;

    do
    {
        // Struct for information about povezano client
        sockaddr_in adresaKlijenta;
        int velicinaAdreseKlijenta = sizeof(struct sockaddr_in);
        
        // pozivamo accept() za prijem konekcije 
        prihvaceniSoket[povezano] = accept(serverSoket, (struct sockaddr*) &adresaKlijenta, &velicinaAdreseKlijenta);

        // proveramo da li se accept() funkcija uspesno izvrsila
        if (prihvaceniSoket[povezano] != INVALID_SOCKET)
        {
            // Ispis poruke o uspesnoj konekciji
            printf("\nPrihvacen novi zahtev od klijenta. Adresa klijenta: %s : %d\n", inet_ntoa(adresaKlijenta.sin_addr), ntohs(adresaKlijenta.sin_port));

            // Postavljanje uticnica namenjenih klijentima u neblokirajuci rezim (mode = 1)
            unsigned long mode = 1; 
            iResult = ioctlsocket(prihvaceniSoket[povezano], FIONBIO, &mode);

            if (iResult != NO_ERROR)
			{
                printf("ioctlsocket neuspesan.\nGreska: %ld\n", iResult);
			}
			
			// Uvecavamo brojac konektovanih klijenata
			povezano++;
        }
		// Funkcija je vratila INVALID_SOCKET
        else 
        {
            // Obavezno proveriti da li je razlog WSAEWOULDBLOCK
            // tj. zahtev za vezom jos nije stigao
            if (WSAGetLastError() == WSAEWOULDBLOCK) 
			{
                Sleep(2000);
            }
			// Ili je neka druga greska zbog koje cemo ugasiti server
            else  
            {
                printf("accept failed with error: %d\n", WSAGetLastError());
                closesocket(serverSoket);
                WSACleanup();
                return 1;
            }
        }
		
		// Vracamo se na pocetak petlje dok se ne spoje 2 klijenta
        if (povezano < 2) 
        {
            continue;
        }
        
		// Kad se spoje 2 klijenta program nastavlje dalje
        // Ocekuje se razmena poruka sa ta dva klijenta
	
		// Pokazivac na primljen niz celobrojnih vrednosti
        int* primljenNiz; 
		// Maksimalni (najveci) broj iz primljenog niza
        int max = 0; 
        
        // Polling model prijema poruka
        do
        {
            // For petljom prolazimo kroz niz prihvaceniSoket od dve uticnice 
            // i proveravamo da li su primile poruku primenom polling modela 
            for (int i = 0; i < 2; i++)
            {
                iResult = recv(prihvaceniSoket[i], dataBuffer, BUFFER_SIZE, 0);
				
				// Poruka je uspesno primljena
                if (iResult > 0)	
                {
                	// Pristupamo adresi gde je smestena primljena poruka, adekvatno kastujemo pokazivac
                    primljenNiz = (int*)dataBuffer;
                    printf("Klijent br. [%d] je poslao: %d %d %d.\n", i+1, ntohl(primljenNiz[0]), ntohl(primljenNiz[1]), ntohl(primljenNiz[2]));
                    
                    // Pronalazenje najvece vrednosti, inicijalno uzimamo prvi element niza
					// kad citamo iz poruke (sadrzaj je u mreznom formatu), pa nam treba ntohl()
                    max = ntohl(primljenNiz[0]);

					// Klasična pretraga najvećeg elementa u nizu
                    for (int i = 1; i < 3; i++)
                    {
                        if (ntohl(primljenNiz[i])> max)  
						{
                            max = ntohl(primljenNiz[i]); 
						}
                    }
                     
                    // Priprema poruke o pronadjenoj max vrednosti 
                    sprintf_s(dataBuffer, "Najveci poslati broj je %d.", max);
                    
					// Slanje poruke ka klijentu
                    iResult = send(prihvaceniSoket[i], dataBuffer, strlen(dataBuffer), 0);

                    // Check result of send function
                    if (iResult == SOCKET_ERROR)
                    {
                        printf("Slanje poruke ka klijentu je neuspesno.\nGreska: %d\n", WSAGetLastError());
                        closesocket(prihvaceniSoket[i]);
                        povezano--;
                        break;
                    }   
                }
				// Check if shutdown command is received
                else if (iResult == 0)	
                {
                    // Connection was closed successfully
                    printf("Veza sa klijentom je uspesno zatvorena.\n");
                    closesocket(prihvaceniSoket[i]);
                    povezano--;
                    break;
                }
				// U neblokirajucem rezimu funkcija se cesto neuspesno izvrsi jer nije spremna, pa bi zelela da blokira program
                else	
                {
                    if (WSAGetLastError() == WSAEWOULDBLOCK) 
					{
                        // U pitanju je blokirajuca operacija tj. poruka jos nije primljena
                    }
                    else 
					{
                        // Desila se neka druga greska prilikom poziva operacije
                        printf("Poruka od klijenta nije stigla.\nGreska: %d\n", WSAGetLastError());
                        closesocket(prihvaceniSoket[i]);
                        povezano--;
                        break;
                    }
                }
            }
           	
			// Kad smo proverili na obe uticnice, do naredne iteracije pauziramo 1s
            Sleep(1000); 
			
			// Ako se desila greska i jedna ili obe uticnice  su zatvorene prekidamo petlju prijema poruka 
            if (povezano < 2) 
            {
				break;
			}
        } while (true);
		
		// Resetujemo brojac povezanih za nova dva klijenta sa kojima ce se ostvariti veza
        povezano = 0;

    } while (true);

    // Close listen and accepted sockets
    closesocket(serverSoket);
    closesocket(prihvaceniSoket[0]);
    closesocket(prihvaceniSoket[1]);

    // Deinitialize WSA library
    WSACleanup();

    return 0;
}
