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
	// 1. Inicijalizovati winsock biblioteku
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR)
	{
		printf("Greska pri inicijalizaciji winsock biblioteke.\n");
		return 1;
	}
	else
	{
		printf("Bibilioteka uspesno pokrenuta.\n");
	}

	// 2. Kreirati adresnu strukturu koja koristi 
	// ip adresu 127.0.0.1 i broj porta 5555
	int port = 5555;
	char* ip = "127.0.0.1";

	sockaddr_in = adresaServera;
	adresaServera.sin_family = AF_INET;
	adresaServera.sin_port = htons(port);
	adresaServera.sin_addr.s_addr = inet_addr(ip);

	// 3. Kreirati jedan TCP soket (utičnicu)
	SOCKET serverSoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSoket == INVALID_SOCKET)
	{
		printf("Greska pri pravljenju uticnice za server.\n");
		WSACleanup();
		return 1;
	}
	else
	{
		printf("Uticnica za server je uspesno napravljena.\n");
	}
	
	// 4. Povezati soket sa kreiranom adresnom strukturom
	if (bind(serverSocket, (SOCKADDR*) &adresaServera, sizeof(adresaServera)) == SOCKET_ERROR)
	{
		int kodGreske = WSAGetLastError();
		printf("Neuspesno spajanje uticnice sa adresom.\n");
		printf("GRESKA: %d\n", kodGreske);

		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		printf("Uticnica je uspesno spojena sa adresom.\n");
	}

	// 5. Zatvoriti soket
	closesocket(serverSoket);
	// 6. Zatvoriti winsock bibilioteku
	WSACleanup();

	return 0;
}
