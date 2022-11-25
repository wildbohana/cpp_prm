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

#define SERVER_PORT 18010
#define BUFFER_SIZE 256

int main()
{
    SOCKET serverskaUticnica = INVALID_SOCKET;

    SOCKET prihvacenaUticnica1 = INVALID_SOCKET;
    SOCKET prihvacenaUticnica2 = INVALID_SOCKET;

    int iResult;

    char dataBuffer[BUFFER_SIZE];

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("Pokretanje winsock biblioteke neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        return 1;
    }

    sockaddr_in adresaServera;
    memset((char*) &adresaServera, 0, sizeof(adresaServera));

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

	// Postavljanje serverske uticnice u rezim prisluskivanja
    iResult = listen(serverskaUticnica, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("Postavljanje serverske uticnice u rezim prisluskivanja neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        closesocket(serverskaUticnica);
        WSACleanup();
        return 1;
    }

	printf("Uticnica servera je u rezimu slusanja. Cekanje na nove zahteve za povezivanje.\n");

    do
    {
        sockaddr_in adresaKlijenta;
        int velicinaAdreseKlijenta = sizeof(adresaKlijenta);

        // Prihvat veze sa prvim klijentom 
        prihvacenaUticnica1 = accept(serverskaUticnica, (SOCKADDR*) &adresaKlijenta, velicinaAdreseKlijenta);
        if (prihvacenaUticnica1 == INVALID_SOCKET)
        {
            printf("Prihvatanje klijenta nesupesno.\nGRESKA: %d\n", WSAGetLastError());
            closesocket(serverskaUticnica);
            WSACleanup();
            return 1;
        }

        printf("\nNovi klijent je prihvacen. Adresa klijenta: %s : %d\n", inet_ntoa(adresaKlijenta.sin_addr), ntohs(adresaKlijenta.sin_port));

        // Prihvat veze sa drugim klijentom 
        prihvacenaUticnica2 = accept(serverskaUticnica, (SOCKADDR*) &adresaKlijenta, &velicinaAdreseKlijenta);
        if (prihvacenaUticnica2 == INVALID_SOCKET)
        {
            printf("Prihvatanje klijenta nesupesno: %d\n", WSAGetLastError());
            closesocket(serverskaUticnica);
            WSACleanup();
            return 1;
        }

        printf("\nNovi klijent je prihvacen. Adresa klijenta: %s : %d\n", inet_ntoa(adresaKlijenta.sin_addr), ntohs(adresaKlijenta.sin_port));

		// Postavljanje uticnica namenjenih klijentima u neblokirajuci rezim (mode = 1)
        unsigned long mode = 1; 

        iResult = ioctlsocket(prihvacenaUticnica1, FIONBIO, &mode);
        if (iResult != NO_ERROR)
		{
            printf("ioctlsocket neuspesan.\nGRESKA: %ld\n", iResult);
		}

        iResult = ioctlsocket(prihvacenaUticnica2, FIONBIO, &mode);
        if (iResult != NO_ERROR)
		{
            printf("ioctlsocket neuspesan.\nGRESKA: %ld\n", iResult);
		}

		// Dodato za niz brojeva za koji se racuna suma
        int* primljenNiz;
        int suma = 0;

		// Polling model prijema poruka
        do
        {
            // Prijem poruke na prvoj uticnici
            iResult = recv(prihvacenaUticnica1, dataBuffer, BUFFER_SIZE, 0);
            if (iResult > 0)	
            {
                primljenNiz = (int*) dataBuffer;

                printf("Klijent br. 1 je poslao: %d %d %d.\n", ntohl(primljenNiz[0]), ntohl(primljenNiz[1]), ntohl(primljenNiz[2]));
				
				// Kad citamo iz poruke, treba nam ntohl() - racunamo sumu
                for (int i = 0; i < 3; i++)
				{
                    suma += ntohl(primljenNiz[i]);
				}

                // Priprema podatka za slanje, treba nam  htonl()
                suma = htonl(suma);

                // Slanje poruke sa sumom ka prvom klijentu
                iResult = send(prihvacenaUticnica1, (char*) &suma, sizeof(int), 0);
                if (iResult == SOCKET_ERROR)
                {
                    printf("Slanje poruke ka klijentu neuspesno.\nGRESKA: %d\n", WSAGetLastError());
                    closesocket(prihvacenaUticnica1);
                    break;
                }

				// Ponovno postavljanje na 0, za naredno izracunavanje nove sume brojeva
                suma = 0; 
            }
			// Proveri da li je stigla komanda za shutdown
            else if (iResult == 0)	
            {
                printf("Veza sa klijentom je zatvorena.\n");
                closesocket(prihvacenaUticnica1);
                break;
            }
			// U neblokirajucem rezimu funkcija se cesto neuspesno izvrsi jer nije spreman, pa bi zelela da blokira program
            else	
            {
                if (WSAGetLastError() == WSAEWOULDBLOCK) 
				{
                    // U pitanju je blokirajuca operacija tj. poruka jos nije stigla
                }
                else 
				{
                    // Desila se neka druga greska prilikom poziva operacije
                    printf("Poruka od klijenta nije stigla.\nGRESKA: %d\n", WSAGetLastError());
                    closesocket(prihvacenaUticnica1);
                    closesocket(prihvacenaUticnica2);
                    break;
                }
            }

            // Prijem poruke na drugoj uticnici
            iResult = recv(prihvacenaUticnica2, dataBuffer, BUFFER_SIZE, 0);
            if (iResult > 0)	
            {
                primljenNiz = (int*) dataBuffer;

                printf("Klijent br. 2 je poslao: %d %d %d.\n", ntohl(primljenNiz[0]), ntohl(primljenNiz[1]), ntohl(primljenNiz[2]));
				
				// Kad citamo iz poruke, treba nam ntohl()
                for (int i = 0; i < 3; i++)
				{
                    suma += ntohl(primljenNiz[i]); 
				}

                // Priprema podatka za slanje, treba nam htonl()
                suma = htonl(suma);

                // Slanje sume ka drugom klijentu
                iResult = send(prihvacenaUticnica2, (char*) &suma, sizeof(int), 0);
                if (iResult == SOCKET_ERROR)
                {
                    printf("Slanje poruke ka klijentu neuspesno.\nGRESKA: %d\n", WSAGetLastError());
                    closesocket(prihvacenaUticnica2);
                    break;
                }

                suma = 0;
            }
			// Proveri da li je stigla poruka za gasenje
            else if (iResult == 0)	
            {
                printf("Veza sa klijentom je zatvorena.\n");
                closesocket(prihvacenaUticnica2);
                break;
            }
			// U neblokirajucem rezimu funkcija se cesto neuspesno izvrsi jer nije spreman, pa bi zelela da blokira program
            else	
            {
                if (WSAGetLastError() == WSAEWOULDBLOCK) 
				{
                    // U pitanju je blokirajuca operacija tj. poruka jos nije stigla
                    Sleep(1500);
                }
                else 
				{
                    // Desila se neka druga greska prilikom poziva operacije
                    printf("Poruka od klijenta nije stigla.\nGRESKA: %d\n", WSAGetLastError());
                    closesocket(prihvacenaUticnica2);
                    closesocket(prihvacenaUticnica1);
                    break;
                }
            }

        } while (true);

    } while (true);

    closesocket(serverskaUticnica);
    closesocket(prihvacenaUticnica1);
    closesocket(prihvacenaUticnica2);

    WSACleanup();

    return 0;
}
