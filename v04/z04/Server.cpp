// TCP server, blokirajuce uticnice
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
    SOCKET serverskaUticnica = INVALID_SOCKET;

    SOCKET prihvacenaUticnica[2];
    prihvacenaUticnica[0] = INVALID_SOCKET;
    prihvacenaUticnica[1] = INVALID_SOCKET;

    int iResult;

    char dataBuffer[BUFFER_SIZE];

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("Pokretanje winsock biblioteke neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        return 1;
    }

    sockaddr_in adresaServera;
    memset((char*)&adresaServera, 0, sizeof(adresaServera));

    adresaServera.sin_family = AF_INET;	
    adresaServera.sin_addr.s_addr = INADDR_ANY;	
    adresaServera.sin_port = htons(SERVER_PORT);

    serverskaUticnica = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverskaUticnica == INVALID_SOCKET)
    {
        printf("Otvaranje serverske uticnice neuspesno.\nGRESKA: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    iResult = bind(serverskaUticnica, (SOCKADDR*) &adresaServera, sizeof(adresaServera));
    if (iResult == SOCKET_ERROR)
    {
        printf("Povezivanje serverske uticnice sa adresom neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        closesocket(serverskaUticnica);
        WSACleanup();
        return 1;
    }

    iResult = listen(serverskaUticnica, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("Postavljanje serverske uticnice u rezim prisluskivanja neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        closesocket(serverskaUticnica);
        WSACleanup();
        return 1;
    }

	unsigned long mode = 1; 
    iResult = ioctlsocket(serverskaUticnica, FIONBIO, &mode);
    if (iResult != NO_ERROR)
	{
        printf("Postavljanje serverske uticnice u neblokirajuci rezim neuspesno.\nGreska: %ld\n", iResult);
	}

	printf("Serverska uticnica je u rezimu prisluskivanja. Cekanje na nove zahteve za povezivanje.\n");
	
	// Brojac koliko klijenata je ostvarilo vezu sa serverom
    int povezano = 0;

    do
    {
        sockaddr_in adresaKlijenta;
        int velicinaAdreseKlijenta = sizeof(struct sockaddr_in);
        
        // Pozivamo accept() za prijem konekcije 
        prihvacenaUticnica[povezano] = accept(serverskaUticnica, (SOCKADDR*) &adresaKlijenta, &velicinaAdreseKlijenta);

        // Proveramo da li se accept() funkcija uspesno izvrsila
        if (prihvacenaUticnica[povezano] != INVALID_SOCKET)
        {
            // Ispis poruke o uspesnoj konekciji
            printf("\nPrihvacen novi zahtev od klijenta. Adresa klijenta: %s : %d\n", inet_ntoa(adresaKlijenta.sin_addr), ntohs(adresaKlijenta.sin_port));

            // Postavljanje uticnica namenjenih klijentima u neblokirajuci rezim (mode = 1)
            unsigned long mode = 1; 
            iResult = ioctlsocket(prihvacenaUticnica[povezano], FIONBIO, &mode);

            if (iResult != NO_ERROR)
			{
                printf("ioctlsocket neuspesan.\nGreska: %ld\n", iResult);
			}
			
			// Uvecavamo brojac spojenih klijenata
			povezano++;
        }
		// Funkcija je vratila INVALID_SOCKET
        else 
        {
            // Obavezno proveriti da li je razlog WSAEWOULDBLOCK
            if (WSAGetLastError() == WSAEWOULDBLOCK) 
			{
                Sleep(2000);
            }
			// Ili je neka druga greska zbog koje cemo ugasiti server
            else  
            {
                printf("Neuspesno spajanje sa klijentom.\nGRESKA: %d\n", WSAGetLastError());
                closesocket(serverskaUticnica);
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
	
        int* primljenNiz; 
		int max = 0; 
        
        // Polling model prijema poruka
        do
        {
            // For petljom prolazimo kroz niz prihvacenaUticnica od dve uticnice 
            // i proveravamo da li su primile poruku primenom polling modela 
            for (int i = 0; i < 2; i++)
            {
                iResult = recv(prihvacenaUticnica[i], dataBuffer, BUFFER_SIZE, 0);
				
				// Poruka je uspesno primljena
                if (iResult > 0)
                {
                	// Pristupamo adresi gde je smestena primljena poruka, adekvatno kastujemo pokazivac
                    primljenNiz = (int*) dataBuffer;
                    printf("Klijent br. [%d] je poslao: %d %d %d.\n", i+1, ntohl(primljenNiz[0]), ntohl(primljenNiz[1]), ntohl(primljenNiz[2]));
                    
                    // Pronalazenje najvece vrednosti, inicijalno uzimamo prvi element niza
					// kad citamo iz poruke (sadrzaj je u mreznom formatu), pa nam treba ntohl()
                    max = ntohl(primljenNiz[0]);

					// Klasična pretraga najvećeg elementa u nizu
                    for (int i = 1; i < 3; i++)
                    {
                        if (ntohl(primljenNiz[i]) > max)  
						{
                            max = ntohl(primljenNiz[i]); 
						}
                    }
                     
                    // Priprema poruke o pronadjenoj max vrednosti 
                    sprintf_s(dataBuffer, "Najveci poslati broj je %d.", max);
                    
					// Slanje poruke ka klijentu
                    iResult = send(prihvacenaUticnica[i], dataBuffer, strlen(dataBuffer), 0);

                    // Check result of send function
                    if (iResult == SOCKET_ERROR)
                    {
                        printf("Slanje poruke ka klijentu je neuspesno.\nGRESKA: %d\n", WSAGetLastError());
                        closesocket(prihvacenaUticnica[i]);
                        povezano--;
                        break;
                    } 
                }
				// Da li je poslata komanda za gasenje
                else if (iResult == 0)	
                {
                    printf("Veza sa klijentom je uspesno zatvorena.\n");
                    closesocket(prihvacenaUticnica[i]);
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
                        printf("Poruka od klijenta nije stigla.\nGreska: %d\n", WSAGetLastError());
                        closesocket(prihvacenaUticnica[i]);
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

    closesocket(serverskaUticnica);
    closesocket(prihvacenaUticnica[0]);
    closesocket(prihvacenaUticnica[1]);

    WSACleanup();

    return 0;
}
