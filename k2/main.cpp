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

int packet_counter = 0;
int najmanjiTTL = 1000;

void packet_handler(unsigned char* param, const struct pcap_pkthdr* packet_header, const unsigned char* packet_data);
pcap_dumper_t* encrypted_dumper;
int arpBrojac = 0;

int main()
{
	int i = 0;
	pcap_t* device_handle;
	char error_buffer[PCAP_ERRBUF_SIZE];

	// Fajl za čitanje
	if ((device_handle = pcap_open_offline("example.pcap", error_buffer)) == NULL)
	{
		printf("\n Unable to open the file %s.\n", "example.pcap");
		return -1;
	}

	// Fajl za pisanje
	encrypted_dumper = pcap_dump_open(device_handle, "encrypted_packets.pcap");
	if (encrypted_dumper == NULL)
	{
		printf("\n Error opening output file\n");
		return -1;
	}

	// Provera za Ethernet
	if (pcap_datalink(device_handle) != DLT_EN10MB)
	{
		printf("\nThis program works only on Ethernet networks.\n");
		return -1;
	}

	// Filter (da li uopšte treba?)
	unsigned int netmask = 0xffffff;
	char filter_exp[] = "arp or (ip and (icmp or udp or tcp))";
	struct bpf_program fcode;

	if (pcap_compile(device_handle, &fcode, filter_exp, 1, netmask) < 0)
	{
		printf("\n Unable to compile the packet filter. Check the syntax.\n");
		return -1;
	}

	if (pcap_setfilter(device_handle, &fcode) < 0)
	{
		printf("\n Error setting the filter.\n");
		return -1;
	}

	// Pročitaj tačno 10 paketa
	pcap_loop(device_handle, 10, packet_handler, (unsigned char*)encrypted_dumper);

	pcap_close(device_handle);
	printf("\nFile: example.pcap is successfully processed.\n");

	return 0;
}

void packet_handler(unsigned char* param, const struct pcap_pkthdr* packet_header, const unsigned char* packet_data)
{
	// WINPCAP HEADER -> packet_header
	int velicinaPaketa = packet_header->len;

	// ETHERNET HEADER
	ethernet_header* eh = (ethernet_header*)packet_data;

	printf("\n\tDestination address:\t%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
		eh->dest_address[0], eh->dest_address[1], eh->dest_address[2], 
		eh->dest_address[3], eh->dest_address[4], eh->dest_address[5]);

	// ARP PROTOCOL
	if (ntohs(eh->type) == 0x806)
	{
		pcap_dump((unsigned char*) arp_dumper, packet_header, packet_data);
		return;
	}
	// IP PROTOCOL
	else if (ntohs(eh->type) == 0x0800)
	{
		ip_header* ih = (ip_header*) (packet_data + sizeof(ethernet_header));
		int ip_len = ih->header_length * 4;

		printf("\n\tSource:\t\t\t%u.%u.%u.%u", 
			ih->src_addr[0], ih->src_addr[1], 
			ih->src_addr[2], ih->src_addr[3]);
		printf("\n\tDestination:\t\t%u.%u.%u.%u", 
			ih->dst_addr[0], ih->dst_addr[1], 
			ih->dst_addr[2], ih->dst_addr[3]);

		// IP -> ICMP HEADER
		if (ih->next_protocol == 1)
		{
			icmp_header* icmph = (icmp_header*)((unsigned char*)ih + ip_len);

			printf("\n\tData:\t\t%u %u %u %u", 
				icmph->data[0], icmph->data[1], 
				icmph->data[2], icmph->data[3]);

			return;
		}

		// IP -> UDP HEADER
		if (ih->next_protocol == 17)
		{		
			udp_header* uh = (udp_header*)((unsigned char*)ih + ip_len);
		
			printf("\n\tSource Port:\t\t%u", ntohs(uh->src_port));
			printf("\n\tDestination Port:\t%u", ntohs(uh->dest_port));
			printf("\n\tDatagram Length:\t%u", ntohs(uh->datagram_length));
		
			// UDP -> APPLICATION LAYER
			unsigned char* app_data = (unsigned char*)uh + sizeof(udp_header);
			int app_length = ntohs(uh->datagram_length) - sizeof(udp_header);
			
			// Ispis ASCII
			for (int i = 0; i < 16; i++)
			{
				printf("%c", app_data[i]);
			}
			printf("\n");

			return;
		}

		// IP -> TCP HEADER
		if (ih->next_protocol == 6)
		{
			tcp_header* th = (tcp_header*) ((unsigned char*)ih + ih->header_length * 4);
			int velicina_prozora = ntohs(th->windows_size);
			printf("Velicina TCP prozora: %d\n", velicina_prozora);
			
			// TCP -> APPLICATION LAYER
			unsigned char* app_data =  (unsigned char *)th + th->header_length * 4;
			int app_length = packet_header->len - (sizeof(ethernet_header) + ih->header_length * 4 + th->header_length * 4);
			
			// TCP -> HTTP
			if (ntohs(th->src_port) == 80 || ntohs(th->dest_port) == 80)
			{
				printf("HTTP sadrzaj: ");
				app_data[app_length] = '\0';
				printf("%s\n", app_data);
			}
			else
			{
				for (int i = 0; i < 16; i++)
				{
					printf("%c", app_data[i]);
				}
				printf("\n");	
			}

			return;
		}
		
		else
			return;
	}
}

#endif
