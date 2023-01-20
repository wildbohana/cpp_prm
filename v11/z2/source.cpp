// Rad sa datotekama

// Onemoguci bespotrebne warninge
#ifdef _MSC_VER
	#define _CRT_SECURE_NO_WARNINGS
#endif

// Include libraries
#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "conio.h"
#include "pcap.h"
#include "protocol_headers.h"

// Deklaracije funkcija
void packet_handler(unsigned char *param, const struct pcap_pkthdr *packet_header, const unsigned char *packet_data);

pcap_dumper_t* icmp_dumper;
pcap_dumper_t* udp_dumper;
pcap_dumper_t* tcp_dumper;
pcap_dumper_t* arp_dumper;

int main()
{
	pcap_t* device_handle;
	char error_buffer[PCAP_ERRBUF_SIZE];
	
	// Otvaranje fajla
	if ((device_handle = pcap_open_offline("example.pcap", error_buffer)) == NULL)
	{
		printf("\n Unable to open the file %s.\n", "example.pcap");
		return -1;
	}

	// Otvaranje izlaznih fajlova
	arp_dumper = pcap_dump_open(device_handle, "arp_packets.pcap");
	icmp_dumper = pcap_dump_open(device_handle, "icmp_packets.pcap");
	udp_dumper = pcap_dump_open(device_handle, "udp_packets.pcap");
	tcp_dumper = pcap_dump_open(device_handle, "tcp_packets.pcap");	

	if (icmp_dumper == NULL || udp_dumper == NULL || tcp_dumper == NULL || arp_dumper == NULL)
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

	// Postavljanje izraza za filter
	struct bpf_program fcode;

	// Kompaliranje filtera
	if (pcap_compile(device_handle, &fcode, "arp or (ip and (icmp or udp or tcp))", 1, 0xffffff) < 0)
	{
		printf("\n Unable to compile the packet filter. Check the syntax.\n");
		return -1;
	}

	// Postavljanje filtera
	if (pcap_setfilter(device_handle, &fcode) < 0)
	{
		printf("\n Error setting the filter.\n");
		return -1;
	}

	// Čitaj i "primaj" pakete dok ne dođeš do EOF
	pcap_loop(device_handle, 0, packet_handler, NULL);

	// Zatvori sve fajlove vezane za device_handle i dealociraj resurce
	pcap_close(device_handle);

	printf("\nFile: example.pcap is successfully processed.\n");

	return 0;
}

// Callback funkcija koju WinPcap poziva za svaki pristigli paket
void packet_handler(unsigned char* user, const struct pcap_pkthdr* packet_header, const unsigned char* packet_data)
{
	/* DATA LINK LAYER - Ethernet */

	// Dobavljanje pozicije ethernet hedera
	ethernet_header* eh = (ethernet_header*)packet_data;

	// ARP
	if (ntohs(eh->type) == 0x806)
	{
		pcap_dump((unsigned char*) arp_dumper, packet_header, packet_data);
		return;
	}
	
	/* NETWORK LAYER - IPv4 */
	// Dobavljanje pozicije ip hedera
	ip_header* ih = (ip_header*) (packet_data + sizeof(ethernet_header));
	
	// TRANSPORT LAYER
	switch (ih->next_protocol)
	{
		// ICMP
		case 1:
			pcap_dump((unsigned char*) icmp_dumper, packet_header, packet_data);
			break;

		// TCP
		case 6:
			pcap_dump((unsigned char*) tcp_dumper, packet_header, packet_data);
			break;

		// UDP
		case 17:
			pcap_dump((unsigned char*) udp_dumper, packet_header, packet_data);
			break;
	}
}
