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

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27016
#define BUFFER_SIZE 256

// STRUKTURA ZA ZADATAK
struct studentInfo {
	char ime[15];
	char prezime[20];
	short poeni;
};

// TCP client that uses non-blocking sockets
int main() 
{
    // Socket used to communicate with server
    SOCKET klijentSoket = INVALID_SOCKET;

    // Variable used to store function return value
    int iResult;

    // Buffer we will use to store message
    char dataBuffer[BUFFER_SIZE];

	// WSADATA data structure that is to receive details of the Windows Sockets implementation
    WSADATA wsaData;

	// Initialize windows sockets library for this process
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
        printf("WSAStartup neuspesan.\nGreska: %d\n", WSAGetLastError());
        return 1;
    }

    // create a socket
    klijentSoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (klijentSoket == INVALID_SOCKET)
    {
        printf("Otvaranje soketa neuspesno.\nGreska: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Create and initialize address structure
    sockaddr_in adresaServera;
    adresaServera.sin_family = AF_INET;
	adresaServera.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    adresaServera.sin_port = htons(SERVER_PORT);

    // Connect to server specified in adresaServera and socket klijentSoket
	iResult = connect(klijentSoket, (SOCKADDR*) &adresaServera, sizeof(adresaServera));
    if (iResult == SOCKET_ERROR)
    {
        printf("Povezivanje sa serverom nije uspelo.\n");
        closesocket(klijentSoket);
        WSACleanup();
		return 1;
    }
        
    // Promenljiva tipa studentInfo cija ce se polja popunuti i cela struktura poslati u okviru jedne poruke
	studentInfo student;  
	short poeni;
 
	while (true)
	{
		// Unos potrebnih podataka koji ce se poslati serveru
		printf("Unesite ime studenta: ");
		gets_s(student.ime, 15);

		printf("Unesite prezime studenta: ");
		gets_s(student.prezime, 20);

		printf("Unesite osvojene poene na testu: ");
		scanf("%d", &poeni);
		
		// Obavezno je pozvati funkciju htons() jer cemo slati podatak tipa short 
		student.poeni = htons(poeni);  
		// Pokupiti enter karakter iz bafera tastature
		getchar();    

		// Slanje pripremljene poruke zapisane unutar strukture studentInfo
		// prosleđujemo adresu promenljive student u memoriji, jer se na toj adresi nalaze podaci koje saljemo
		// kao i velicinu te strukture (jer je to duzina poruke u bajtima)
		iResult = send(klijentSoket, (char*) &student, (int) sizeof(studentInfo), 0);

		// Check result of send function
		if (iResult == SOCKET_ERROR)
		{
			printf("Slanje poruke serveru neuspesno.\nGreska: %d\n", WSAGetLastError());
			closesocket(klijentSoket);
			WSACleanup();
			return 1;
		}

		printf("Poruka uspesno poslata serveru. Ukupno bajtova: %ld\n", iResult);

		printf("\nPritisni 'x' za izlaz, ili bilo koji drugi taster za nastavak: ");
		if (getch() == 'x')
		{
			break;
		}
	}

	// Shutdown the connection since we're done
	iResult = shutdown(klijentSoket, SD_BOTH);

	// Check if connection is succesfully shut down.
	if (iResult == SOCKET_ERROR)
	{
		printf("Shutdown klijentskog soketa neuspesan.\nBreak: %d\n", WSAGetLastError());
		closesocket(klijentSoket);
		WSACleanup();
		return 1;
	}

	Sleep(1000);

    // Close connected socket
    closesocket(klijentSoket);

	// Deinitialize WSA library
    WSACleanup();

    return 0;
}
