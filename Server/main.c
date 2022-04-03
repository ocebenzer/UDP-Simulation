#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>

struct data_packet {
    long id;
    struct timeval time;
};

void recv_packet(int socketfd){
    socklen_t len = 0;
    struct data_packet packet = {0};
    recvfrom(socketfd, (struct data_packet *) &packet, sizeof(packet),
                        MSG_WAITALL, 0, &len);
    printf("packet #%ld: %ld.%06ld\n", packet.id, packet.time.tv_sec, packet.time.tv_usec);

}

int main(int argc, char* argv[]) {
    printf("Server starting");
    int packet_amount;
    if (argc < 2) {
        fprintf(stderr, "usage: '%s [PORT]'\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if (argc == 3) packet_amount = atoi(argv[2]);
    int port = atoi(argv[1]);

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

    printf("Server ready");
    for (int i = 0; argc < 3 || i < packet_amount; i++) {
        recv_packet(socketfd);
    }
}
