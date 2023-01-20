// Interpretacija sadrzaja paketa (2.deo)
// Resenje gde se hvataju paketi bez callback funkcije

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

int main()
{
	pcap_if_t* devices;
	pcap_if_t* device;
	pcap_t* device_handle;
	char error_buffer[PCAP_ERRBUF_SIZE];
	
	// Dobavljanje liste svih dostupnih kontrolera na lokalnoj mašini
    if (pcap_findalldevs(&devices, error_buffer) == -1)
	{
		printf("Error in pcap_findalldevs: %s\n", error_buffer);
		return -1;
	}

	// Broji svaki paket kako bismo imali numerisane pakete
	int i = 0;

    // Ispiši listu uređaja
    for(device = devices; device; device = device->next)
    {
        printf("%d. %s", ++i, device->name);
        if (device->description)
            printf(" (%s)\n", device->description);
        else
            printf(" (No description available)\n");
    }
    
	// Provera da li je lista uređaja prazna
    if (i == 0)
    {
        printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
        return NULL;
    }
    
	// Izbor jednog uređaja iz liste
	int device_number;
    printf("Enter the interface number (1-%d):",i);
    scanf_s("%d", &device_number);
    
    if (device_number < 1 || device_number > i)
    {
        printf("\nInterface number out of range.\n");
        return NULL;
    }
    
	// Izaberi prvi uređaj
	device = devices;

	// I onda dođi do izabranog uređaja
    for (i = 1; i < device_number; i++)
	{
		device = device->next;
	}
	
	printf("\nChosen adapter is  %s\n", device->description);

    // Otvaranje uređaja za prihvatanje podataka
    if ((device_handle = pcap_open_live(device->name, 65536, 0, 1000, error_buffer)) == NULL)
    {
        printf("\nUnable to open the adapter. %s is not supported by WinPcap\n", device->name);
        pcap_freealldevs(devices);
        return -1;
    }

	// Proveri sloj veze; radi jednostavnosti, samo Ethernet je podržan
	if (pcap_datalink(device_handle) != DLT_EN10MB)
	{
		printf("\nThis program works only on Ethernet networks.\n");
		pcap_freealldevs(devices);
		return -1;
	}

	// Postavljanje izraza za filter
	// Savet: pronađi sopstvenu MAC adresu korišćenjem ipconfig
	unsigned int netmask;
	char filter_exp[] = "ether src host  90-1B-0E-67-9A-37 and ip and (udp or tcp)";
	struct bpf_program fcode;

	// Dobavi masku prve adrese interfejsa
	if (device->addresses != NULL)
		netmask = ((struct sockaddr_in *) (device->addresses->netmask))->sin_addr.s_addr;
	// Ako interfejs nema adresu, pretpostavljamo da je u C klasi networka
	else
		netmask = 0xffffff;

	// Kompaliramo filter
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
    
	// Oslobađamo listu uređaja (više nam nije potrebna)
    pcap_freealldevs(devices);

	// Započni prihvatanje paketa
	int result;							// rezultat pcap_next_ex funkcije
	int packet_counter = 0;				// broji pakete kao bismo mogli da ih numerišemo
	struct pcap_pkthdr* packet_header;	// heder paketa (timestamp i dužina)
	const unsigned char* packet_data;	// sadržaj paketa
	
	// Prihvatanje paketa
	while ((result = pcap_next_ex(device_handle, &packet_header, &packet_data)) >= 0)
	{
		// Provera da li je timeout istekao
		if (result == 0)
			continue;
		
		// result > 0 znači da smo prihvatili paket
		// Ispiši timestamp i dužinu prihvaćenog paketa
		printf("\nNew packet arrived. Size: %d byte\n", packet_header->len);
		printf("\nPacket No.  \t%d", ++packet_counter);
		
		/* DATA LINK LAYER - Ethernet */
		// Dobavljanje pozicije ethernet hedera
		ethernet_header * eh = (ethernet_header *)packet_data;

		printf("\n\nEthernet \tDestination MAC address:\t%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
			eh->dest_address[0], eh->dest_address[1], eh->dest_address[2], 
			eh->dest_address[3], eh->dest_address[4], eh->dest_address[5]);

		/* NETWORK LAYER - IPv4 */
		// Dobavljanje pozicije ip hedera
		ip_header* ih = (ip_header*) (packet_data + sizeof(ethernet_header));
		
		printf("\n\nIP \tSource IP address:\t\t%u.%u.%u.%u", 
			ih->src_addr[0], ih->src_addr[1], 
			ih->src_addr[2], ih->src_addr[3]);
		
		/* TRANSPORT LAYER */
		unsigned char* app_data;
		int app_length;

		/* UDP */
		if (ih->next_protocol == 17)
		{
			udp_header* uh = (udp_header*) ((unsigned char*)ih + ih->header_length * 4);

			printf("\n\nUDP  \tDestination Port:\t\t%u", ntohs(uh->dest_port));

			// Dobavljanje pozicije podataka od aplikacije
			app_data = (unsigned char *)uh + sizeof(udp_header);

			// Ukupna dužina hedera i podataka od aplikacije
			app_length = ntohs(uh->datagram_length) - sizeof(udp_header);
		}

		/* TCP */
		else if (ih->next_protocol == 6)
		{
			tcp_header* th = (tcp_header*) ((unsigned char*)ih + ih->header_length * 4);

			printf("\n\nTCP  \tDestination Port:\t\t%u", ntohs(th->dest_port));

			// Dobavljanje pozicije podataka od aplikacije
			app_data = (unsigned char *)th + th->header_length * 4;

			// Ukupna dužina hedera i podataka od aplikacije
			app_length = packet_header->len - (sizeof(ethernet_header) + ih->header_length * 4 + th->header_length * 4);
		}

		/* OSTALO */
		else
		{
			return -1;
		}
		
		/* APPLICATION LAYER */
		// Ispiši heder i podatke od aplikacije
		printf("\n-------------------------------------------------------------\n\t");
		for (int i = 0; i < app_length; i = i + 1)
		{
			// Ispis svakog bajta kao dva hex broja
			printf("%.2x ", (app_data)[i]);

			// 16 bajta po liniji (word)
			if ((i+1) % 16 == 0)
				printf("\n\t");
		}
		printf("\n-------------------------------------------------------------");

		// Ako smo prikupili 10 paketa, prestani sa radom
		if (packet_counter == 10)
			break;
	}

	// Ako je došlo do greške
	if (result == -1) 
	{
		printf("Error reading the packets: %s\n", pcap_geterr(device_handle));
		return -1;
	}
	
	printf("\nPress any key to close application...");
	getchar();
	
	return 0;
}
