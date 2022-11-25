// UDP server, neblokirajuce uticnice
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

#define SERVER_PORT1 15011
#define SERVER_PORT2 15012

#define BUFFER_SIZE 512

int main()
{
	sockaddr_in adresaServera1;
	sockaddr_in adresaServera2;

	char dataBuffer[BUFFER_SIZE];

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		printf("Pokretanje winsock biblioteke neuspesno.\nGRESKA: %d\n", WSAGetLastError);
		return 1;
	}

	// Server ima dva porta - pravimo dve adrese
	memset((char*) &adresaServera1, 0, sizeof(adresaServera1));
	adresaServera1.sin_family = AF_INET;
	adresaServera1.sin_addr.s_addr = INADDR_ANY;
	adresaServera1.sin_port = htons(SERVER_PORT1);

	memset((char*) &adresaServera2, 0, sizeof(adresaServera2));
	adresaServera2.sin_family = AF_INET;
	adresaServera2.sin_addr.s_addr = inet_addr("127.0.0.1");
	adresaServera2.sin_port = htons(SERVER_PORT2);

	// Otvaranje prve uticnice
	SOCKET serverskaUticnica1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serverskaUticnica1 == INVALID_SOCKET)
	{
		printf("Otvaranje serverske uticnice neuspesno.\nGRESKA: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Otvaranje druge uticnice
	SOCKET serverskaUticnica2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serverskaUticnica2 == INVALID_SOCKET)
	{
		printf("Otvaranje serverske uticnice neuspesno.\nGRESKA: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Povezivanje prve uticnice za adresu
	int iResult = bind(serverskaUticnica1, (SOCKADDR*) &adresaServera1, sizeof(adresaServera1));
	if (iResult == SOCKET_ERROR)
	{
		printf("Neuspelo povezivanje uticnice sa adresom.\nGRESKA: %d\n", WSAGetLastError());
		closesocket(serverskaUticnica1);
		WSACleanup();
		return 1;
	}

	// Povezivanje druge uticnice za adresu
	iResult = bind(serverskaUticnica2,(SOCKADDR*) &adresaServera2, sizeof(adresaServera2));
	if (iResult == SOCKET_ERROR)
	{
		printf("Neuspelo povezivanje uticnice sa adresom.\nGRESKA: %d\n", WSAGetLastError());
		closesocket(serverskaUticnica2);
		WSACleanup();
		return 1;
	}

	// Podesavanje obe uticnice u neblokirajuci rezim (mode = 1)
	unsigned long mode = 1;
	if (ioctlsocket(serverskaUticnica1, FIONBIO, &mode) != 0 
	||  ioctlsocket(serverskaUticnica2, FIONBIO, &mode) != 0)
	{
		printf("Prebacivanje uticnica u neblokirajuci rezim neuspesno.\nGRESKA: %d\n", WSAGetLastError());
		closesocket(serverskaUticnica1);
		closesocket(serverskaUticnica2);
		WSACleanup();
		return 1;
	}

	printf("UDP server je pokrenut. Cekaju se poruke od klijenata.\n");

	while (true)
	{
		sockaddr_in adresaKlijenta;
		int duzinaAdreseKlijenta = sizeof(adresaKlijenta);
		memset(&adresaKlijenta, 0, duzinaAdreseKlijenta);

		memset(dataBuffer, 0, BUFFER_SIZE);

		// Set deskriptora za čitanje bafera
		fd_set fdCitanje;
		FD_ZERO(&fdCitanje);

		// Dodavanje uticnica u set fdCitanje
		FD_SET(serverskaUticnica1, &fdCitanje);
		FD_SET(serverskaUticnica2, &fdCitanje);

		// Set deskriptora za čitanje bafera - za izuzetke
		fd_set fdIzuzeci;
		FD_ZERO(&fdIzuzeci);

		// Dodavanje uticnica u set fdIzuzeci
		FD_SET(serverskaUticnica1, &fdIzuzeci);
		FD_SET(serverskaUticnica2, &fdIzuzeci);

		// Cekanje da nova poruka stigne u neku od uticnica u fdCitanje setu
		// i cekanje za moguće izuzetke u fdIzuzeci setu
		int sResult = select(0, &fdCitanje, NULL, &fdIzuzeci, NULL);

		if (sResult == SOCKET_ERROR)
		{
			printf("Operacija select neuspesna.\nGRESKA: %d\n", WSAGetLastError());
			break;
		}
		else if (sResult > 0)
		{
			// Ako je rezultat pozitivan, paket je stigao na jedan ili više utičnica, ili negde postoji izuzetak
			// Broj rezultata je broj uticnica na kojima se desio događaj
			unsigned long brojPorta = 0;

			// Proveravamo da li se događaj desio na prvoj uticnici
			if (FD_ISSET(serverskaUticnica1, &fdCitanje))
			{
				iResult = recvfrom(serverskaUticnica1, dataBuffer, BUFFER_SIZE, 0, (SOCKADDR*) &adresaKlijenta, duzinaAdreseKlijenta);
				brojPorta = SERVER_PORT1;

				if (iResult != SOCKET_ERROR)
				{
					dataBuffer[iResult] = '\0';

					printf("Server je primio poruku na %d portu. Klijent je poslao: %s.\n", brojPorta, dataBuffer);
				}
				else
				{
					printf("Prijem poruke od klijenta neuspesan.\nGRESKA: %d\n", WSAGetLastError());
					continue;
				}
			}

			// Proveravamo da li se događaj desio na drugoj uticnici
			if (FD_ISSET(serverskaUticnica2, &fdCitanje))
			{
				iResult = recvfrom(serverskaUticnica2, dataBuffer, BUFFER_SIZE, 0, (SOCKADDR*) &adresaKlijenta, &duzinaAdreseKlijenta);
				brojPorta = SERVER_PORT2;

				if (iResult != SOCKET_ERROR)
				{
					dataBuffer[iResult] = '\0';

					printf("Server je primio poruku na %d portu. Klijent je poslao: %s.\n", brojPorta, dataBuffer);
				}
				else
				{
					printf("Prijem poruke od klijenta neuspesan.\nGRESKA: %d\n", WSAGetLastError());
					continue;
				}
			}

			// Proveravamo da li se izuzetak desio na uticnicama
			if (FD_ISSET(serverskaUticnica1, &fdIzuzeci))
			{
				break;
			}
			if (FD_ISSET(serverskaUticnica2, &fdIzuzeci))
			{
				break;
			}
			else
			{
				continue;
			}
		}
	}

	if (SOCKET_ERROR == closesocket(serverskaUticnica1) 
	||  SOCKET_ERROR == closesocket(serverskaUticnica2))
	{
		printf("Zatvarnje serverskih uticnica neuspesno.\nGRESKA: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	printf("Server je uspesno ugasen.\n");

	WSACleanup();

	return 0;
}
