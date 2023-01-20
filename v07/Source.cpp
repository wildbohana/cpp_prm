// Elektroenergetski softverski inzenjering
// Primenjene racunarske mreze u namenskim sistemima 2
// Vezba 3: Dobavljanje informacija o mreznim karticama

// We do not want the warnings about the old deprecated and unsecure CRT functions since these examples can be compiled under *nix as well
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

// Include libraries
#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "pcap.h"
#include "conio.h"

// Function declarations
void print_interface(pcap_if_t* dev);
char* convert_sockaddr_to_string(struct sockaddr* addr);

int main()
{
	pcap_if_t *devices;						// List of network interface controllers
	pcap_if_t *device;						// Network interface controller
	char errorMsg[PCAP_ERRBUF_SIZE + 1];	// Error buffer

	// Retrieve the list of available network interfaces
	if (pcap_findalldevs(&devices, errorMsg) == -1)
	{
		printf("Error in pcap_findalldevs: %s\n", errorMsg);
		return 1;
	}

	// Print all available network interfaces
	for (device = devices; device; device = device->next)
	{
		// Print all the available information on the given interface
		print_interface(device);
	}

	// Free the device list
	pcap_freealldevs(devices);

	// For demonstration purpose
	printf("\nPress any key to exit: ");
	_getch();

	return 0;
}

void print_interface(pcap_if_t *dev)
{
	printf("\n\t ============================= Interface ====================================\n\n");

	/* Name */
	printf("\t Name: \t\t %s\n", dev->name);

	/* Description */
	if (dev->description)
		printf("\t Description: \t %s\n", dev->description);

	// TODO 1: print all logical address, subnet mask and broadcast addresses on the interface
}

// TODO 2: convert sackaddr structure to string in order to have printable format
//char* convert_sockaddr_to_string(struct sockaddr* address)
//{
//	
//}
