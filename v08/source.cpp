// Dobavljanje informacija o mreznim karticama

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

// Deklaracije funkcija
void print_interface(pcap_if_t* dev);
char* convert_sockaddr_to_string(struct sockaddr* addr);

int main()
{
	pcap_if_t* devices;						// Lista kontrolera
	pcap_if_t* device;						// Jedan kontroler
	char errorMsg[PCAP_ERRBUF_SIZE + 1];	// Bafer za greške

	// Dobavljanje liste svih dostupnih kontrolera
	if (pcap_findalldevs(&devices, errorMsg) == -1)
	{
		printf("Error in pcap_findalldevs: %s\n", errorMsg);
		return -1;
	}

	// Ispis svih dostupnih kontrolera
	// Ispis svih dostupnih informacija o kontroleru
	for (device = devices; device; device = device->next)
	{
		print_interface(device);
	}

	// Oslobađanje liste uređaja
	pcap_freealldevs(devices);

	// Da vidimo ispis konzole
	printf("\nPress any key to exit: ");
	_getch();

	return 0;
}

// Ispis informacija
void print_interface(pcap_if_t* dev)
{
	pcap_addr_t* addr;

	printf("\n\t ============================= Interface ====================================\n\n");

	/* Name */
	printf("\t Name: \t\t %s\n", dev->name);

	/* Description */
	if (dev->description)
		printf("\t Description: \t %s\n", dev->description);

	// TODO 1
	// Ispiši sve logičke adrese, subnet maske i broadcast adrese na interfejsu

	/* Loopback Address*/
	printf("\t Loopback: \t %s\n",(dev->flags & PCAP_IF_LOOPBACK) ? "yes" : "no");

	/* IP addresses */
	for (addr = dev->addresses; addr; addr = addr->next)
	{
		printf("\n\t ADDRESS\n");

		switch (addr->addr->sa_family)
		{
			case AF_INET:
			{

				printf("\t # Address Type: \t IPv4\n");

				if (addr->addr)
					printf("\t # Address: \t\t %s\n", convert_sockaddr_to_string(addr->addr));

				if (addr->netmask)
					printf("\t # Subnet mask: \t %s\n", convert_sockaddr_to_string(addr->netmask));

				if (addr->broadaddr)
					printf("\t # Broadcast Address: \t %s\n", convert_sockaddr_to_string(addr->broadaddr));

				break;
			}

			case AF_INET6:
			{
				printf("\t # Address Type: \t IPv6\n");
				break;
			}

			default:
			{
				printf("\t # Address Type: \t Other\n");
				break;
			}
		}
	}
}

// TODO 2:
// Pretvori sockaddr strukturu u string da bi se mogla ispisati
char* convert_sockaddr_to_string(struct sockaddr* address)
{
	return inet_ntoa(((struct sockaddr_in*) address)->sin_addr);
}
