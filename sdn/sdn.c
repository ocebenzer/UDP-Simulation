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
    pid_t pid;
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
                if (tunnels[i].pid) {
                    printf("[%d] %s -> %s\n", i, tunnels[i].localport, tunnels[i].dest);
                }
            }
        }
        else if (!strcmp(token, "add")) {
            if (tunnel_ctr == TUNNELS_SIZE) {
                printf("Maximum tunnel amount reacehd, please delete one\n");
                continue;
            }
            while (tunnels[t_index].pid) t_index = (t_index+1) % TUNNELS_SIZE;
            int parse_status = sscanf(buffer, "%*s %s %s",
                tunnels[t_index].localport, tunnels[t_index].dest);
            if (parse_status < 0 || is_invalid_dest(tunnels[t_index].dest)) {
                printf("Couldn't parse %s", buffer);
                continue;
            }
            tunnels[t_index].pid = fork();
            if (tunnels[t_index].pid < 0) {
                printf("Couldn't create tunnel\n");
                continue;
            }
            else if (tunnels[t_index].pid == 0) {
                sprintf(buffer, "socat - udp4-listen:%s | socat - udp-sendto:%s", tunnels[t_index].localport, tunnels[t_index].dest);
                printf("Tunnel command: \"%s\"\n", buffer);
                int status = system(buffer);
                printf("Warning - Tunnel [%d] might not be added\n", t_index);
                exit(1);
            }
            else {
                tunnel_ctr++;
                printf("[%d] added\n", t_index);
            }
        }
        else if (!strcmp(token, "update")) {
            int parse_status = sscanf(buffer, "%*s %d %s",&t_index, token);
            strcpy(tunnels[t_index].dest, token);
            if (parse_status < 0 || is_invalid_dest(tunnels[t_index].dest)) {
                printf("Couldn't parse %s", buffer);
                continue;
            }
            if (kill(tunnels[t_index].pid, SIGINT)) {
                printf("Error removing old Tunnel [%d]\n", tunnels[t_index].pid);
                continue;
            }
            tunnels[t_index].pid = fork();
            if (tunnels[t_index].pid < 0) {
                printf("Couldn't create tunnel\n");
                continue;
            }
            else if (tunnels[t_index].pid == 0) {
                sprintf(buffer, "socat - udp4-listen:%s | socat - udp-sendto:%s", tunnels[t_index].localport, tunnels[t_index].dest);
                printf("Tunnel command: \"%s\"\n", buffer);
                int status = system(buffer);
                printf("Warning - Tunnel [%d] might not be added", t_index);
                exit(1);
            }
            else {
                printf("[%d] updated\n", t_index);
            }
        }
        else if (!strcmp(token, "remove")) {
            sscanf(buffer, "%*s %d",&t_index);
            int pid = tunnels[t_index].pid;
            memset(&tunnels[t_index], 0, sizeof(struct tunnel));
            if (kill(pid, SIGINT)) {
                printf("Error removing Tunnel [%d]\n", pid);
                continue;
            }
            tunnel_ctr--;
            printf("[%d] removed\n", t_index);
        }
        else if (!strcmp(token, "exit")) {
            for (int i = 0; i < TOKEN_SIZE; i++) {
                if (tunnels[i].pid) {
                    kill(tunnels[i].pid, SIGINT);
                }
            }
            printf("Removing tunnels..");
            break;
        }
        else {
            printf("invalid token %s, type help\n", token);
        }
    }
    printf(" done\n");
    return 0;
}