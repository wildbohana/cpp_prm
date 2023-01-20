// Kriptografija

// Onemoguci bespotrebne warninge
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "conio.h"
#include "pcap.h"
#include "protocol_headers.h"

// Deklaracije funkcija
void dispatcher_handler(unsigned char* user, const struct pcap_pkthdr* packet_header, const unsigned char* packet_data);
const char* plejfer(char* poruka);

// Plejfer matrica
char kljuc[5][5] = { 
	{'M', 'E', 'D', 'V', 'A'}, 
	{'B', 'C', 'F', 'G', 'H'}, 
	{'I', 'K', 'L', 'N', 'O'},  
	{'P', 'Q', 'R', 'S', 'T'},  
	{'U', 'W', 'X', 'Y', 'Z'} };

int icmpBrojac = 0;

// Referenca na fajl u koji se upisuje
pcap_dumper_t* file_dumper;

int main()
{
	pcap_t* device_handle;
	char error_buffer[PCAP_ERRBUF_SIZE];

	// Otvaranje fajla za čitanje
	if ((device_handle = pcap_open_offline("packetsv12.pcap", error_buffer)) == NULL)
	{
		printf("\n Unable to open the file %s.\n", "packetsv12.pcap");
		return -1;
	}

	// Otvaranje fajla za pisanje
	file_dumper = pcap_dump_open(device_handle, "encrypackets.pcap");
	if (file_dumper == NULL)
	{
		printf("\n Error opening output file\n");
		return -1;
	}

	// Proveri sloj veze; radi jednostavnosti, samo Ethernet je podržan
	if (pcap_datalink(device_handle) != DLT_EN10MB)
	{
		printf("\nThis program works only on Ethernet networks.\n");
		return -1;
	}

	// Čitaj i "primaj" pakete dok ne dođeš do EOF
	pcap_loop(device_handle, 0, dispatcher_handler, NULL);

	printf("Broj ICMP paketa: %d\n", icmpBrojac);

	// Zatvori sve fajlove vezane za device_handle i dealociraj resurce
	pcap_close(device_handle);

	getchar();
}

// Isto kao packet_handler, samo prekršten
void dispatcher_handler(unsigned char* user, const struct pcap_pkthdr* packet_header, const unsigned char* packet_data)
{
	// Ispiši timestamp paketa
	printf("Paket pristigao: %ld:%ld\n", 
		packet_header->ts.tv_sec,
		packet_header->ts.tv_usec);

	// Ispiši dužinu paketa
	int velicinaPaketa = packet_header->len;

	// Kopija paketa, postavlja se na vrednosti 0
	char kopija[1000];
	memset(kopija, 0, velicinaPaketa * sizeof(char));

	// Preuzimanje podataka iz ethernet okvira i smestanje zaglavlja u kopiju
	ethernet_header* eh = (ethernet_header*)packet_data;
	memcpy(kopija, eh, sizeof(ethernet_header) * sizeof(char));

	// Provera da li je IPv4
	if (ntohs(eh->type) == 0x0800)
	{
		// Pristupanje IP zaglavlju i smestanje u kopiju
		ip_header* ih = (ip_header*)((unsigned char*)eh + sizeof(ethernet_header));
		memcpy(kopija + sizeof(ethernet_header), ih, (ih->header_length * 4) * sizeof(char));

		printf("Logicka adresa primaoca: %d.%d.%d.%d\n", 
			ih->dst_addr[0], ih->dst_addr[1], 
			ih->dst_addr[2], ih->dst_addr[3]);

		// Provera sledeceg protokola: ICMP - 1; TCP - 6; UDP - 17
		if (ih->next_protocol == 1)
		{
			printf("Protokol: ICMP");
			icmpBrojac++;
		}
		else if (ih->next_protocol == 6)
		{
			// Pristupanje TCP zaglavlju
			tcp_header* th = (tcp_header*)((unsigned char*)ih + ih->header_length * 4);

			printf("Protokol: TCP\n");
			printf("Zaglavlje:");
			
			// Ispis paketa
			for (int i = 0; i < th->header_length * 4; i++)
			{
				printf("%.2x ", th[i]);
				if ((i + 1) % 16 == 0)
					printf("\n");
			}
			printf("\n");

			// Provera da li je port 80 -> HTTP (vidi se u Wireshark-u)
			if (ntohs(th->src_port) == 80 || ntohs(th->dest_port) == 80)
			{
				printf("HTTP sadrzaj: ");
				char* app_data = (char*)((unsigned char*)th + th->header_length * 4);

				for (int i = 0; i < 16; i++)
				{
					printf("%c", app_data[i]);
				}
				printf("\n");
			}
		}
		else if (ih->next_protocol == 17)
		{
			// Pristupanje UDP zaglavlju i smestanje u kopiju
			printf("Protokol: UDP\n");
			udp_header* uh = (udp_header*)((unsigned char*)ih + ih->header_length * 4);
			memcpy(kopija + sizeof(ethernet_header) + ih->header_length * 4, uh, sizeof(udp_header));

			// Aplikativni deo
			char* app_data = (char*)((unsigned char*)uh + sizeof(udp_header));
			int app_length = ntohs(uh->datagram_length) - sizeof(udp_header);

			printf("Aplikativni deo: ");
			for (int i = 0; i < app_length; i++)
			{
				printf("%c", app_data[i]);
				if ((i + 1) % 16 == 0)
					printf("\n");
			}
			printf("\n");

			app_data[app_length] = '\0';

			// Sifrovanje poruke
			char cipher[200] = "\0";
			strcpy(cipher, plejfer(app_data));
			printf("Sifrovano: %s", cipher);

			// Kopiranje sifrovane poruke u kopiju aplikativnog dela paketa
			memcpy(kopija + sizeof(ethernet_header) + ih->header_length * 4 + sizeof(udp_header), cipher, app_length);

			// Zapisivanje kopije u fajl
			pcap_dump((unsigned char*)file_dumper, packet_header, (const unsigned char*)kopija);
		}
	}
	// Provera da li je protokol ARP
	else if (ntohs(eh->type) == 0x0806)
	{
		printf("Protokol: ARP");
	}
	printf("\n\n");
}

