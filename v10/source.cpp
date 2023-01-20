// Interpretacija sadrzaja paketa

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
pcap_if_t* select_device(pcap_if_t* devices);
void print_raw_data(unsigned char* data, int data_length);

// Ispis hedera paketa
void print_winpcap_header(const struct pcap_pkthdr* packet_header, int packet_counter);
void print_ethernet_header(ethernet_header* eh);
void print_ip_header(ip_header* ih);
void print_icmp_header(icmp_header* icmph);
void print_udp_header(udp_header* uh);
void print_application_data(unsigned char* data, long data_length);

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
    
	// Izbor jednog uređaja iz liste
	device = select_device(devices);
	
	// Provera da li je uređaj validan
	if (device == NULL)
	{
		pcap_freealldevs(devices);
		return -1;
	}

	// Otvaranje uređaja za prihvatanje
    if ((device_handle = pcap_open_live(device->name, 65536, 1, 500, error_buffer)) = NULL)
    {
        printf("\nUnable to open the adapter. %s is not supported by WinPcap\n", device->name);
        pcap_freealldevs(devices);
        return -1;
    }

	// Proveri sloj veze; radi jednostavnosti, samo Ethernet je podržan
	if (pcap_datalink(device_handle) != DLT_EN10MB)
	{
		printf("\nThis program works only on Ethernet networks.\n");
		return -1;
	}

	// Postavljanje izraza za filter
	unsigned int netmask;
	char filter_exp[] = "ip and (udp or icmp)";
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
    
	// ZADATAK - ČITA PODATKE I ISPISUJE IH
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

		/* WINPCAP */
		// Ispis pseudohedera od winpcap
		print_winpcap_header(packet_header, ++packet_counter);

		/* DATA LINK LAYER - Ethernet */
		// Dobavljamo poziciju ethernet hedera
		ethernet_header* eh = (ethernet_header*)packet_data;

		// Ispis ethernet hedera
		print_ethernet_header(eh);

		/* NETWORK LAYER - IPv4 */
		// Dobavljamo poziciju ip hedera
		ip_header* ih = (ip_header*) (packet_data + sizeof(ethernet_header));
		
		// Ispis ip hedera
		print_ip_header(ih);

		/* TRANSPORT LAYER - UDP */
		// Dobavljamo poziciju udp hedera
		// header_lenghth se računa korišćenjem reči/word (1 word = 4 bajta)
		int ip_len = ih->header_length * 4;

		// Ako je ICMP
		if (ih->next_protocol == 1)
		{
			// TODO 1: 
			// Ispiši ICMP heder
			icmp_header* icmph = (icmp_header*)((unsigned char*)ih + ip_len);

			print_icmp_header(icmph);
		}
		// Ako je UDP
		else if (ih->next_protocol == 17) 
		{
			udp_header* uh = (udp_header*)((unsigned char*)ih + ip_len);

			// TODO 1:
			// Ispiši UDP heder
			print_udp_header(uh);

			/* APPLICATION LAYER */
			// TODO 2:
			// Dobavi poziciju podataka aplikacije
			unsigned char* app_data = (unsigned char*)uh + sizeof(udp_header);

			// TODO 3:
			// Ukupna dužina hedera i podataka od aplikacije
			int app_length = ntohs(uh->datagram_length) - sizeof(udp_header);

			// TODO 4:
			// Ispis hedera i podataka od aplikacije
			print_application_data(app_data, app_length);
		}

		// Prihvatanje sledećeg paketa
		printf("\n\nPress enter to receive new packet\n");
		getchar();
    }
    
    if (result == -1)
	{
        printf("Error reading the packets: %s\n", pcap_geterr(device_handle));
        return -1;
    }
    
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
            printf(" (No description available)\n");
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
    
	// Skok na selektovani uređaj
    for (device = devices, i = 0; i < device_number - 1; device = device->next, i++);

	return device;  
}

// Ispis sirovih podataka iz hedera i od aplikacija
void print_raw_data(unsigned char* data, int data_length)
{
	printf("\n-------------------------------------------------------------\n\t");
	for (int i = 0; i < data_length; i = i + 1)
	{
		// Ispis svakog bajta kao dva hex broja
		printf("%.2x ", ((unsigned char*)data)[i]);

		// 16 bytes per line (word)
		if ((i + 1) % 16 == 0)
			printf("\n\t");
	}
	printf("\n-------------------------------------------------------------");
}

