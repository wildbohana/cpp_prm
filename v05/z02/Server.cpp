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

// TCP server that use non-blocking sockets
int main() 
{
	// Socket used for listening for new clients 
	SOCKET serverSoket = INVALID_SOCKET;

	// Sockets used for communication with client
	SOCKET klijentSoketi[MAX_CLIENTS];
	short poslednjiIndeks = 0;

	// Variable used to store function return value
	int iResult;

	// Buffer used for storing incoming data
	char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
	WSADATA wsaData;

	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		printf("WSAStartup neuspesan.\nGreska: %d\n", WSAGetLastError());
		return 1;
	}

	// Initialize adresaServera structure used by bind
	sockaddr_in adresaServera;
	memset((char*) &adresaServera, 0, sizeof(adresaServera));
	adresaServera.sin_family = AF_INET;
	adresaServera.sin_addr.s_addr = INADDR_ANY;
	adresaServera.sin_port = htons(SERVER_PORT);

	// Initialise all client_socket[] to 0 so not checked
	memset(klijentSoketi, 0, MAX_CLIENTS * sizeof(SOCKET));

	// Create a SOCKET for connecting to server
	serverSoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check if socket is successfully created
	if (serverSoket == INVALID_SOCKET)
	{
		printf("Otvaranje serverskog soketa nije uspelo.\nGreska: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address to socket
	iResult = bind(serverSoket,(struct sockaddr*) &adresaServera, sizeof(adresaServera));

	// Check if socket is successfully binded to address and port from sockaddr_in structure
	if (iResult == SOCKET_ERROR)
	{
		printf("bind soketa za server nije uspelo.\nGreska: %d\n", WSAGetLastError());
		closesocket(serverSoket);
		WSACleanup();
		return 1;
	}
	
	// BITNO ???!!!?!?!
	// All connections are by default accepted by protocol stek if socket is in listening mode
	// With SO_CONDITIONAL_ACCEPT parameter set to true, connections will not be accepted by default
	bool bOptVal = true;
	int bOptLen = sizeof (bool);
	
	iResult = setsockopt(serverSoket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char*) &bOptVal, bOptLen);
	if (iResult == SOCKET_ERROR) 
	{
	    printf("setsockopt za SO_CONDITIONAL_ACCEPT nije uspela.\nGreska: %u\n", WSAGetLastError());
	}

	unsigned long  mode = 1;
	if (ioctlsocket(serverSoket, FIONBIO, &mode) != 0)
	{
		printf("ioctlsocket nesupesan.");
	}

	// Set serverSoket in listening mode
	iResult = listen(serverSoket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen za server neuspesan.\nGreska: %d\n", WSAGetLastError());
		closesocket(serverSoket);
		WSACleanup();
		return 1;
	}

	printf("Soket za server je u rezimu prisluskivanja. Cekanje na nove zahteve za povezivanje.\n");

	// Set of socket descriptors
	fd_set fdCitanje;

	// Timeout for select function
	timeval vreme;
	vreme.tv_sec = 1;
	vreme.tv_usec = 0;
	
	studentInfo *student;

	while (true)
	{
		// Initialize socket set
		FD_ZERO(&fdCitanje);

		// Add server's socket and clients' sockets to set
		if (poslednjiIndeks != MAX_CLIENTS)
		{
			FD_SET(serverSoket, &fdCitanje);
		}

		for (int i = 0 ; i < poslednjiIndeks ; i++) 
		{
			FD_SET(klijentSoketi[i], &fdCitanje);
		}

		// Wait for events on set
		int selectResult = select( 0 , &fdCitanje , NULL , NULL , &vreme);

		if (selectResult == SOCKET_ERROR)
		{
			printf("Select neuspesan.\nGreska: %d\n", WSAGetLastError());
			closesocket(serverSoket);
			WSACleanup();
			return 1;
		}
		// Timeout expired
		else if (selectResult == 0) 
		{
			// Check if some key is pressed
			if (_kbhit()) 
			{
				getch();
				printf("Primena racunarskih mreza u infrstrukturnim sistemima 2022/2023\n");
			}
			continue;
		}
		else if (FD_ISSET(serverSoket, &fdCitanje)) 
		{
			// Struct for information about connected client
			sockaddr_in clientAddr;
			int clientAddrSize = sizeof(struct sockaddr_in);

			// New connection request is received. Add new socket in array on first free position.
			klijentSoketi[poslednjiIndeks] = accept(serverSoket, (struct sockaddr*) &clientAddr, &clientAddrSize);

			if (klijentSoketi[poslednjiIndeks] == INVALID_SOCKET)
			{
				if (WSAGetLastError() == WSAECONNRESET)
				{
					printf("accept neuspesan, jer je timeout za zahtev klijenta istekao.\n");
				}
				else
				{
					printf("accept neuspesan.\nGreska: %d\n", WSAGetLastError());
				}
			}
			else
			{
				if (ioctlsocket(klijentSoketi[poslednjiIndeks], FIONBIO, &mode) != 0)
				{
					printf("ioctlsocket neuspesan.\n");
					continue;
				}

				poslednjiIndeks++;
				printf("Prihvacen novi zahtev klijenta (%d). Adresa klijenta: %s : %d\n", poslednjiIndeks, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			}
		}
		else
		{
			// Check if new message is received from connected clients
			for (int i = 0; i < poslednjiIndeks ; i++) 
			{
				// Check if new message is received from client on position "i"
				if (FD_ISSET(klijentSoketi[i], &fdCitanje)) 
				{
					iResult = recv(klijentSoketi[i], dataBuffer, BUFFER_SIZE, 0);

					if (iResult > 0)
					{
						dataBuffer[iResult] = '\0';
						printf("Poruka je primljena od klijenta (%d):\n", i+1);
						
						// Primljenoj poruci u memoriji pristupiti preko pokazivaca tipa (studentInfo *)
						// Jer znamo format u kom je poruka poslata a to je struct studentInfo
						student = (studentInfo*) dataBuffer;

						printf("Ime i prezime: %s %s  \n", student->ime, student->prezime);
						printf("Poeni studenta: %d  \n", ntohs(student->poeni));
						printf("_______________________________  \n");		
					}
					else if (iResult == 0)
					{
						// Connection was closed gracefully
						printf("Veza sa klijentom (%d) je zatvorena.\n", i+1);
						closesocket(klijentSoketi[i]);

						// Sort array and clean last place
						for (int j = i; j < poslednjiIndeks-1; j++) 
						{
							klijentSoketi[j] = klijentSoketi[j+1];
						}
						klijentSoketi[poslednjiIndeks-1] = 0;

						poslednjiIndeks--;
					}
					else
					{
						// There was an error during recv
						printf("Poruka od klijenta nije primljena.\nGreska: %d\n", WSAGetLastError());
						closesocket(klijentSoketi[i]);

						// Sort array and clean last place
						for (int j = i; j < poslednjiIndeks-1; j++) 
						{
							klijentSoketi[j] = klijentSoketi[j+1];
						}
						klijentSoketi[poslednjiIndeks-1] = 0;

						poslednjiIndeks--;
					}
				}
			}
		}
	}

	// Close listen and accepted sockets
	closesocket(serverSoket);

	// Deinitialize WSA library
	WSACleanup();

	return 0;
}
