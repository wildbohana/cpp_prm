// TCP klijent
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
#define SERVER_PORT 27016
#define BUFFER_SIZE 256

// TCP client that uses blocking sockets
int main()
{
	SOCKET klijentskaUticnica = INVALID_SOCKET;

	int iResult;

	char dataBuffer[BUFFER_SIZE];

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Pokretanje winsock biblioteke neuspesno.\nGRESKA: %d\n", WSAGetLastError());
		return 1;
	}

	klijentskaUticnica = socket(AF_INET, SOCK_STREAM,	IPPROTO_TCP);
	if (klijentskaUticnica == INVALID_SOCKET)
	{
		printf("Stvaranje klijentske uticnice neuspesno.\nGRESKA: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	sockaddr_in adresaServera;
	adresaServera.sin_family = AF_INET;
	adresaServera.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
	adresaServera.sin_port = htons(SERVER_PORT);

	if (connect(klijentskaUticnica, (SOCKADDR*) &adresaServera, sizeof(adresaServera)) == SOCKET_ERROR)
	{
		printf("Spajanje uticnice sa adresom servera nije uspelo.\n");
		closesocket(klijentskaUticnica);
		WSACleanup();
		return 1;
	}

	// DODATO - provera da li je došlo do kraja (koja stiže od servera)
	bool kraj = false;

	do {
		iResult = recv(klijentskaUticnica, dataBuffer, BUFFER_SIZE, 0);
		if (iResult > 0)	
		{
			dataBuffer[iResult] = '\0';

			printf("Server je poslao: %s.\n", dataBuffer);

			// strstr pronalazi podstring u datom stringu
			if (strstr(dataBuffer, "POBEDILI") 
			||  strstr(dataBuffer, "IZGUBILI"))
			{
				kraj = true;
			}
		}
		// Proveri da li je komanda za gasenje poslata
		else if (iResult == 0)	
		{
			printf("Veza sa serverom je zatvorena.\n");
			closesocket(klijentskaUticnica);
			WSACleanup();
			return 0;
		}
		// Greska prilikom preuzimanja poruke
		else	
		{
			printf("Primanje poruke od servera neuspesno.\nGRESKA: %d\n", WSAGetLastError());
			closesocket(klijentskaUticnica);
			WSACleanup();
			return 1;
		}

		if (kraj)
		{
			break;
		}

		// Klijent salje rec na zadato slovo
		printf("Posalji rec na odabrano slovo za slanje: ");
		gets_s(dataBuffer, BUFFER_SIZE);

		iResult = send(klijentskaUticnica, dataBuffer, (int) strlen(dataBuffer), 0);
		if (iResult == SOCKET_ERROR)
		{
			printf("Slanje poruke serveru neuspesno.\nGRESKA: %d\n", WSAGetLastError());
			closesocket(klijentskaUticnica);
			WSACleanup();
			return 1;
		}

	} while (true);

	// Gasenje veze sa serverom	
	iResult = shutdown(klijentskaUticnica, SD_BOTH);
	if (iResult == SOCKET_ERROR)
	{
		printf("Zatvaranje korisnicke uticnice neuspesno.\nGRESKA: %d\n", WSAGetLastError());
		closesocket(klijentskaUticnica);
		WSACleanup();
		return 1;
	}

	printf("Pritisnite bilo koji taster za izlaz iz programa... ");
	_getch();

	closesocket(klijentskaUticnica);

	WSACleanup();

	return 0;
}
