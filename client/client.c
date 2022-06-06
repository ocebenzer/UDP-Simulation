#include <math.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define POISSON_AVG 50
const double POISSON_LIMIT = exp(-POISSON_AVG);
int poisson_random() { // -ln(POISSON_LIMIT) will be avg
    int n = 0;
    double x = (double) rand() / INT_MAX;
    do {
        x *= (double) rand() / INT_MAX;
        n++;
    } while(x > POISSON_LIMIT);
    return n;
}

struct data_packet {
    int id;
    struct timeval timestamp;
};

void send_packet(int socketfd, struct sockaddr_in server_address, int packet_id) {
    struct data_packet packet;
    packet.id = packet_id;
    gettimeofday(&packet.timestamp, NULL);

    // printf("send_packet - before\n");
    // fflush(stdout);
    long len = sendto(socketfd, (const struct data_packet *) &packet, sizeof(packet),
            0, (const struct sockaddr *) &server_address, sizeof(server_address));
    if (len == -1) {
        perror("failed to send packet");
    }
    // printf("send_packet - after\n");
    // fflush(stdout);
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        fprintf(stderr, "usage: '%s [SERVER_IP] [PORT] [PACKET_AMOUNT] [DELAY_MULTIPLIER]'\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char* server_ip = argv[1];
    const int port = atoi(argv[2]);
    const int packet_amount = atoi(argv[3]);
    const int delay_multiplier = atoi(argv[4]);

    struct sockaddr_in server_address = {0};
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(server_ip);

    srand(time(NULL));

    printf("Starting sending %d Packets to %s:%d with poisson random delay %dms\n", packet_amount, server_ip, port, POISSON_AVG*delay_multiplier/1000);
    for (int i = 0; i < packet_amount; i++) {
        int delay = poisson_random() * delay_multiplier;
        printf("Sending Packet #%d/%d - next delay is %dusec\r", i+1, packet_amount, delay);
        // fflush(stdout);
        send_packet(socketfd, server_address, i+1); // packet ids start from 1
        // printf("Packet #%d sent\n", i+1);
        usleep(delay);
    }

    send_packet(socketfd, server_address, -1);
    close(socketfd);
    printf("\nDone\n");
    return 0;
}
