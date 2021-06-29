#pragma once

#include <stdint.h>

#define MAX_PACKET_SIZE 1000

typedef struct Packet
{
    unsigned int totalFrag; // Total number of fragments of the file
    unsigned int fragNo;    // Sequence number of the fragment
    unsigned int size;       // Size of the fragment in bytes
    char *filename;          // Name of the data
    uint8_t filedata[MAX_PACKET_SIZE]; // Contents of the packet
} Packet;

struct Packet **create_packets(char *fileName);

struct Packet *createPacket(unsigned int totalFrag,
                      unsigned int fragNo,
                      unsigned int size,
                      char *filename,
                      FILE *fptr);

// Returns the size of the char array in arraySize
char *Packet_to_char_array(struct Packet *packet, int *arraySize);
