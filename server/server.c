// References
// https://beej.us/guide/bgnet/html/#client-server-background page 31
// Communication Networks Textbook Section 2.4

// Server Program(server.c)
//      Its execution command has the following structure : server<UDP listen port>
//      Upon execution, the server should:
//      1. Open a UDP socket and listen at the specified port number
//      2. Receive a message from the client
//          a. if the message is “ftp”, reply with a message “yes” to the client.
//          b.else, reply with a message “no” to the client.

/* Server using UDP */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "packet.h"
#include "helper.h"
#include <limits.h>

#define SERVER_UDP_PORT "50000" // Well-known port
#define MAX_LEN 10000             // Beej and the textbook use different ones so idk

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char **argv)
{
    // Server information
    int sockfd;                  // Listen on sock_fd, socket descriptor
    const char *port = NULL;     // Server port number
    struct addrinfo hints;       // Information about the server
    struct addrinfo *res = NULL; // Will point to results
    int yes = 1;
    int rv; // Used for error checking

    // Client information
    struct sockaddr_storage client;        // Client's address information
    socklen_t client_len = sizeof(client); // Client size
    char buf[MAX_LEN];                     // Where the message from client will be stored

    const char *message = NULL; // Message server will send to client

    ssize_t n; // Used for error checking

    // Set the UDP server listen port
    switch (argc)
    {
    case 1:
        port = SERVER_UDP_PORT; // If no arguments, just start the server at UDP port 50000
        break;
    case 2:
        port = argv[1]; // Start the server at specified port
        break;
    default: // Invalid entry
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(1);
    }

    // Set up the information about the server socket
    memset(&hints, 0, sizeof(hints)); // Make sure empty
    hints.ai_family = AF_INET;        // Use IPv4
    hints.ai_socktype = SOCK_DGRAM;   // For UDP
    hints.ai_flags = AI_PASSIVE;      // Use my IP

    if ((rv = getaddrinfo(NULL, port, &hints, &res)) != 0)
    {
        fprintf(stderr, "server: getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // Create the socket
    // socket returns the socket descriptor number (? idrk what that means)
    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
    {
        perror("server: socket\n");
        exit(1);
    }

    //? Umm this is in Beej but idrk what it does
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("server: setsockopt\n");
        exit(1);
    }

    // Bind the socket port to an IP address
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        close(sockfd);
        perror("server: bind\n");
        exit(1);
    }

    freeaddrinfo(res); // all done with this structure

    // We have now successfully created a socket and binded it to an address
    // Do not need to accept(), just recvfrom():

    // while (1)
    // { // Main recvform() loop
        printf("server: waiting for connections on port %s...\n", port);

        // recvfrom returns the number of bytes recieved or -1 on error
        // recvfrom will block until there is data to read
        n = recvfrom(sockfd, buf, sizeof buf, 0, (struct sockaddr *)&client, &client_len);
        if (n < 0)
        {
            perror("server: recvfrom\n");
            fprintf(stderr, "Can’t receive datagram\n");
            exit(1);
        }

        // Check the data recieved and send data to client
        message = strcmp(buf, "ftp") == 0 ? "yes" : "no";
        n = sendto(sockfd, message, strlen(message) + 1, 0, (struct sockaddr *)&client, client_len);
        if (n < 0)
        {
            perror("server: sendto\n");
            fprintf(stderr, "Can’t send datagram\n");
            exit(1);
        }

        int waitingForPacket = 1;
        int numPackets = INT_MAX;
        FILE *fptr = NULL;

        while (waitingForPacket <= numPackets) // Main recieve packet loop
        {
            // Wait to recieve packet
            printf("Waiting to recieve packet %d...\n", waitingForPacket);
            n = recvfrom(sockfd, buf, sizeof buf, 0, (struct sockaddr *)&client, &client_len);
            printf("Recieved a packet\n");

            if (n < 0)
            {
                fprintf(stderr, "server: recvfrom can’t receive datagram\n");
                if (fptr != NULL)
                {
                    fclose(fptr);
                }
                exit(1);
            }

            // Read packet using delimiter ":"
            char *array[4];
            struct Packet packet;

            // printf("%s\n", buf);

            array[0] = strtok(buf, ":");

            for(int i = 1; i < 3; ++i){
                array[i] = strtok(NULL, ":");   // strtok adds \0 to end of strings
            }

            sscanf(array[0], "%d", &(packet.totalFrag));
            sscanf(array[1], "%d", &(packet.fragNo));
            sscanf(array[2], "%d", &(packet.size));

            array[3] = strtok(NULL, "\0");
            packet.filename = array[3];

            int infoSize = countDigits(packet.totalFrag) + 1 + countDigits(packet.fragNo) + 1 + countDigits(packet.size) + 1 + strlen(packet.filename) + 1 + 1;

            memcpy(packet.filedata, buf + infoSize, packet.size);

            printf("%d %d %d %s\n", packet.totalFrag, packet.fragNo, packet.size, packet.filename);

            if(packet.fragNo == waitingForPacket){
                waitingForPacket++;

                if (fptr == NULL){
                    numPackets = packet.totalFrag;
                    fptr = fopen(packet.filename, "w+");
                }

                fwrite(packet.filedata, sizeof(char), packet.size, fptr);
            }
            
            char temp[256];
            sprintf(temp, "ACK %d \0", waitingForPacket);
            message = temp;

            printf("ACKING THIS NUMBER: %s", message);

            n = sendto(sockfd, message, strlen(message) + 1, 0, (struct sockaddr *)&client, client_len);
            if (n < 0)
            {
                perror("server: sendto\n");
                fprintf(stderr, "Can’t send datagram\n");
                if (fptr != NULL)
                {
                    fclose(fptr);
                }
                exit(1);
            }
        }

        fclose(fptr);

        printf("Finished recieving packets\n\n");
    // }

    // Close the socket
    close(sockfd);
    return 0;
}