// Ispis pseudohedera generisanog od WinPcap drajvera
void print_winpcap_header(const struct pcap_pkthdr* packet_header, int packet_counter)
{
	printf("\n\n=============================================================");
	printf("\n\tWINPCAP PSEUDO LAYER");
	printf("\n-------------------------------------------------------------");

	time_t timestamp;			// Sirovo vreme kada je paket primljen (u bitima)
	struct tm* local_time;		// Lokalno vreme kada je paket primljen
	char time_string[16];		// Lokalno vreme pretvoreno u string

	// Pretvaranje timestamp u format koji možemo da pročitamo
	timestamp = packet_header->ts.tv_sec;
	local_time = localtime(&timestamp);
	strftime(time_string, sizeof time_string, "%H:%M:%S", local_time);

	// Ispis timestamp i dužine paketa
	printf("\n\tPacket number:\t\t%u", packet_counter);
	printf("\n\tTimestamp:\t\t%s.", time_string);
	printf("\n\tPacket length:\t\t%u ", packet_header->len);
	printf("\n=============================================================");

	return;
}

// Ispis sadržaja Ethernet hedera
void print_ethernet_header(ethernet_header * eh)
{
	printf("\n=============================================================");
	printf("\n\tDATA LINK LAYER  -  Ethernet");

	print_raw_data((unsigned char*)eh, sizeof(ethernet_header));

	printf("\n\tDestination address:\t%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
		eh->dest_address[0], eh->dest_address[1], eh->dest_address[2], 
		eh->dest_address[3], eh->dest_address[4], eh->dest_address[5]);
	printf("\n\tSource address:\t\t%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
		eh->src_address[0], eh->src_address[1], eh->src_address[2], 
		eh->src_address[3], eh->src_address[4], eh->src_address[5]);
	printf("\n\tNext protocol:\t\t0x%.4x", ntohs(eh->type));
	
	printf("\n=============================================================");

	return;
}

// Ispis sadržaja ip hedera
void print_ip_header(ip_header* ih)
{
	printf("\n=============================================================");
	printf("\n\tNETWORK LAYER  -  Internet Protocol (IP)");

	print_raw_data((unsigned char*)ih, ih->header_length * 4);

	printf("\n\tVersion:\t\t%u", ih->version);	
	printf("\n\tHeader Length:\t\t%u", ih->header_length * 4);
	printf("\n\tType of Service:\t%u", ih->tos);
	printf("\n\tTotal length:\t\t%u", ntohs(ih->length));
	printf("\n\tIdentification:\t\t%u", ntohs(ih->identification));
	printf("\n\tFragments:\t\t%u", ntohs(ih->fragm_fo));
	printf("\n\tTime-To-Live:\t\t%u", ih->ttl);
	printf("\n\tNext protocol:\t\t%u", ih->next_protocol);
	printf("\n\tHeader checkSum:\t%u", ntohs(ih->checksum));
	printf("\n\tSource:\t\t\t%u.%u.%u.%u", ih->src_addr[0], ih->src_addr[1], ih->src_addr[2], ih->src_addr[3]);
	printf("\n\tDestination:\t\t%u.%u.%u.%u", ih->dst_addr[0], ih->dst_addr[1], ih->dst_addr[2], ih->dst_addr[3]);
	
	printf("\n=============================================================");

	return;
}

// Ispis sadržaja icmp hedera
void print_icmp_header(icmp_header* icmph) {
	printf("\n=============================================================");
	printf("\n\tNETWORK LAYER  -  Internet Control Messaging Protocol (ICMP)");

	print_raw_data((unsigned char*)icmph, sizeof(icmp_header));

	printf("\n\tType:\t\t%u", icmph->type);
	printf("\n\tCode:\t%u", icmph->code);
	printf("\n\tChecksum:\t%u", ntohs(icmph->checksum));
	printf("\n\tData:\t\t%u %u %u %u", icmph->data[0], icmph->data[1], icmph->data[2], icmph->data[3]);

	printf("\n=============================================================");

	return;
}

// Ispis sadržaja UDP hedera
void print_udp_header(udp_header * uh)
{
	printf("\n=============================================================");
	printf("\n\tTRANSPORT LAYER  -  User Datagram Protocol (UDP)");

	print_raw_data((unsigned char*)uh, sizeof(udp_header));
	
	printf("\n\tSource Port:\t\t%u", ntohs(uh->src_port));
	printf("\n\tDestination Port:\t%u", ntohs(uh->dest_port));
	printf("\n\tDatagram Length:\t%u", ntohs(uh->datagram_length));
	printf("\n\tChecksum:\t\t%u", ntohs(uh->checksum));

	printf("\n=============================================================");

	return;
}

// Ispis sadržaja sloja aplikacije
void print_application_data(unsigned char* data, long data_length)
{
	printf("\n=============================================================");
	printf("\n\tAPPLICATION LAYER");

	print_raw_data(data, data_length);

	printf("\n=============================================================");
}
