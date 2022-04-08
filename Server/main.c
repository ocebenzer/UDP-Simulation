#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>

struct data_packet {
    long id;
    struct timeval time_client;
    struct timeval time_server;
    double time_relative;
};

/*
 * double theTime = tv.tv_sec + (0.000001f * tv.tv_usec);
 * printf( "time as double = %f\n", theTime );
 * printf( "sys time sec = %d\n", (unsigned int)tv.tv_sec );
 * printf( "sys time usec = %d\n", (unsigned int)tv.tv_usec );
 * printf( "sys time = %d\n", (unsigned int)time(0) );
 */

double time_to_double(struct timeval time) {
    return time.tv_sec + 0.000001*time.tv_usec;
}

void recv_packet(int socketfd, struct data_packet *packet){
    recv(socketfd, packet, sizeof(*packet), MSG_WAITALL);
    gettimeofday(&packet->time_server, NULL);
    packet->time_relative = time_to_double(packet->time_server) - time_to_double(packet->time_client);
}

int main(int argc, char* argv[]) {
    puts("Server starting");
    if (argc < 3) {
        fprintf(stderr, "usage: '%s [PORT] [PACKET_AMOUNT]'\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    int packet_amount = atoi(argv[2]);
    struct data_packet packets[packet_amount];

    struct sockaddr_in server_address = {0};
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketfd == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int rc = bind(socketfd, (const struct sockaddr *) &server_address, sizeof(server_address));
    if (rc == -1) {
        perror("failed to bind socket");
        close(socketfd);
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen("./log.csv", "w");
    puts("Server ready");
    fprintf(file, "id, diff, time send, time receive\n");
    for (int i = 0; i < packet_amount; i++) {
        recv_packet(socketfd, &packets[i]);
        fprintf(file, "%ld, %lf, %ld.%06ld, %ld.%06ld\n",packets[i].id, packets[i].time_relative,
                packets[i].time_server.tv_sec, packets[i].time_server.tv_usec,
                packets[i].time_client.tv_sec, packets[i].time_client.tv_usec);
    }
}
