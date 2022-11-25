// TCP klijent, neblokirajuce uticnice
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

int main() 
{
    SOCKET klijentskaUticnica = INVALID_SOCKET;

    int iResult;

    char dataBuffer[BUFFER_SIZE];

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
    {
        printf("Pokretanje winsock biblioteke neuspesno.\nGRESKA: %d\n", WSAGetLastError());
        return 1;
    }

    klijentskaUticnica = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (klijentskaUticnica == INVALID_SOCKET)
    {
        printf("Otvaranje klijentske uticnice neuspesno.\nGRESKA: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    sockaddr_in adresaServera;
    adresaServera.sin_family = AF_INET;
	adresaServera.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    adresaServera.sin_port = htons(SERVER_PORT);

	iResult = connect(klijentskaUticnica, (SOCKADDR*) &adresaServera, sizeof(adresaServera));
    if (iResult == SOCKET_ERROR)
    {
        printf("Povezivanje korisnicke uticnice sa adresom servera nije uspelo.\n");
        closesocket(klijentskaUticnica);
        WSACleanup();
		return 1;
    }
    
    // Promenljiva tipa studentInfo cija ce se polja popunuti
	// i cela struktura poslati u okviru jedne poruke
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
		student.poeni = htons(poeni);  
		getchar();    

		// Slanje pripremljene poruke zapisane unutar strukture studentInfo
		// prosleđujemo adresu promenljive student u memoriji, jer se na toj adresi nalaze podaci koje saljemo
		// kao i velicinu te strukture (jer je to duzina poruke u bajtima)
		iResult = send(klijentskaUticnica, (char*) &student, (int) sizeof(studentInfo), 0);

		// Check result of send function
		if (iResult == SOCKET_ERROR)
		{
			printf("Slanje poruke serveru neuspesno.\nGreska: %d\n", WSAGetLastError());
			closesocket(klijentskaUticnica);
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

	// Gasenje uticnice
	iResult = shutdown(klijentskaUticnica, SD_BOTH);
	if (iResult == SOCKET_ERROR)
	{
		printf("Shutdown klijentskog soketa neuspesan.\nBreak: %d\n", WSAGetLastError());
		closesocket(klijentskaUticnica);
		WSACleanup();
		return 1;
	}
	
	// Sacekaj pre nego što zatvoriš utičnicu
	Sleep(1000);

    closesocket(klijentskaUticnica);

    WSACleanup();

    return 0;
}
