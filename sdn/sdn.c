#include <stdio.h>
#include <stdlib.h>
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

int is_invalid_dest(char* dest) {
    u_int8_t ip[4];
    unsigned short port;
    int parse_status = sscanf(dest, "%hhu.%hhu.%hhu.%hhu:%hu", &ip[0], &ip[1], &ip[2], &ip[3], &port);
    return parse_status <= 0;
}

int create_tunnel(struct tunnel *t) {      
    int fd[2]; // fd[1] -> fd[0]
    if (pipe(fd) == -1) {
        printf("Piping failed\n");
        return 1;
    }

    // create sender
    t->pid_sender = fork();
    if (t->pid_sender < 0) {
        printf("Couldn't create sender\n");
        return 1;
    }
    else if (t->pid_sender == 0) {
        close(fd[1]);
        dup2(STDIN_FILENO, fd[0]);
        sprintf(buffer, "udp-sendto:%s", t->dest);
        printf("[%d] Creating Sender: \"%s\"\n", t_index, buffer);
        int status = execl("/usr/bin/socat", "/usr/bin/socat", "-", buffer, NULL);
        printf("[%d] Warning - Sender exit status %d\n", t_index, status);
        exit(1);
    }

    // create listener
    t->pid_listener = fork();
    if (t->pid_listener < 0) {
        printf("Couldn't create listener\n");
        kill(t->pid_sender, SIGINT);
        return 1;
    }
    else if (t->pid_listener == 0) {
        close(fd[0]);
        dup2(STDOUT_FILENO, fd[1]);
        sprintf(buffer, "udp4-listen:%s", t->localport);
        printf("[%d] Creating Listener: \"%s\"\n", t_index, buffer);
        int status = execl("/usr/bin/socat", "/usr/bin/socat", "-", buffer, NULL);
        printf("[%d] Warning - Listener exit status %d\n", t_index, status);
        exit(1);
    }

    close(fd[0]);
    close(fd[1]);
    return 0;
}

int remove_tunnel(struct tunnel *t) {
    int status_listener, status_sender;
    if (t->pid_listener != 0){
        status_listener = kill(t->pid_listener, SIGINT);
        if (status_listener) {
            printf("Error removing Listener PID %d\n", t->pid_listener);
        }
    }
    if (t->pid_sender != 0) {
        status_sender = kill(t->pid_sender, SIGINT);
        if (status_sender) {
            printf("Error removing Sender PID %d\n", t->pid_sender);
        }
    }
    return (status_listener || status_sender) ? 1 : 0;
}

int main() {
    printf("SDN Started\n");
    while(1) {
        usleep(200000);
        memset(token, 0, sizeof(token));
        printf("> ");
        fgets(buffer, BUFFER_SIZE, stdin);
        sscanf(buffer, "%s", token);
        if (!strcmp(token, "help")) {
            printf("Listing all commands:\n"
                "- help - this\n"
                "- list - lists all tunnels\n"
                "- add [LOCALPORT] [DEST_IP]:[DEST_PORT] - adds a tunnel, returns TUNNEL_ID\n"
                "- update [TUNNEL_ID] [DEST_IP]:[DEST_PORT] - updates a tunnel, returns TUNNEL_ID\n"
                "- remove [TUNNEL_ID] - removes a tunnel\n"
                "- exit - exit\n");
        }
        else if (!strcmp(token, "list")) {
            printf("Listing all tunnels:\n");
            for (int i = 0; i < TUNNELS_SIZE; i++) {
                if (tunnels[i].pid_sender) {
                    printf("[%d] %s -> %s\n", i, tunnels[i].localport, tunnels[i].dest);
                }
            }
        }
        else if (!strcmp(token, "")) continue;
        else if (!strcmp(token, "add")) {
            if (tunnel_ctr == TUNNELS_SIZE) {
                printf("Maximum tunnel amount reacehd, please delete one\n");
                continue;
            }
            while (tunnels[t_index].pid_sender) t_index = (t_index+1) % TUNNELS_SIZE;
            int parse_status = sscanf(buffer, "%*s %s %s",
                tunnels[t_index].localport, tunnels[t_index].dest);
            if (parse_status < 0 || is_invalid_dest(tunnels[t_index].dest)) {
                printf("Couldn't parse %s\n", buffer);
                continue;
            }

            if (create_tunnel(&tunnels[t_index])) {
                printf("Couldn't create %s\n", buffer);
                continue;
            }

            tunnel_ctr++;
            printf("[%d] added, PID %d -> %d\n", t_index, tunnels[t_index].pid_listener, tunnels[t_index].pid_sender);
        }
        else if (!strcmp(token, "update")) {
            int parse_status = sscanf(buffer, "%*s %d %s",&t_index, token);
            strcpy(tunnels[t_index].dest, token);
            if (parse_status < 0 || is_invalid_dest(tunnels[t_index].dest)) {
                printf("Couldn't parse %s", buffer);
                continue;
            }

            remove_tunnel(&tunnels[t_index]);

            if (create_tunnel(&tunnels[t_index])) {
                printf("Couldn't create %s\n", buffer);
                continue;
            }

            printf("[%d] updated, PID %d -> %d\n", t_index, tunnels[t_index].pid_listener, tunnels[t_index].pid_sender);
        }
        else if (!strcmp(token, "remove")) {
            sscanf(buffer, "%*s %d",&t_index);

            remove_tunnel(&tunnels[t_index]);

            memset(&tunnels[t_index], 0, sizeof(struct tunnel));
            tunnel_ctr--;
            printf("[%d] removed\n", t_index);
        }
        else if (!strcmp(token, "exit")) {
            printf("Removing tunnels..");
            for (int i = 0; i < TOKEN_SIZE; i++) {
                remove_tunnel(&tunnels[i]);
            }
            break;
        }
        else {
            printf("invalid token %s, type help\n", token);
        }
    }
    printf(" done\n");
    return 0;
}