#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "./packet.h"
#include "./helper.h"

// Reads data from file fileName and splits it into Packets
struct Packet **createPackets(char *fileName)
{
    // Open the file
    FILE *fptr;
    int fileSize;

    fptr = fopen(fileName, "r");

    if (fptr == NULL)
    {
        fprintf(stderr, "Cannot open %s.\n", fileName);
        return NULL;
    }

    // Number of bytes in the file
    fseek(fptr, 0L, SEEK_END);
    fileSize = ftell(fptr);
    rewind(fptr); // Go back to the beginning

    // Create all the packets
    struct Packet **packets;
    int numPackets;
    int lastPacketSize;

    numPackets = ceil((float)fileSize / (float)MAX_PACKET_SIZE);
    packets = (struct Packet **)malloc(sizeof(struct Packet *) * numPackets);
    lastPacketSize = fileSize % MAX_PACKET_SIZE == 0 ? MAX_PACKET_SIZE : fileSize % MAX_PACKET_SIZE;

    for (int i = 0; i < numPackets - 1; ++i)
        packets[i] = createPacket(numPackets, i + 1, MAX_PACKET_SIZE, fileName, fptr);

    packets[numPackets - 1] = createPacket(numPackets, numPackets, lastPacketSize, fileName, fptr);

    printf("Finished creating all %d packets\n\n", numPackets);

    // Close the file
    fclose(fptr);

    return packets;
}

// Create a packet
struct Packet *createPacket(unsigned int totalFrag,
                             unsigned int fragNo,
                             unsigned int size,
                             char *fileName,
                             FILE *fptr)
{
    // printf("Fragement # %d\n", fragNo);
    // printf("size: %d\n", size);

    // Create a Packet
    struct Packet *packet = (struct Packet *)malloc(sizeof(struct Packet));

    packet->totalFrag = totalFrag;
    packet->fragNo = fragNo;
    packet->size = size;
    packet->filename = fileName;

    fread(packet->filedata, sizeof(char), size, fptr);

    return packet;

    /*
    // make a string for the packet
    char *packetString;

    // add total frag, packet number, packet info & data size, fileName, data
    char totalFragString[countDigits(totalFrag) + 1];
    char fragNumStrong[countDigits(fragNo) + 1];
    char sizeString[countDigits(size) + 1];

    sprintf(totalFragString, "%d", totalFrag);
    sprintf(fragNumStrong, "%d", fragNo);
    sprintf(sizeString, "%d", size);

    char *temp = totalFragString;
    strcat(temp, ":");
    strcat(temp, fragNumStrong);
    strcat(temp, ":");
    strcat(temp, sizeString);
    strcat(temp, ":");
    strcat(temp, fileName);
    strcat(temp, ":");

    int infoSize = strlen(temp);

    Remove \0 from temp
    char temp2[strlen(temp)];
    memcpy(temp2, temp, strlen(temp));

    char dataString[size];
    fread(dataString, sizeof(char), size, fptr);

    packetString = (char *)malloc(sizeof(char) * (size + infoSize));
    //mem copy into packetstring info part
    memcpy(packetString, temp2, infoSize);

    memcpy(packetString + infoSize, dataString, size);

    //mem copy into packetstring + length of data
    printf("%s\n", packetString);
    */
}

char *Packet_to_char_array(struct Packet *packet, int *arraySize)
{
    char *PacketCharArray;
    int packetSize;
    int infoSize;
    int totalFragDigits = countDigits(packet->totalFrag);
    int fragNoDigits = countDigits(packet->fragNo);
    int sizeDigits = countDigits(packet->size);
    int fileNameSize = strlen(packet->filename);

    infoSize = totalFragDigits + 1 + fragNoDigits + 1 + sizeDigits + 1 + fileNameSize + 1;
    packetSize = infoSize + MAX_PACKET_SIZE;

    PacketCharArray = (char *)malloc(sizeof(char) * packetSize);
    // PacketCharArray = (char *)malloc(sizeof(char) * packetSize + 1);

    char totalFragString[totalFragDigits + 1];
    char fragNumStrong[fragNoDigits + 1];
    char sizeString[sizeDigits + 1];

    sprintf(totalFragString, "%d", packet->totalFrag);
    sprintf(fragNumStrong, "%d", packet->fragNo);
    sprintf(sizeString, "%d", packet->size);

    char *infoString = totalFragString;
    strcat(infoString, ":");
    strcat(infoString, fragNumStrong);
    strcat(infoString, ":");
    strcat(infoString, sizeString);
    strcat(infoString, ":");
    strcat(infoString, packet->filename);
    strcat(infoString, ":");

    memcpy(PacketCharArray, infoString, infoSize);
    memcpy(PacketCharArray + infoSize, packet->filedata, packet->size);

    // PacketCharArray[packetSize] = '\0';
    // printf("%s\n", PacketCharArray);

    *arraySize = packetSize;

    return PacketCharArray;
}
