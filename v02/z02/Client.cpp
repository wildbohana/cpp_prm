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
#define SERVER_PORT 15001
#define BUFFER_SIZE 512

int main()
{
	sockaddr_in adresaServera;

	int duzinaAdreseServera = sizeof(adresaServera);

	char dataBuffer[BUFFER_SIZE];

	unsigned short serverPort = SERVER_PORT;

	WSADATA wsaData;

	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("Pokretanje winsock biblioteke neuspesno.\nGRESKA: %d\n", iResult);
		return 1;
	}

	memset((char*) &adresaServera, 0, duzinaAdreseServera);
	adresaServera.sin_family = AF_INET;
	adresaServera.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
	adresaServera.sin_port = htons(serverPort);

	SOCKET klijentskaUticnica = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (klijentskaUticnica == INVALID_SOCKET)
	{
		printf("Pravljenje klijentske uticnice nije uspelo.\nGRESKA: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	
	char prethodna_poruka[BUFFER_SIZE];

	while (true)
	{
		printf("Unesi poruku za slanje: ");
		gets_s(dataBuffer, BUFFER_SIZE);

		if (!strcmp(dataBuffer, "stop client"))
		{
			break;
		}

		iResult = sendto(klijentskaUticnica, dataBuffer, strlen(dataBuffer), 0, (SOCKADDR*) &adresaServera, sizeof(adresaServera));
		if (iResult == SOCKET_ERROR)
		{
			printf("Slanje poruke ka serveru neuspeno.\nGRESKA: %d\n", WSAGetLastError());
			closesocket(klijentskaUticnica);
			WSACleanup();
			return 1;
		}

		if (!strcmp(prethodna_poruka, dataBuffer))
		{
			printf("Dve identicne poruke su poslate uzastopno. Server se gasi.\n");
			break;
		}

		strcpy(prethodna_poruka, dataBuffer);

		iResult = recvfrom(klijentskaUticnica, dataBuffer, BUFFER_SIZE, 0, (SOCKADDR*) &adresaServera, &duzinaAdreseServera);
		if (iResult == SOCKET_ERROR)
		{
			printf("Primanje poruke od servera neuspesno.\nGRESKA: %d\n", WSAGetLastError());
			continue;
		}

		dataBuffer[iResult] = '\0';

		printf("Poruka od servera: %s.\n\n", dataBuffer);
	}

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
