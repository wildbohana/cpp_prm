// TCP server, neblokirajuce uticnice
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

#pragma pack(1)

#define SERVER_PORT 27016
#define BUFFER_SIZE 256
#define MAX_CLIENTS 3

// STRUKTURA ZA ZADATAK
struct studentInfo{
	char ime [15];
	char prezime [20];
	short poeni;
};

int main() 
{
	SOCKET uticnicaServera = INVALID_SOCKET;

	SOCKET uticniceKlijenata[MAX_CLIENTS];
	short poslednjiIndeks = 0;

	int iResult;

	char dataBuffer[BUFFER_SIZE];

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		printf("Pokretanje winsock biblioteke neuspesno.\nGRESKA: %d\n", WSAGetLastError());
		return 1;
	}

	sockaddr_in adresaServera;
	memset((char*) &adresaServera, 0, sizeof(adresaServera));

	adresaServera.sin_family = AF_INET;
	adresaServera.sin_addr.s_addr = INADDR_ANY;
	adresaServera.sin_port = htons(SERVER_PORT);

	memset(uticniceKlijenata, 0, MAX_CLIENTS * sizeof(SOCKET));

	uticnicaServera = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (uticnicaServera == INVALID_SOCKET)
	{
		printf("Otvaranje serverske uticnice neuspesno.\nGRESKA: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Not sure, ali mislim da ovde bas mora (struct sockaddr*)
	iResult = bind(uticnicaServera,(SOCKADDR*) &adresaServera, sizeof(adresaServera));
	if (iResult == SOCKET_ERROR)
	{
		printf("Povezivanje serverske uticnice sa adresom neuspesno.\nGRESKA: %d\n", WSAGetLastError());
		closesocket(uticnicaServera);
		WSACleanup();
		return 1;
	}
	
	// Sve veze se automatski spajaju ako je uticnica u listen rezimu
	// Sa SO_CONDITIONAL_ACCEPT parametrom koji je true, veze se neće automatski spojiti
	bool uslov = true;
	int duzinaUslova = sizeof(bool);
	
	iResult = setsockopt(uticnicaServera, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char*) &uslov, duzinaUslova);
	if (iResult == SOCKET_ERROR)
	{
	    printf("Podesavanje parametra za serversku uticnicu neuspesno.\nGRESKA: %u\n", WSAGetLastError());
	}

	unsigned long mode = 1;
	if (ioctlsocket(uticnicaServera, FIONBIO, &mode) != 0)
	{
		printf("Promena rezima uticnice neuspesan.\nGRESKA: %d\n", WSAGetLastError());
		closesocket(uticnicaServera);
		WSACleanup();
		return 1;
	}

	iResult = listen(uticnicaServera, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("Postavljanje servera u rezim prisluskivanja neuspesan.\nGRESKA: %d\n", WSAGetLastError());
		closesocket(uticnicaServera);
		WSACleanup();
		return 1;
	}

	printf("Serverska uticnica je u rezimu prisluskivanja. Cekanje na nove zahteve za povezivanje.\n");

	// Set descriptora za uticnice
	fd_set fdCitanje;

	// Timeout za funkcije
	timeval vreme;
	vreme.tv_sec = 1;
	vreme.tv_usec = 0;
	
	// Struktura za studenta
	studentInfo *student;

	while (true)
	{
		// Inicijalizacija seta za uticnicu
		FD_ZERO(&fdCitanje);

		// Dodavanje serverskih uticnica u set
		if (poslednjiIndeks != MAX_CLIENTS)
		{
			FD_SET(uticnicaServera, &fdCitanje);
		}

		// Dodavanje klijentskih uticnica u set
		for (int i = 0; i < poslednjiIndeks; i++) 
		{
			FD_SET(uticniceKlijenata[i], &fdCitanje);
		}

		// Cekanje da se dese događaji
		int selectResult = select(0, &fdCitanje, NULL, NULL, &vreme);
		
		if (selectResult == SOCKET_ERROR)
		{
			printf("Select neuspesan.\nGRESKA: %d\n", WSAGetLastError());
			closesocket(uticnicaServera);
			WSACleanup();
			return 1;
		}
		else if (selectResult == 0) 
		{
			// Isteklo vreme; proveri da li je neki taster pritisnut
			if (_kbhit()) 
			{
				getch();
				printf("PRMuIS 2022/2023\n");
			}
			continue;
		}
		else if (FD_ISSET(uticnicaServera, &fdCitanje)) 
		{
			// Klijent se povezao
			sockaddr_in adresaKlijenta;
			int velicinaAdreseKlijenta = sizeof(struct sockaddr_in);

			// Novi zahtev za povezivanje. Dodaj novu uticnicu na prvo slobodno mesto u nizu
			uticniceKlijenata[poslednjiIndeks] = accept(uticnicaServera, (SOCKADDR*) &adresaKlijenta, &velicinaAdreseKlijenta);

			if (uticniceKlijenata[poslednjiIndeks] == INVALID_SOCKET)
			{
				if (WSAGetLastError() == WSAECONNRESET)
				{
					printf("Prihvatanje neuspesno, jer je timeout za zahtev klijenta istekao.\n");
				}
				else
				{
					printf("Prihvatanje neuspesno.\nGRESKA: %d\n", WSAGetLastError());
				}
			}
			else
			{
				if (ioctlsocket(uticniceKlijenata[poslednjiIndeks], FIONBIO, &mode) != 0)
				{
					printf("Podesavanje uticnice neuspesno.\n");
					continue;
				}

				poslednjiIndeks++;
				printf("Prihvacen novi zahtev klijenta (%d). Adresa klijenta: %s : %d\n", 
				poslednjiIndeks, inet_ntoa(adresaKlijenta.sin_addr), ntohs(adresaKlijenta.sin_port));
			}
		}
		else
		{
			// STIZE NOVA PORUKA
			// Provera da li je nova poruka stigla od nekog od povezanih klijenata
			for (int i = 0; i < poslednjiIndeks ; i++) 
			{
				// Da li je poruka poslata od klijenta na poziciji "i"
				if (FD_ISSET(uticniceKlijenata[i], &fdCitanje)) 
				{
					iResult = recv(uticniceKlijenata[i], dataBuffer, BUFFER_SIZE, 0);

					if (iResult > 0)
					{
						dataBuffer[iResult] = '\0';
						printf("Poruka je primljena od klijenta (%d):\n", i+1);
						
						// Primljenoj poruci u memoriji čemo pristupiti preko pokazivaca tipa (studentInfo *)
						// Jer znamo format u kom je poruka poslata a to je struct studentInfo
						student = (studentInfo*) dataBuffer;

						printf("Ime i prezime: %s %s  \n", student->ime, student->prezime);
						printf("Poeni studenta: %d  \n", ntohs(student->poeni));
						printf("_______________________________  \n");		
					}
					else if (iResult == 0)
					{
						// Veza je zatvorena
						printf("Veza sa klijentom (%d) je zatvorena.\n", i+1);
						closesocket(uticniceKlijenata[i]);

						// Sortiraj niz i oslobodi poslednje mesto
						for (int j = i; j < poslednjiIndeks - 1; j++) 
						{
							uticniceKlijenata[j] = uticniceKlijenata[j + 1];
						}
						uticniceKlijenata[poslednjiIndeks - 1] = 0;

						poslednjiIndeks--;
					}
					else
					{
						printf("Poruka od klijenta nije primljena.\nGRESKA: %d\n", WSAGetLastError());
						closesocket(uticniceKlijenata[i]);

						// Sortiraj niz i oslobodi poslednje mesto
						for (int j = i; j < poslednjiIndeks - 1; j++) 
						{
							uticniceKlijenata[j] = uticniceKlijenata[j + 1];
						}
						uticniceKlijenata[poslednjiIndeks - 1] = 0;

						poslednjiIndeks--;
					}
				}
			}
		}
	}

	closesocket(uticnicaServera);

	WSACleanup();

	return 0;
}
