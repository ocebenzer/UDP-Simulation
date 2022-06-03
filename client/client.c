#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>

struct data_packet {
    int id;
    struct timeval timestamp;
};

void send_packet(int socketfd, struct sockaddr_in server_address, int packet_id) {
    struct data_packet packet;
    packet.id = packet_id;
    gettimeofday(&packet.timestamp, NULL);

    printf("send_packet - before\n");
    fflush(stdout);
    long len = sendto(socketfd, (const struct data_packet *) &packet, sizeof(packet),
            0, (const struct sockaddr *) &server_address, sizeof(server_address));
    if (len == -1) {
        perror("failed to send packet");
    }
    printf("send_packet - after\n");
    fflush(stdout);
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        fprintf(stderr, "usage: '%s [SERVER_IP] [PORT] [PACKET_AMOUNT] [DELAY_MULTIPLIER]'\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char* server_ip = argv[1];
    int port = atoi(argv[2]);
    int packet_amount = atoi(argv[3]);
    int delay_multiplier = atoi(argv[4]);

    struct sockaddr_in server_address = {0};
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(server_ip);

    printf("Starting to send Packets to %s:%d\n", server_ip, port);
    for (int i = 0; i < packet_amount; i++) {
        printf("Trying to send Packet#%d/%d\n", i+1, packet_amount);
        fflush(stdout);
        send_packet(socketfd, server_address, i+1); // packet ids start from 1
        int delay = 1 + rand() % 4; // 1 - 5
        printf("Packet #%d sent\n", i+1);
        usleep(delay * delay_multiplier);
    }

    send_packet(socketfd, server_address, -1);
    close(socketfd);
    return 0;
}
