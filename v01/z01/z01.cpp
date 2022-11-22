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

int main()
{
	// 1. Inicijalizovati windosk biblioteku
	WSADATA wsaData;
	int rezultat_inicijalizacije = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (rezultat_inicijalizacije != NO_ERROR)
	{
		printf("Greska pri pozivu WSAStartup().\n");
		return 1;
	}
	else
	{
		printf("WSA biblioteka je uspesno incijalizovana.\n");
	}

	// 2. Kreirati adresnu strukturu koja koristi 
	// ip adresu 127.0.0.1 i broj porta 5555
	unsigned short port = 5555;
	const char* ip = "127.0.0.1";

	sockaddr_in adresaServera;
	adresaServera.sin_family = AF_INET;
	adresaServera.sin_addr.s_addr = inet_addr(ip);
	adresaServera.sin_port = htons(port);

	// 3. Kreirati jedan TCP soket (utičnicu)
	SOCKET serverSoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSoket == INVALID_SOCKET)
	{
		printf("Greska pri pozivu socket().\n");
		WSACleanup();
		return 1;
	}
	else
	{
		printf("\nTCP soket je uspesno napravljen.\n");
	}

	// 4. Povezati soket sa kreiranom adresnom strukturom
	if (bind(serverSoket, (SOCKADDR*) &adresaServera, sizeof(adresaServera)) == SOCKET_ERROR)
	{
		int kodGreske = WSAGetLastError();
		printf("Neuspesan poziv funkcije bind().\n");
		printf("ERROR: %d\n", kodGreske);

		closesocket(serverSoket);
		WSACleanup();
		return 0;
	}
	else
	{
		printf("\nSoket je uspesno spojen sa adresom.\n");
	}

	// 5. Zatvoriti soket
	closesocket(serverSoket);
	// 6. Zatvoriti winsock bibilioteku
	WSACleanup();

	return 0;
}
