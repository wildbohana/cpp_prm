// Hvatanje i filtriranje paketa 

// Onemoguci bespotrebne warninge
#ifdef _MSC_VER
	#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "pcap.h"
#include "conio.h"

// Broji svaki paket
int packet_counter = 0;	

// Deklaracije funkcija
pcap_if_t* select_device(pcap_if_t* devices);
void packet_handler(unsigned char* param, const struct pcap_pkthdr* packet_header, const unsigned char* packet_data);

int main()
{	
	pcap_if_t* devices;						// Lista kontrolera
	pcap_if_t* device;						// Jedan kontroler
	pcap_t* device_handle;					// Deskriptor uređaja za prihvatanje paketa
	char error_buffer[PCAP_ERRBUF_SIZE];	// Bafer za greške

	// Dobavljanje liste svih dostupnih kontrolera na lokalnoj mašini
    if (pcap_findalldevs(&devices, error_buffer) == -1)
	{
		printf("Error in pcap_findalldevs: %s\n", error_buffer);
		return -1;
	}
    
	// Izbor jendog uređaja is liste
	device = select_device(devices);
	
	// Provera da li je uređaj validan
	if (device == NULL)
	{
		pcap_freealldevs(devices);
		return -1;
	}

	// Otvaranje uređaja za prihvatanje
    if ((device_handle = pcap_open_live(device->name, 65536, 1, 500, error_buffer)) == NULL)
    {
        printf("\nUnable to open the adapter. %s is not supported by WinPcap\n", device->name);
        pcap_freealldevs(devices);
        return -1;
    }
    
	// TODO 1:
	// Postavi izraz za filtriranje paketa
	// Savet: koristi ipconfig da dobiješ ip adresu hosta na kom je pokrenut program
	unsigned int netmask;
	char filter_exp[] = "ip dst host 192.168.1.100 and tcp";  
	struct bpf_program fcode;

	// Dobavi masku prve adrese interfejsa
	if (device->addresses != NULL)
		netmask = ((struct sockaddr_in*)(device->addresses->netmask))->sin_addr.s_addr;
	// Ako interfejs nema adresu, pretpostavljamo da je u C klasi networka
	else
		netmask = 0xffffff;

	// Kompajliramo filter
	if (pcap_compile(device_handle, &fcode, filter_exp, 1, netmask) < 0)
	{
		 printf("\n Unable to compile the packet filter. Check the syntax.\n");
		 return -1;
	}

	// Postavljamo filter
	if (pcap_setfilter(device_handle, &fcode) < 0)
	{
		printf("\n Error setting the filter.\n");
		return -1;
	}

    printf("\nListening on %s...\n", device->description);
    
	// Oslobađamo listu uređaja (više nam nije potrebna)
    pcap_freealldevs(devices);
    
	// Započni prihvatanje podataka
    pcap_loop(device_handle, 0, packet_handler, NULL);
    
    return 0;
}

// Ova funkcija nam omogućava da izaberemo uređaj iz liste dostupnih uređaja
pcap_if_t* select_device(pcap_if_t* devices)
{
	// Brojač za uređaje i povratna promenljiva
	int i = 0;	
	pcap_if_t* device;

    // Ispisujemo listu
    for (device = devices; device; device = device->next)
    {
        printf("%d. %s", ++i, device->name);
        if (device->description)
            printf(" (%s)\n", device->description);
        else
            printf(" (No description available) \n");
    }
    
	// Proveramo da li je lista prazna
    if (i == 0)
    {
        printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
        return NULL;
    }
    
	// Izbor jednog uređaja iz liste
	int device_number;
    printf("Enter the interface number (1-%d):", i);
    scanf_s("%d", &device_number);
    
    if (device_number < 1 || device_number > i)
    {
        printf("\nInterface number out of range.\n");
        return NULL;
    }
    
	// Skok na selektovani uređaj (jako kul fora)
    for (device = devices, i = 0; i < device_number - 1; device = device->next, i++);

	return device;
}

// Callback funkcija koju WinPcap poziva za svaki pristigli paket
void packet_handler(unsigned char* param, const struct pcap_pkthdr* packet_header, const unsigned char* packet_data)
{
	// Print timestamp and length of the packet
	time_t timestamp;			// Sirovo vreme kada je paket primljen (u bitima)
	struct tm* local_time;		// Lokalno vreme kada je paket primljen
	char time_string[16];		// Lokalno vreme pretvoreno u string

	// Pretvaranje timestamp u format koji možemo da pročitamo
	timestamp = packet_header->ts.tv_sec;
	local_time = localtime(&timestamp);
	strftime(time_string, sizeof time_string, "%H:%M:%S", local_time);
	
	printf("\n-------------------------------------------");
	printf("\nPaket (%d): %s, %d byte\n", ++packet_counter, time_string, packet_header->len);

	// TODO 2:
	// Ispis sadržaja svakog paketa
	for (int i = 0; i < packet_header->len; i++)
	{
		// Ispis svakog bajta kao dva hex broja
		printf("%.2x ", packet_data[i]);
		
		// 32 bajta po liniji ispisa
		if ((i + 1) % 32 == 0)
			printf("\n");
	}
}
