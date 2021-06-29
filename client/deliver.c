// References
// https://beej.us/guide/bgnet/html/#client-server-background doc
// Communication Networks Textbook Section 2.4

// Client Program (deliver.c)
//      The client program will send a message to the server.
//      Its execution command should have the following structure:
//      deliver <server address> <server port number>
//      After executing the server, the client should:
//          1. Ask the user to input a message as follows: ftp <file name>
//          2. Check the existence of the file:
//              a. if exist, send a message “ftp” to the server
//              b. else, exit
//          3. Receive a message from the server:
//              a. if the message is “yes”, print out “A file transfer can start.”
//              b. else, exit

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>

#include <time.h>
#include <sys/time.h> // get timeofday()
#include <fcntl.h>

#include "./packet.h"
#include "./helper.h"
#include <stdint.h>

// referenced the talker.c code from Beej
// and client source from Beej page 34

#define SERVER_UDP_PORT 50000 // the port client will be connecting to
#define MAX_LEN 1000           // max # of bytes we can get at once
#define TIMEOUT_TIME 1000000
#define SERVER_IP "128.100.13.217"
#define K 4

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    char *serverName = NULL;
    int portNumber;

    if(argc != 3){
        printf("Using default values for port number and server IP address.\n");
        serverName = SERVER_IP;
        portNumber = SERVER_UDP_PORT;
    }else{
        serverName = argv[1];
        portNumber = atoi(argv[2]);
    }


    while(1){
        int sockfd;
        char buffer[MAX_LEN];            // message from server
        char messageToServer[4] = "ftp"; // message to server
        struct sockaddr_in servaddr;

        bool isInputting = true;
        char fileName[256];

        // Get user input and find the file
        while (isInputting)
        {
            char userInput[256];
            char *token = NULL;
            char *array[2];
            int i = 0; // Starter to store the strings into the array
            printf("Input a message in the form: ftp <file name>\n");

            // https://stackoverflow.com/questions/15472299/split-string-into-tokens-and-save-them-in-an-array/15472429
            fgets(userInput, sizeof(userInput), stdin); // split a string from https://www.educative.io/edpresso/splitting-a-string-using-strtok-in-c
            token = strtok(userInput, " ");             // Returns first token

            // Keep printing tokens while one of the delimiters present in userInput[]
            while (token != NULL)
            {
                if (i == 2)
                {
                    fprintf(stderr, "Input not accepted.\n");
                    exit(1);
                }
                array[i++] = token;
                token = strtok(NULL, " \n");
            }

            // not ftp we don't want that
            if (strcmp(array[0], "ftp") != 0)
            {
                fprintf(stderr, "Input not accepted.\n");
            }
            else if (access(array[1], F_OK) != 0)
            {
                fprintf(stderr, "File was not found.\n");
            }
            else
            {
                printf("File was found. Sending a message ftp to server.\n");
                isInputting = false;
                strcpy(fileName, array[1]);
                printf("filenamejhbhj\n");
            }
        }

        struct Packet **packets;

        // Create packets, must free each Packet in array as well as the array itself
        packets = create_packets(fileName);

        printf("kdsjfh\n");

        /*for (int i = 0; i < packets[0]->totalFrag; ++i)
        {
            printf("\nTotal frags: %d\n", packets[i]->totalFrag);
            printf("Frag number: %d\n", packets[i]->fragNo);
            printf("File name: %s\n", packets[i]->filename);
            printf("Data size: %d\n", packets[i]->size);
        }*/

        sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    
        // Creating socket file descriptor
        if (sockfd < 0)
        {
            perror("socket creation failed\n");
            return 1;
        }

        // Filling server information
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(portNumber);
        inet_aton(serverName, (struct in_addr *)(&(servaddr.sin_addr.s_addr)));

        ssize_t n; // Used for error checking
        unsigned int len = sizeof(servaddr);

        struct timeval startRTT; 
        struct timeval stopRTT; // RTT variables

        // Send message to server
        // start timer
        gettimeofday(&startRTT, NULL);
        n = sendto(sockfd, (const char *)messageToServer, sizeof(messageToServer),
                   MSG_CONFIRM, (const struct sockaddr *)&servaddr, len);

        if (n < 0)
        {
            perror("client: sendto\n");
            fprintf(stderr, "Can’t send datagram\n");
            exit(1);
        }

        // Recieve message from server
        n = recvfrom(sockfd, (char *)buffer, MAX_LEN,
                     MSG_WAITALL, (struct sockaddr *)&servaddr,
                     &len);
        
        fcntl(sockfd, F_SETFL, O_NONBLOCK);
        
        gettimeofday(&stopRTT, NULL);
        int sampleRTT = (1000000*stopRTT.tv_sec + stopRTT.tv_usec) - (1000000*startRTT.tv_sec + startRTT.tv_usec);
        printf("RTT from client to server: %d mews\n", sampleRTT);

        // MEASUREMENTS FROM PIAZZA @170
        int devRTT = sampleRTT / 2;
        // int timeout = 10000000;
        //printf("Timeout:%d\n", timeout);
        int timeout = sampleRTT + K * devRTT;




        // Check if message from buffer is "yes"
        if (n < 0)
        {
            perror("client: recvfrom\n");
            fprintf(stderr, "Can’t recvfrom datagram\n");
            //exit(1);
        }
        

        buffer[3] = '\0';

        if (strcmp(buffer, "yes") != 0)
        {
            perror("A file transfer cannot start.\n");
            exit(1);
        }

        memset(buffer, 0, sizeof(buffer));

        printf("Starting file transfer.\n");

        // Send strings made from info in packets
        int numPackets = packets[0]->totalFrag;
        bool DONE =false;

        for (int i = 0; i < numPackets; ++i)
        {
            // for (int j = 0; j < packets[0]->totalFrag; ++j)
            // {
            //     printf("\nTotal frags: %d\n", packets[j]->totalFrag);
            //     printf("Frag number: %d\n", packets[j]->fragNo);
            //     printf("File name: %s\n", packets[j]->filename);
            //     printf("Data size: %d\n", packets[j]->size);
            // }

            // Convert the Packet into a char *
 
            bool ACK = false;

            // keep sending same packet until ACK the current packet
            while(!ACK) {
                
                int packetSize;

                char *packetString = Packet_to_char_array(packets[i], &packetSize); // Must remember to free

                printf("%s\n", packetString);

                struct timeval start;
                struct timeval current;
                gettimeofday(&start, NULL);
                
                // Send the data over
                n = sendto(sockfd, packetString, packetSize,
                       MSG_CONFIRM, (const struct sockaddr *)&servaddr, len);
                
                printf("sending packet %d\n", i+1);
                if (n < 0)
                {
                    fprintf(stderr, "client: sendto can’t send datagram\n");
                    exit(1);
                }

                // printf("Packet %d out of %d sent. Waiting for response...\n", i + 1, numPackets);

                do{
                    // Wait for response from the server
                    n = recvfrom(sockfd, (char *)buffer, MAX_LEN,
                         MSG_WAITALL, (struct sockaddr *)&servaddr,
                         &len);
                    
                    gettimeofday(&current, NULL);
                    //printf("Timeout: %");
                    if (n > 0 && ((1000000*current.tv_sec + current.tv_usec) - (1000000*start.tv_sec + start.tv_usec)) < timeout) {
                        
                        // Check if ACK or NACK
                        if (strncmp(buffer, "ACK", 3) == 0) {
                            // parse the string
                            strtok(buffer, " ");
                            int ACKdNum;
                            char *temp = strtok(NULL, " ");
                            sscanf(temp, "%d", &(ACKdNum));

                            if(ACKdNum - 1 == numPackets){
                                DONE = true;
                            }

                            printf("THE ACKED NUMBER: %d\n", ACKdNum);                            
                            // set i = ACK'd -1
                            i = ACKdNum - 2;
                            
                            printf("Recieved ACK %d.\nSending next packet\n", i + 1);
                            printf("Got %s\n", buffer);
                            ACK = true;
                        }
                        else if (strcmp(buffer, "yes") == 0) {
                            continue;
                        }
                        else{
                            fprintf(stderr, "client: recvfrom got %s\n", buffer);
                            exit(1);
                        }
                    }
                } while(((1000000*current.tv_sec + current.tv_usec) - (1000000*start.tv_sec + start.tv_usec)) < timeout && !ACK);

                free(packetString);

                // if(ACK && i == 0)

            }
            
                if(DONE){
                    break;
                }
            // Loop and go onto next packet


        }

        printf("Finished transmitting all packets\n\n");

        // Free each Packet in packets
        for (int i = 0; i < numPackets; ++i){
            printf("%s\n", packets[i]->filename);
            free(packets[i]->filename);
            free(packets[i]);

        }


        // Free the whole packets array
        free(packets);

        close(sockfd);
    }

    return 0;
}
