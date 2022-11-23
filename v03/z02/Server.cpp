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
	// Socket used for listening for new clients 
	SOCKET serverSoket = INVALID_SOCKET;

	// Socket used for communication with clients
	SOCKET prihvaceniSoket1 = INVALID_SOCKET;
	SOCKET prihvaceniSoket2 = INVALID_SOCKET;

	// Variable used to store function return value
	int iResult;

	// DODATO ZA LOGIKU IGRE
	bool prvi, drugi;
	bool odustaje1, odustaje2;

	int tacneReci1, tacneReci2;
	int duzina1, duzina2;

	// PRAVIMO DVA BAFERA - JER IMAMO DVA IGRAČA
	// Buffer used for storing incoming data
	char dataBuffer1[BUFFER_SIZE];
	char dataBuffer2[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup nije uspeo.\nGreska: %d\n", WSAGetLastError());
		return 1;
	}

	// Initialize adresaServera structure used by bind
	sockaddr_in adresaServera;
	memset((char*) &adresaServera, 0, sizeof(adresaServera));
	adresaServera.sin_family = AF_INET;				// IPv4 address family
	adresaServera.sin_addr.s_addr = INADDR_ANY;		// Use all available addresses
	adresaServera.sin_port = htons(SERVER_PORT);	// Use specific port

	// Create a SOCKET for connecting to server
	serverSoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check if socket is successfully created
	if (serverSoket == INVALID_SOCKET)
	{
		printf("Otvaranje serverskog soketa neuspesno.\nGreska: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address to socket
	iResult = bind(serverSoket, (struct sockaddr*)&adresaServera, sizeof(adresaServera));

	// Check if socket is successfully binded to address and port from sockaddr_in structure
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(serverSoket);
		WSACleanup();
		return 1;
	}

	// Set serverSoket in listening mode
	// SOMAXCON - maksimalan broj zahteva
	iResult = listen(serverSoket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen neuspesan.\nGreska: %d\n", WSAGetLastError());
		closesocket(serverSoket);
		WSACleanup();
		return 1;
	}

	printf("Soket za server je postavljen u mod za prisluskivanje. Cekanje na nove zahteve za konekcijom.\n");

	do
	{
		// Struct for information about connected client
		sockaddr_in klijentskaAdresa;

		int velicinaKlijentskeAdrese = sizeof(struct sockaddr_in);

		// Accept new connections from clients 
		prihvaceniSoket1 = accept(serverSoket, (struct sockaddr*) &klijentskaAdresa, &velicinaKlijentskeAdrese);

		// Check if accepted socket is valid 
		if (prihvaceniSoket1 == INVALID_SOCKET)
		{
			printf("accept neuspesan.\nGreska: %d\n", WSAGetLastError());
			closesocket(serverSoket);
			WSACleanup();
			return 1;
		}

		printf("\nPrvi klijent uspesno prihvacen. Adresa klijenta: %s : %d\n", 
			inet_ntoa(klijentskaAdresa.sin_addr), ntohs(klijentskaAdresa.sin_port));

		// Accept the second client
		prihvaceniSoket2 = accept(serverSoket, (struct sockaddr*) &klijentskaAdresa, &velicinaKlijentskeAdrese);

		// Check if accepted socket is valid 
		if (prihvaceniSoket2 == INVALID_SOCKET)
		{
			printf("accept neuspesan.\nGreska: %d\n", WSAGetLastError());
			closesocket(serverSoket);
			closesocket(prihvaceniSoket1);
			WSACleanup();
			return 1;
		}

		printf("\nDrugi klijent uspesno prihvacen. Adresa klijenta: %s : %d\n", 
			inet_ntoa(klijentskaAdresa.sin_addr), ntohs(klijentskaAdresa.sin_port));

		// Nakon uspešnog konektovanja na server, na serveru se zadaje slovo na koje će se igrati.
		printf("Unesite slovo na koje pocinje igra: ");
		char slovo = scanf("%c", &slovo);
		getchar();

		sprintf(dataBuffer1, "Pocinje igra na slovo na slovo %c. Posaljite vasu rec!\n", slovo);
		sprintf(dataBuffer2, "Pocinje igra na slovo na slovo %c. Posaljite vasu rec!\n", slovo);
		
		// Count number of correct words of prvi and drugi player
		tacneReci1 = tacneReci2 = 0;
		// Total length of correct words of prvi and drugi player
		duzina1 = duzina2 = 0;
		// Indicates if player quits the game
		odustaje1 = odustaje2 = false;
		// Indicator of the game end
		bool kraj = false;

		do
		{
			// Send message to client1 using connected socket
			iResult = send(prihvaceniSoket1, dataBuffer1, (int) strlen(dataBuffer1), 0);

			// Check result of send function
			if (iResult == SOCKET_ERROR)
			{
				printf("Slanje poruke od servera ka klijentu neuspesno.\nGreska: %d\n", WSAGetLastError());
				shutdown(prihvaceniSoket1, SD_BOTH);
				shutdown(prihvaceniSoket2, SD_BOTH);
				closesocket(prihvaceniSoket1);
				closesocket(prihvaceniSoket2);
				break;
			}

			// Send message to client2 using connected socket
			iResult = send(prihvaceniSoket2, dataBuffer2, (int)strlen(dataBuffer2), 0);

			// Check result of send function
			if (iResult == SOCKET_ERROR)
			{
				printf("Slanje poruke od servera ka klijentu neuspesno.\nGreska: %d\n", WSAGetLastError());
				shutdown(prihvaceniSoket1, SD_BOTH);
				shutdown(prihvaceniSoket2, SD_BOTH);
				closesocket(prihvaceniSoket1);
				closesocket(prihvaceniSoket2);
				break;
			}

			// If the play between two players ends
			if (kraj)
			{
				break;
			}

			// Receive data from prvi client 
			iResult = recv(prihvaceniSoket1, dataBuffer1, BUFFER_SIZE, 0);
			
			// Check if message is successfully received
			if (iResult > 0)
			{
				// iResult - duzina poruke, ovim dodajemo kraj na kraj stringa
				dataBuffer1[iResult] = '\0';

				// Log message text
				printf("Klijent 1 je poslao: %s.\n", dataBuffer1);
				
				// First player is quitting the game
				if (!strcmp(dataBuffer1, "Kraj"))  
				{
					odustaje1 = true;
					prvi = false;
				}
				// Check if player sent word on selected letter.
				else
				{
					// Change letter in upper-case to include both small and big letters
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
			// Check if shutdown command is received
			else if (iResult == 0)	
			{
				// Connection was closed successfully
				printf("Konekcija sa prvim klijentom je zatvorena.\n");
				shutdown(prihvaceniSoket2, SD_BOTH);
				closesocket(prihvaceniSoket1);
				closesocket(prihvaceniSoket2);
				break;
			}
			// There was an error during recv
			else	
			{
				printf("Primanje poruke od klijenta neuspesno.\nGreska: %d\n", WSAGetLastError());
				shutdown(prihvaceniSoket1, SD_BOTH);
				shutdown(prihvaceniSoket2, SD_BOTH);
				closesocket(prihvaceniSoket1);
				closesocket(prihvaceniSoket2);
				break;
			}

			// Receive data from second client 
			iResult = recv(prihvaceniSoket2, dataBuffer2, BUFFER_SIZE, 0);
			
			// Check if message is successfully received
			if (iResult > 0)	
			{
				dataBuffer2[iResult] = '\0';

				// Log message text
				printf("Klijent 2 je poslao: %s.\n", dataBuffer2);
				
				// drugi player is quitting
				if (!strcmp(dataBuffer2, "Kraj"))    
				{
					odustaje2 = true;
					drugi = false;
				}
				// Check if player sent word on selected letter
				else
				{
					// Change letter in upper-case to include both small and big letters
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
			// Check if shutdown command is received
			else if (iResult == 0)	
			{
				// Connection was closed successfully
				printf("Veza sa klijentom je uspesno zatvorena.\n");
				shutdown(prihvaceniSoket1, SD_BOTH);
				closesocket(prihvaceniSoket1);
				closesocket(prihvaceniSoket2);
				break;
			}
			// There was an error during recv
			else	
			{
				printf("Primanje poruke od klijenta neuspesno.\nGreska: %d\n", WSAGetLastError());
				shutdown(prihvaceniSoket1, SD_BOTH);
				shutdown(prihvaceniSoket2, SD_BOTH);
				closesocket(prihvaceniSoket1);
				closesocket(prihvaceniSoket2);
				break;
			}
			
			// The play continues if both clients have good words or both mistake
			if (prvi == drugi)
			{
				strcpy(dataBuffer1, "Molimo Vas posaljite sledecu rec na zadato slovo.");
				strcpy(dataBuffer2, "Molimo Vas posaljite sledecu rec na zadato slovo.");
			}
			// One has mistaken, other has right word (he is the winner)
			else
			{
				// first player wins
				if (prvi)
				{
					sprintf(dataBuffer1, "Prvi igrac je poslao %d ispravnih reci, drugi igrac je poslao %d ispravnih reci. Vi ste POBEDILI!\n", tacneReci1, tacneReci2);
					sprintf(dataBuffer2, "Prvi igrac je poslao %d ispravnih reci, drugi igrac je poslao %d ispravnih reci. Vi ste IZGUBILI!\n", tacneReci1, tacneReci2);
					kraj = true;
				}
				// drugi player wins
				if (drugi)
				{
					sprintf(dataBuffer1, "Prvi igrac je poslao %d ispravnih reci, drugi igrac je poslao %d ispravnih reci. Vi ste IZGUBILI!\n", tacneReci1, tacneReci2);
					sprintf(dataBuffer2, "Prvi igrac je poslao %d ispravnih reci, drugi igrac je poslao %d ispravnih reci. Vi ste POBEDILI!\n", tacneReci1, tacneReci2);
					kraj = true;
				}
			}

			// If both players quit at the same time
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

	// Shutdown the connection since we're done - PRVI SOKET
	iResult = shutdown(prihvaceniSoket1, SD_BOTH);

	// Check if connection is succesfully shut down.
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown neuspesan.\nGreska: %d\n", WSAGetLastError());
		closesocket(prihvaceniSoket1);
		closesocket(prihvaceniSoket2);
		WSACleanup();
		return 1;
	}

	// Shutdown the connection since we're done - DRUGI SOKET
	iResult = shutdown(prihvaceniSoket2, SD_BOTH);

	// Check if connection is succesfully shut down.
	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown neuspesan.\nGreska: %d\n", WSAGetLastError());
		closesocket(prihvaceniSoket1);
		closesocket(prihvaceniSoket2);
		WSACleanup();
		return 1;
	}

	// Close listen and accepted sockets
	closesocket(serverSoket);
	closesocket(prihvaceniSoket1);
	closesocket(prihvaceniSoket2);

	// Deinitialize WSA library
	WSACleanup();

	return 0;
}
