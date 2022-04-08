#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>

struct data_packet {
    long id;
    struct timeval time_client;
    struct timeval time_server;
    struct timeval time_relative;
};

int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y)
{
  result->tv_sec = x->tv_sec - y->tv_sec;

  if ((result->tv_usec = x->tv_usec - y->tv_usec) < 0)
  {
    result->tv_usec += 1000000;
    result->tv_sec--; // borrow
  }

  return result->tv_sec < 0;
}

void recv_packet(int socketfd, struct data_packet *packet){
    recv(socketfd, packet, sizeof(*packet), MSG_WAITALL);
    gettimeofday(&packet->time_server, NULL);
    timeval_subtract(&packet->time_relative, &packet->time_server, &packet->time_client);
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
        fprintf(file, "%ld, %ld.%06ld, %ld.%06ld, %ld.%06ld\n",packets[i].id,
                packets[i].time_relative.tv_sec, packets[i].time_relative.tv_usec,
                packets[i].time_server.tv_sec, packets[i].time_server.tv_usec,
                packets[i].time_client.tv_sec, packets[i].time_client.tv_usec);
    }
}
