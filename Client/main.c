#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>

struct data_packet {
    long id;
    struct timeval timestamp;
};

void send_packet(int socketfd, struct sockaddr_in server_address) {
    static long packet_id = 0;

    struct data_packet packet;
    packet.id = packet_id++;
    gettimeofday(&packet.timestamp, NULL);

    long len = sendto(socketfd, (const struct data_packet *) &packet, sizeof(packet),
            0, (const struct sockaddr *) &server_address, sizeof(server_address));
    if (len == -1) {
        perror("failed to send packet");
    }
}

int main(int argc, char* argv[]) {
    puts("Client starting");
    if (argc < 3) {
        fprintf(stderr, "usage: '%s [IP_ADDR] [PORT] [PACKET_AMOUNT]'\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[2]);
    int packet_amount = atoi(argv[3]);

    struct sockaddr_in server_address = {0};
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(argv[1]);

    puts("Client ready");
    for (int i = 0; i < packet_amount; ++i) {
        send_packet(socketfd, server_address);
    }

    close(socketfd);
    return 0;
}
