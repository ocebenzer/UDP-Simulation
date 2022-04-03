#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>

struct data_packet {
    long id;
    struct timeval time;
};

int main(int argc, char* argv[]) {
    int packet_amount;
    if (argc < 2) {
        fprintf(stderr, "usage: '%s [PORT]'\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (argc == 3) packet_amount = atoi(argv[2]);
    int port = atoi(argv[1]);

    struct sockaddr_in server_address = {0};
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int rc = bind(fd, (const struct sockaddr *) &server_address, sizeof(server_address));
    if (rc == -1) {
        perror("failed to bind socket");
        close(fd);
        exit(EXIT_FAILURE);
    }

    struct data_packet packet = {0};
    socklen_t len = 0;

    for (int i = 0; argc < 3 || i < packet_amount; i++) {
        recvfrom(fd, (struct data_packet *) &packet, 50,
                         MSG_WAITALL, 0, &len);
        printf("packet #%ld: %ld.%06ld\n", packet.id, packet.time.tv_sec, packet.time.tv_usec);
    }
}
