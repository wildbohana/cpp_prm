// TCP server
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

#define SERVER_PORT 27016
#define BUFFER_SIZE 256

// TCP server that use blocking sockets
int main()
{
	SOCKET serverskaUticnica = INVALID_SOCKET;

	SOCKET prihvacenaUticnica1 = INVALID_SOCKET;
	SOCKET prihvacenaUticnica2 = INVALID_SOCKET;

	int iResult;

	// DODATO ZA LOGIKU IGRE
	bool prvi, drugi;
	bool odustaje1, odustaje2;

	int tacneReci1, tacneReci2;
	int duzina1, duzina2;

	// PRAVIMO DVA BAFERA - JER IMAMO DVA IGRAČA
	char dataBuffer1[BUFFER_SIZE];
	char dataBuffer2[BUFFER_SIZE];

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Pokretanje winsock bibilioteke neuspesno.\nGRESKA: %d\n", WSAGetLastError());
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
		printf("Otvaranje serverske uticnice neuspesno.\nGreska: %ld\n", WSAGetLastError());
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

	// SOMAXCON - maksimalan broj zahteva
	iResult = listen(serverskaUticnica, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("Postavljanje serveske uticnice u rezim prisluskivanja neuspesan.\nGreska: %d\n", WSAGetLastError());
		closesocket(serverskaUticnica);
		WSACleanup();
		return 1;
	}

	printf("Serverska uticnica je postavljena u rezim prisluskivanja.\n");
	printf("Cekanje na nove zahteve za uspostavu veze od klijenata...\n");

	do
	{
		sockaddr_in adresaKlijenta;
		int velicinaKlijentskeAdrese = sizeof(struct sockaddr_in);

		// Prihvati prvog klijenta
		prihvacenaUticnica1 = accept(serverskaUticnica, (SOCKADDR*) &adresaKlijenta, &velicinaKlijentskeAdrese);
		if (prihvacenaUticnica1 == INVALID_SOCKET)
		{
			printf("Povezivanje sa klijentom neuspesno.\nGRESKA: %d\n", WSAGetLastError());
			closesocket(serverskaUticnica);
			closesocket(prihvacenaUticnica1);
			WSACleanup();
			return 1;
		}

		printf("\nPrvi klijent uspesno povezan. Adresa klijenta: %s : %d\n", 
			inet_ntoa(adresaKlijenta.sin_addr), ntohs(adresaKlijenta.sin_port));

		// Prihvati drugog klijenta
		prihvacenaUticnica2 = accept(serverskaUticnica, (SOCKADDR*) &adresaKlijenta, &velicinaKlijentskeAdrese);
		if (prihvacenaUticnica2 == INVALID_SOCKET)
		{
			printf("Povezivanje sa klijentom neuspesno.\nGRESKA: %d\n", WSAGetLastError());
			closesocket(serverskaUticnica);
			closesocket(prihvacenaUticnica2);
			WSACleanup();
			return 1;
		}

		printf("\nDrugi klijent uspesno prihvacen. Adresa klijenta: %s : %d\n", 
			inet_ntoa(adresaKlijenta.sin_addr), ntohs(adresaKlijenta.sin_port));

		// Nakon uspešnog konektovanja na server, na serveru se zadaje slovo na koje će se igrati.
		printf("Unesite slovo na koje pocinje igra: ");
		char slovo = scanf("%c", &slovo);
		getchar();

		sprintf(dataBuffer1, "Pocinje igra na slovo na slovo %c. Posaljite vasu rec!\n", slovo);
		sprintf(dataBuffer2, "Pocinje igra na slovo na slovo %c. Posaljite vasu rec!\n", slovo);
		
		tacneReci1 = tacneReci2 = 0;
		duzina1 = duzina2 = 0;
		odustaje1 = odustaje2 = false;
		bool kraj = false;

		do
		{
			iResult = send(prihvacenaUticnica1, dataBuffer1, (int) strlen(dataBuffer1), 0);
			if (iResult == SOCKET_ERROR)
			{
				printf("Slanje poruke ka klijentu neuspesno.\nGRESKA: %d\n", WSAGetLastError());
				shutdown(prihvacenaUticnica1, SD_BOTH);
				shutdown(prihvacenaUticnica2, SD_BOTH);
				closesocket(prihvacenaUticnica1);
				closesocket(prihvacenaUticnica2);
				break;
			}

			iResult = send(prihvacenaUticnica2, dataBuffer2, (int) strlen(dataBuffer2), 0);
			if (iResult == SOCKET_ERROR)
			{
				printf("Slanje poruke ka klijentu neuspesno.\nGRESKA: %d\n", WSAGetLastError());
				shutdown(prihvacenaUticnica1, SD_BOTH);
				shutdown(prihvacenaUticnica2, SD_BOTH);
				closesocket(prihvacenaUticnica1);
				closesocket(prihvacenaUticnica2);
				break;
			}

			// Da li je zavrsila igra?
			if (kraj)
			{
				break;
			}

			iResult = recv(prihvacenaUticnica1, dataBuffer1, BUFFER_SIZE, 0);
			if (iResult > 0)
			{
				dataBuffer1[iResult] = '\0';

				printf("Klijent 1 je poslao: %s.\n", dataBuffer1);
				
				// Da li prvi igrac zavrsava sa igrom
				if (!strcmp(dataBuffer1, "Kraj"))  
				{
					odustaje1 = true;
					prvi = false;
				}
				// Da li je prvi igrac poslao rec na zadato slovo
				else
				{
					if (toupper(dataBuffer1[0]) == toupper(slovo))
					{
						printf("Tacna rec!\n");
						prvi = true;
						tacneReci1++;
						duzina1 += strlen(dataBuffer1);
					}
					else
					{
						printf("Netacna rec!\n");
						prvi = false;
					}
				}
			}
			// Da li je poslata komanda za gasenje
			else if (iResult == 0)	
			{
				printf("Veza sa prvim klijentom je zatvorena.\n");
				shutdown(prihvacenaUticnica2, SD_BOTH);
				closesocket(prihvacenaUticnica1);
				closesocket(prihvacenaUticnica2);
				break;
			}
			// Desila se greska u prenosu
			else	
			{
				printf("Primanje poruke od prvog klijenta neuspesno.\nGRESKA: %d\n", WSAGetLastError());
				shutdown(prihvacenaUticnica1, SD_BOTH);
				shutdown(prihvacenaUticnica2, SD_BOTH);
				closesocket(prihvacenaUticnica1);
				closesocket(prihvacenaUticnica2);
				break;
			}

			iResult = recv(prihvacenaUticnica2, dataBuffer2, BUFFER_SIZE, 0);
			if (iResult > 0)	
			{
				dataBuffer2[iResult] = '\0';

				printf("Klijent 2 je poslao: %s.\n", dataBuffer2);
				
				// Da li drugi igrac zavrsava sa igrom
				if (!strcmp(dataBuffer2, "Kraj"))    
				{
					odustaje2 = true;
					drugi = false;
				}
				// Da li je drugi igrac poslao rec na zadato slovo
				else
				{
					if (toupper(dataBuffer2[0]) == toupper(slovo))
					{
						printf("Tacna rec!\n");
						drugi = true;
						tacneReci2++;
						duzina2 += strlen(dataBuffer2);
					}
					else
					{
						printf("Netacna rec!\n");
						drugi = false;
					}
				}
			}
			// Da li je poslata komanda za gasenje
			else if (iResult == 0)	
			{
				// Connection was closed successfully
				printf("Veza sa drugim klijentom je zatvorena.\n");
				shutdown(prihvacenaUticnica1, SD_BOTH);
				closesocket(prihvacenaUticnica1);
				closesocket(prihvacenaUticnica2);
				break;
			}
			// Desila se greska u prenosu
			else	
			{
				printf("Primanje poruke od klijenta neuspesno.\nGRESKA: %d\n", WSAGetLastError());
				shutdown(prihvacenaUticnica1, SD_BOTH);
				shutdown(prihvacenaUticnica2, SD_BOTH);
				closesocket(prihvacenaUticnica1);
				closesocket(prihvacenaUticnica2);
				break;
			}
			
			// Ako su oba pogodili, ili oba omasili, igra se nastavlja
			if (prvi == drugi)
			{
				strcpy(dataBuffer1, "Molimo Vas posaljite sledecu rec na zadato slovo.");
				strcpy(dataBuffer2, "Molimo Vas posaljite sledecu rec na zadato slovo.");
			}
			// Jedan je omasio, drugi je pogodio (on je pobedio)
			else
			{
				// Prvi igrac je pobedio
				if (prvi)
				{
					sprintf(dataBuffer1, "Prvi igrac je poslao %d ispravnih reci, drugi igrac je poslao %d ispravnih reci. Vi ste POBEDILI!\n", tacneReci1, tacneReci2);
					sprintf(dataBuffer2, "Prvi igrac je poslao %d ispravnih reci, drugi igrac je poslao %d ispravnih reci. Vi ste IZGUBILI!\n", tacneReci1, tacneReci2);
					
					kraj = true;
				}

				// Drugi igrac je pobedio
				if (drugi)
				{
					sprintf(dataBuffer1, "Prvi igrac je poslao %d ispravnih reci, drugi igrac je poslao %d ispravnih reci. Vi ste IZGUBILI!\n", tacneReci1, tacneReci2);
					sprintf(dataBuffer2, "Prvi igrac je poslao %d ispravnih reci, drugi igrac je poslao %d ispravnih reci. Vi ste POBEDILI!\n", tacneReci1, tacneReci2);
					
					kraj = true;
				}
			}

			// Ako oba igraca odustanu u isto vreme, pobedjuje onaj cija rec je duza
			if (odustaje1 && odustaje2)
			{
				if (duzina1 > duzina2)
				{
					sprintf(dataBuffer1, "Prvi igrac je poslao %d ispravnih reci, drugi igrac je poslao %d ispravnih reci.\nUkupna duzina vasih reci je veca. Vi ste POBEDILI!\n", tacneReci1, tacneReci2);
					sprintf(dataBuffer2, "Prvi igrac je poslao %d ispravnih reci, drugi igrac je poslao %d ispravnih reci.\nUkupna duzina vasih reci je kraca.Vi ste IZGUBILI!\n", tacneReci1, tacneReci2);
					
					kraj = true;
				}
				else
				{
					sprintf(dataBuffer1, "Prvi igrac je poslao %d ispravnih reci, drugi igrac je poslao %d ispravnih reci.\nUkupna duzina vasih reci je kraca. Vi ste IZGUBILI!\n", tacneReci1, tacneReci2);
					sprintf(dataBuffer2, "Prvi igrac je poslao %d ispravnih reci, drugi igrac je poslao %d ispravnih reci.\nUkupna duzina vasih reci je veca. Vi ste POBEDILI!\n", tacneReci1, tacneReci2);
					
					kraj = true;
				}
			}

		} while (true);

	} while (true);

	// Gasenje veze sa klijentima
	iResult = shutdown(prihvacenaUticnica1, SD_BOTH);
	if (iResult == SOCKET_ERROR)
	{
		printf("Gasenje prve prihvatne uticnice neuspesno.\nGRESKA: %d\n", WSAGetLastError());
		closesocket(prihvacenaUticnica1);
		closesocket(prihvacenaUticnica2);
		WSACleanup();
		return 1;
	}

	iResult = shutdown(prihvacenaUticnica2, SD_BOTH);
	if (iResult == SOCKET_ERROR)
	{
		printf("Gasenje druge prihvatne uticnice neuspesno.\nGRESKA: %d\n", WSAGetLastError());
		closesocket(prihvacenaUticnica1);
		closesocket(prihvacenaUticnica2);
		WSACleanup();
		return 1;
	}

	closesocket(serverskaUticnica);
	closesocket(prihvacenaUticnica1);
	closesocket(prihvacenaUticnica2);

	WSACleanup();

	return 0;
}
