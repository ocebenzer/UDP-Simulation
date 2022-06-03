#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define TUNNELS_SIZE 64
#define BUFFER_SIZE 256
#define TOKEN_SIZE 32

struct tunnel{
    char localport[8];
    char dest[TOKEN_SIZE];
    pid_t pid_listener, pid_sender;
};

struct tunnel tunnels[TUNNELS_SIZE];
char buffer[BUFFER_SIZE], token[TOKEN_SIZE];
int tunnel_ctr, t_index;

bool is_invalid_dest(char* dest) {
    u_int8_t ip[4];
    unsigned short port;
    int parse_status = sscanf(dest, "%hhu.%hhu.%hhu.%hhu:%hu", &ip[0], &ip[1], &ip[2], &ip[3], &port);
    return parse_status <= 0;
}

int main() {
    printf("SDN Started\n");
    while(true) {
        memset(token, 0, sizeof(token));
        fgets(buffer, BUFFER_SIZE, stdin);
        sscanf(buffer, "%s", token);
        if (!strcmp(token, "help")) {
            printf("Listing all commands:\n"
                "help - this\n"
                "list - lists all tunnels\n"
                "add [LOCALPORT] [DEST_IP]:[DEST_PORT] - adds a tunnel, returns TUNNEL_ID\n"
                "update [TUNNEL_ID] [DEST_IP]:[DEST_PORT] - updates a tunnel, returns TUNNEL_ID\n"
                "remove [TUNNEL_ID] - removes a tunnel\n"
                "exit - exit\n");
        }
        else if (!strcmp(token, "list")) {
            printf("Listing all tunnels:\n");
            for (int i = 0; i < TUNNELS_SIZE; i++) {
                if (tunnels[i].pid_sender) {
                    printf("[%d] %s -> %s\n", i, tunnels[i].localport, tunnels[i].dest);
                }
            }
        }
        else if (!strcmp(token, "add")) {
            if (tunnel_ctr == TUNNELS_SIZE) {
                printf("Maximum tunnel amount reacehd, please delete one\n");
                continue;
            }
            while (tunnels[t_index].pid_sender) t_index = (t_index+1) % TUNNELS_SIZE;
            int parse_status = sscanf(buffer, "%*s %s %s",
                tunnels[t_index].localport, tunnels[t_index].dest);
            if (parse_status < 0 || is_invalid_dest(tunnels[t_index].dest)) {
                printf("Couldn't parse %s", buffer);
                continue;
            }
            tunnels[t_index].pid_sender = fork();
            if (tunnels[t_index].pid_sender < 0) {
                printf("Couldn't create tunnel\n");
                continue;
            }
            else if (tunnels[t_index].pid_sender == 0) {
                sprintf(buffer, "/usr/bin/socat - udp4-listen:%s | /usr/bin/socat - udp-sendto:%s", tunnels[t_index].localport, tunnels[t_index].dest);
                printf("Tunnel command: \"%s\"\n", buffer);
                int status = execl("/bin/sh", "/bin/sh", "-c", buffer, NULL);
                printf("Warning - Tunnel [%d] might not be added - execl status %d\n", t_index, status);
                exit(1);
            }
            else {
                tunnel_ctr++;
                printf("[%d] added, PID %d -> %d\n", t_index, tunnels[t_index].pid_listener, tunnels[t_index].pid_sender);
            }
        }
        else if (!strcmp(token, "update")) {
            int parse_status = sscanf(buffer, "%*s %d %s",&t_index, token);
            strcpy(tunnels[t_index].dest, token);
            if (parse_status < 0 || is_invalid_dest(tunnels[t_index].dest)) {
                printf("Couldn't parse %s", buffer);
                continue;
            }
            if (kill(tunnels[t_index].pid_listener, SIGINT)) {
                printf("Error removing old Tunnel [%d]\n", tunnels[t_index].pid_listener);
                continue;
            }
            if (kill(tunnels[t_index].pid_sender, SIGINT)) {
                printf("Error removing old Tunnel [%d]\n", tunnels[t_index].pid_sender);
                continue;
            }
            tunnels[t_index].pid_sender = fork();
            if (tunnels[t_index].pid_sender < 0) {
                printf("Couldn't create tunnel\n");
                continue;
            }
            else if (tunnels[t_index].pid_sender == 0) {
                sprintf(buffer, "/usr/bin/socat - udp4-listen:%s | /usr/bin/socat - udp-sendto:%s", tunnels[t_index].localport, tunnels[t_index].dest);
                printf("Tunnel command: \"%s\"\n", buffer);
                int status = execl("/bin/sh", "/bin/sh", "-c", buffer, NULL);
                printf("Warning - Tunnel [%d] might not be added - execl status %d\n", t_index, status);
                exit(1);
            }
            else {
                printf("[%d] updated, PID %d -> %d\n", t_index, tunnels[t_index].pid_listener, tunnels[t_index].pid_sender);
            }
        }
        else if (!strcmp(token, "remove")) {
            sscanf(buffer, "%*s %d",&t_index);
            int pid_listener = tunnels[t_index].pid_listener;
            int pid_sender = tunnels[t_index].pid_sender;
            memset(&tunnels[t_index], 0, sizeof(struct tunnel));
            if (pid_sender == 0 || pid_listener) {
                printf("Error, %d is not a valid tunnel\n", t_index);
                continue;
            }
            if (kill(pid_listener, SIGINT)) {
                printf("Error removing Tunnel [%d]\n", pid_listener);
                continue;
            }
            if (kill(pid_sender, SIGINT)) {
                printf("Error removing Tunnel [%d]\n", pid_sender);
                continue;
            }
            tunnel_ctr--;
            printf("[%d] removed\n", t_index);
        }
        else if (!strcmp(token, "exit")) {
            printf("Removing tunnels..");
            for (int i = 0; i < TOKEN_SIZE; i++) {
                if (tunnels[i].pid_sender) {
                    kill(tunnels[i].pid_listener, SIGINT);
                    kill(tunnels[i].pid_sender, SIGINT);
                }
            }
            break;
        }
        else if (!strcmp(token, "")) continue;
        else {
            printf("invalid token %s, type help\n", token);
        }
    }
    printf(" done\n");
    return 0;
}