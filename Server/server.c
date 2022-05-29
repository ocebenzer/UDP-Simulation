#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define PACKET_MAX 500000

struct data_packet {
    int id;
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
    if (argc < 3) {
        fprintf(stderr, "usage: '%s [PORT] [LOGFILE_PATH]'\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    char* filepath = argv[2];
    struct data_packet *packets = malloc(PACKET_MAX*sizeof(struct data_packet));

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

    FILE *file = fopen(filepath, "w");

    printf("Logging at %s\n", filepath);
    printf("Server Listening on %d\n", port);
    fprintf(file, "id, diff, time send, time receive\n");
    fflush(stdout);
    fflush(file);
    int packet_counter = 0;
    struct data_packet *prev_packet, *packet;
    double delay_sum, delay_max=-999, delay_min=999;
    
    // get initial packet
    prev_packet = &packets[packet_counter++];
    packet = &packets[packet_counter];
    recv_packet(socketfd, packet);
    puts("Receiced initial packet");
    
    while (1) {
        // update packet statistics
        delay_sum += packet->time_relative;
        delay_max = MAX(delay_max, packet->time_relative);
        delay_min = MIN(delay_min, packet->time_relative);

        if (packet->id % 50 == 0) printf("Running.. %d\r", packet->id);
        if (prev_packet->id + 1 != packet->id) printf("\r#%d lost\n", prev_packet->id);

        // log packet data
        fprintf(file, "%d, %lf, %ld.%06ld, %ld.%06ld\n",packet->id, packet->time_relative,
                packet->time_server.tv_sec, packet->time_server.tv_usec,
                packet->time_client.tv_sec, packet->time_client.tv_usec);
        
        // get next packet
        prev_packet = &packets[packet_counter++];
        packet = &packets[packet_counter];
        recv_packet(socketfd, packet);
        // if (packet->id == -1) break;
    }
    packet_counter--; // remove last packet from results

    // double packet_mean = delay_sum / packet_counter;
    // double packet_variance = 0;
    // for (int i = 0; i < packet_counter ; i++) {
    //     packet_variance += pow(packet_mean - packets[i].time_relative, 2);
    // }

    // printf("\nResults:\n");
    // printf("Last packet ID: %d\n", prev_packet->id);
    // printf("Packets received: %d\n", packet_counter);
    // printf("Packets lost: %d\n", prev_packet->id - packet_counter);
    // printf("Delay max: %lf\n", delay_max);
    // printf("Delay min: %lf\n", delay_min);
    // printf("Delay mean: %lf\n", packet_mean);
    // printf("Delay variance: %lf\n", packet_variance / (packet_counter - 1));

    return 0;
}
