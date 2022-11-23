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

// TCP server that use blocking sockets
int main()
{
    // Socket used for listening for new clients 
    SOCKET serverskiSoket = INVALID_SOCKET;

    // Sockets used for communication with client
    SOCKET prihvaceniSoket1 = INVALID_SOCKET;
    SOCKET prihvaceniSoket2 = INVALID_SOCKET;

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

    // Initialize serverAddress structure used by bind
    sockaddr_in serverAddress;
    memset((char*) &serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;	
    serverAddress.sin_addr.s_addr = INADDR_ANY;	
    serverAddress.sin_port = htons(SERVER_PORT);

    // Create a SOCKET for connecting to server
    serverskiSoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Check if socket is successfully created
    if (serverskiSoket == INVALID_SOCKET)
    {
        printf("Otvaranje serverskog soketa neuspesno.\nGreska: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket - bind port number and local address to socket
    iResult = bind(serverskiSoket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));

    // Check if socket is successfully binded to address and port from sockaddr_in structure
    if (iResult == SOCKET_ERROR)
    {
        printf("bind neuspesan.\nGreska: %d\n", WSAGetLastError());
        closesocket(serverskiSoket);
        WSACleanup();
        return 1;
    }

    // Set serverskiSoket in listening mode
    iResult = listen(serverskiSoket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen neuspesan.\nGreska: %d\n", WSAGetLastError());
        closesocket(serverskiSoket);
        WSACleanup();
        return 1;
    }

	printf("Uticnica servera je u rezimu slusanja. Cekanje na nove zahteve za spajanje.\n");

    do
    {
        // Struct for information about connected client
        sockaddr_in adresaKlijenta;

        int adresaKlijentaSize = sizeof(struct sockaddr_in);

        // Prihvat veze sa 1. klijentom 
        prihvaceniSoket1 = accept(serverskiSoket, (struct sockaddr*) &adresaKlijenta, &adresaKlijentaSize);

        // Check if accepted socket is valid 
        if (prihvaceniSoket1 == INVALID_SOCKET)
        {
            printf("Prihvatanje klijenta nesupesno.\nGreska: %d\n", WSAGetLastError());
            closesocket(serverskiSoket);
            WSACleanup();
            return 1;
        }

        printf("\nNovi klijent je prihvacen. Adresa klijenta: %s : %d\n", inet_ntoa(adresaKlijenta.sin_addr), ntohs(adresaKlijenta.sin_port));

        // Prihvat veze sa 2. klijentom 
        prihvaceniSoket2 = accept(serverskiSoket, (struct sockaddr*) &adresaKlijenta, &adresaKlijentaSize);

        // Check if accepted socket is valid 
        if (prihvaceniSoket2 == INVALID_SOCKET)
        {
            printf("Prihvatanje klijenta nesupesno: %d\n", WSAGetLastError());
            closesocket(serverskiSoket);
            WSACleanup();
            return 1;
        }

        printf("\nNovi klijent je prihvacen. Adresa klijenta: %s : %d\n", inet_ntoa(adresaKlijenta.sin_addr), ntohs(adresaKlijenta.sin_port));

		// Postavljanje uticnica namenjenih klijentima u neblokirajuci rezim (mode = 1)
        unsigned long mode = 1; 
        iResult = ioctlsocket(prihvaceniSoket1, FIONBIO, &mode);
        if (iResult != NO_ERROR)
		{
            printf("ioctlsocket neuspesan.\nGreska: %ld\n", iResult);
		}

        iResult = ioctlsocket(prihvaceniSoket2, FIONBIO, &mode);
        if (iResult != NO_ERROR)
		{
            printf("ioctlsocket neuspesan.\nGreska: %ld\n", iResult);
		}

		// DODATO ZA POLLING MODEL
        int* primljenNiz;
        int suma = 0;

		// Polling model prijema poruka
        do
        {
            // Prijem poruke na prvoj uticnici
            iResult = recv(prihvaceniSoket1, dataBuffer, BUFFER_SIZE, 0);

			// Check if message is successfully received
            if (iResult > 0)	
            {
                // dataBuffer[iResult] = '\0';
                primljenNiz = (int*)dataBuffer;

                // printf("Klijent br. 1 je poslao: %s.\n", dataBuffer);
                printf("Klijent br. 1 je poslao: %d %d %d.\n", ntohl(primljenNiz[0]), ntohl(primljenNiz[1]), ntohl(primljenNiz[2]));
				
				// Kad citamo iz poruke (sadrzaj je u mreznom formatu), treba nam ntohl()
                for (int i = 0; i < 3; i++)
				{
                    suma += ntohl(primljenNiz[i]); 
				}

                // Priprema podatka za slanje, treba nam konverzija iz host u network zapis tj. htonl()
                suma = htonl(suma);

                // Slanje poruke ka prvom klijentu
                iResult = send(prihvaceniSoket1, (char*)&suma, sizeof(int), 0);

                // Check result of send function
                if (iResult == SOCKET_ERROR)
                {
                    printf("Slanje poruke ka klijentu neuspesno.\nGreska: %d\n", WSAGetLastError());
                    closesocket(prihvaceniSoket1);
                    break;
                }

				// Ponovno postavljanje na 0, za naredno izracunavanje nove sume brojeva
                suma = 0; 
            }
			// Check if shutdown command is received
            else if (iResult == 0)	
            {
                // Connection was closed successfully
                printf("Veza sa klijentom je zatvorena.\n");
                closesocket(prihvaceniSoket1);
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
                    printf("Poruka od klijenta nije stigla.\nGreska: %d\n", WSAGetLastError());
                    closesocket(prihvaceniSoket1);
                    closesocket(prihvaceniSoket2);
                    break;
                }
            }

            // Prijem poruke na drugoj uticnici
            iResult = recv(prihvaceniSoket2, dataBuffer, BUFFER_SIZE, 0);
			
			// Check if message is successfully received
            if (iResult > 0)	
            {
                primljenNiz = (int*)dataBuffer;

                // printf("Klijent br. 2 je poslao: %s.\n", dataBuffer);
                printf("Klijent br. 2 je poslao: %d %d %d.\n", ntohl(primljenNiz[0]), ntohl(primljenNiz[1]), ntohl(primljenNiz[2]));
				
				// Kad citamo iz poruke (sadrzaj je u mreznom formatu), treba nam ntohl()
                for (int i = 0; i < 3; i++)
				{
                    suma += ntohl(primljenNiz[i]); 
				}

                // Priprema podatka za slanje, treba nam konverzija iz host u network zapis tj. htonl()
                suma = htonl(suma);

                // Slanje ka drugom klijentu
                iResult = send(prihvaceniSoket2, (char*) &suma, sizeof(int), 0);

                // Check result of send function
                if (iResult == SOCKET_ERROR)
                {
                    printf("Slanje poruke ka klijentu neuspesno.\nGreska: %d\n", WSAGetLastError());
                    closesocket(prihvaceniSoket2);
                    break;
                }
                suma = 0;
            }
			// Check if shutdown command is received
            else if (iResult == 0)	
            {
                // Connection was closed successfully
                printf("Veza sa klijentom je zatvorena.\n");
                closesocket(prihvaceniSoket2);
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
                    printf("Poruka od klijenta nije stigla.\nGreska: %d\n", WSAGetLastError());
                    closesocket(prihvaceniSoket2);
                    closesocket(prihvaceniSoket1);
                    break;
                }
            }
			
        } while (true);

    } while (true);

    // Close listen and accepted sockets
    closesocket(serverskiSoket);
    closesocket(prihvaceniSoket1);
    closesocket(prihvaceniSoket2);

    // Deinitialize WSA library
    WSACleanup();

    return 0;
}
