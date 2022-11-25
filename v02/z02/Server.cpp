// UDP server
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

#define SERVER_PORT 15001
#define BUFFER_SIZE 512		

int main()
{
	sockaddr_in adresaServera;

	int velicinaAdreseServera = sizeof(adresaServera);

	char dataBuffer[BUFFER_SIZE];

	int iResult;

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Pokretanje biblioteke neuspesno.\nGRESKA: %d\n", WSAGetLastError());
		return 1;
	}

	memset((char*) &adresaServera, 0, velicinaAdreseServera);
	adresaServera.sin_addr.s_addr = INADDR_ANY; 
	adresaServera.sin_family = AF_INET;
	adresaServera.sin_port = htons(SERVER_PORT);

	SOCKET serverskaUticnica = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serverskaUticnica == INVALID_SOCKET)
	{
		printf("Stvaranje uticnice za server neuspesno.\nGRESKA: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	iResult = bind(serverskaUticnica, (SOCKADDR*) &adresaServera, velicinaAdreseServera);
	if (iResult == SOCKET_ERROR)
	{
		printf("Neuspesno povezivanje serverske uticnice sa adresom.\nGRESKA: %d\n", WSAGetLastError());
		closesocket(serverskaUticnica);
		WSACleanup();
		return 1;
	}

	printf("UDP server je pokrenut i ceka poruke od klijenata.\n");

	int velika_slova, mala_slova, ostali_karakteri;
	char prethodni[BUFFER_SIZE];

	while (true)
	{
		sockaddr_in adresaKlijenta;
		memset(&adresaKlijenta, 0, velicinaAdreseServera);

		memset(dataBuffer, 0, BUFFER_SIZE);

		iResult = recvfrom(serverskaUticnica, dataBuffer, BUFFER_SIZE, 0, (SOCKADDR*) &adresaKlijenta, &velicinaAdreseServera);
		if (iResult == SOCKET_ERROR)
		{
			printf("Server nije uspeo da preuzme poruku od klijenta.\nGRESKA: %d\n", WSAGetLastError());
			continue;
		}

		dataBuffer[iResult] = '\0';
		
		// U HEADER-U UDP PORUKE SE NALAZE IP ADRESA I PORT POŠILJAOCA
		// SAD TO IZVLAČIMO, DA BI SMO ZNALI GDE DA ODGOVORIMO

		// 15 spaces for decimal notation (for example: "192.168.100.200") + '\0'
		char ipAdresa[16]; 

		strcpy_s(ipAdresa, sizeof(ipAdresa), inet_ntoa(adresaKlijenta.sin_addr));
		unsigned short klijentPort = ntohs(adresaKlijenta.sin_port);

		velika_slova = mala_slova = ostali_karakteri = 0;
		for (int i = 0; i < strlen(dataBuffer); i++)
		{
			// 65 = A, 90 = Z
			if (65 <= dataBuffer[i] && dataBuffer[i] <= 90)
			{
				++velika_slova;
			}
			// 97 = a, 122 = z
			else if (97 <= dataBuffer[i] && dataBuffer[i] <= 122)
			{
				++mala_slova;
			}
			else
			{
				++ostali_karakteri;
			}
		}

		printf("Klijent spojen sa IP: %s, port: %d, poslao: %s.\n", ipAdresa, klijentPort, dataBuffer);

		printf("Poruka ima %d karaktera.\n", iResult);
		printf("Poruka ima %d velikih slova.\n", velika_slova);
		printf("Poruka ima %d malih slova.\n", mala_slova);
		printf("Poruka ima %d ostalih karaktera.\n", ostali_karakteri);

		if (!strcmp(prethodni, dataBuffer))
		{
			printf("Klijent je uzastopno poslao dve identicne poruke. Server se gasi.\n");
			break;
		}

		strcpy(prethodni, dataBuffer);

		iResult = sendto(serverskaUticnica, dataBuffer, strlen(dataBuffer), 0, (SOCKADDR*) &adresaKlijenta, velicinaAdreseServera);
		if (iResult == SOCKET_ERROR)
		{
			printf("Slanje poruke od servera ka klijentu neuspesno.\nGRESKA: %d\n", WSAGetLastError());
			continue;
		}
	}

	iResult = closesocket(serverskaUticnica);
	if (iResult == SOCKET_ERROR)
	{
		printf("Zatvaranje serverske uticnice neuspesno.\nGRESKA: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	printf("Server je uspesno ugasen.\n");

	WSACleanup();

	return 0;
}