// Plejfer šifra
const char* plejfer(char* poruka)
{
	// Pozicija slova u redovima i kolonama matrice
	int r1 = -1, r2 = -1;
	int k1 = -1, k2 = -1;

	int duzinaPoruke = strlen(poruka);

	// Ako je poruka neparne duzine, na kraj se dodaje neutralni karakter
	char neutralniKarakter = 'Z';
	if (duzinaPoruke % 2 == 1)
	{
		strncat(poruka, &neutralniKarakter, 1);
		duzinaPoruke += 1;
	}

	char kriptovanaPoruka[200];
	
	// Trazenje pozicije parova slova u matrici
	for (int i = 0; i < duzinaPoruke; i += 2)
	{
		// Ako se u poruci pojavi slovo J menja se u slovo I
		if (poruka[i] == 'J')
		{
			poruka[i] = 'I';
		}

		for (int j = 0; j < 5; j++)
		{
			for (int k = 0; k < 5; k++)
			{
				if (kljuc[j][k] == poruka[i])
				{
					r1 = j;
					k1 = k;
				}
				if (kljuc[j][k] == poruka[i + 1])
				{
					r2 = j;
					k2 = k;
				}
			}
		}

		// Ako su dva ista slova
		if (r1 == r2 && k1 == k2)
		{
			// Ono ostaje isto, i dodaje se X
			kriptovanaPoruka[i] = poruka[i];
			kriptovanaPoruka[i + 1] = 'X';
		}
		else
		{
			// Ako su slova u istom redu
			if (r1 == r2)
			{
				// Ako je poslednja kolona, pomera se na prvu
				if (k1 == 4)
				{
					kriptovanaPoruka[i] = kljuc[r1][0];
				}
				// U suprotnom, pomera se u kolonu desno
				else
				{
					kriptovanaPoruka[i] = kljuc[r1][k1 + 1];
				}

				if (k2 == 4)
				{
					kriptovanaPoruka[i + 1] = kljuc[r2][0];
				}
				else
				{
					kriptovanaPoruka[i + 1] = kljuc[r2][k2 + 1];
				}
			}
			// Ako su slova u istoj koloni
			else if (k1 == k2)
			{
				// Ako je poslednji red, pomera se na prvi
				if (r1 == 4)
				{
					kriptovanaPoruka[i] = kljuc[0][k1];
				}
				// U suprotnom, pomera se u red dole
				else
				{
					kriptovanaPoruka[i] = kljuc[r1 + 1][k1];
				}

				if (r2 == 4)
				{
					kriptovanaPoruka[i + 1] = kljuc[0][k2];
				}
				else
				{
					kriptovanaPoruka[i + 1] = kljuc[r2 + 1][k2];
				}
			}
			// U slucaju da su u razlicitim redovima i kolonama, menjaju se kolone
			else
			{
				kriptovanaPoruka[i] = kljuc[r1][k2];
				kriptovanaPoruka[i + 1] = kljuc[r2][k1];
			}
		}
	}
	
	// Zavrsava se poruka
	kriptovanaPoruka[duzinaPoruke] = '\0';
	return kriptovanaPoruka;
}
