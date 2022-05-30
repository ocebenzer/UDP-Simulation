#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_SIZE 64
#define MAX_RRU 16

struct route {
    short status; // 0 is empty, 1 is full
    char localport[8];
    char ip[16];
    char port[8];
};


void main() {
    char line[LINE_SIZE];
    char token[LINE_SIZE];
    int index = 0;

    struct route routes[MAX_RRU];
    memset(routes, 0, sizeof(routes));

    printf("SDN Ready\n");
    while(1){
        fgets(line, LINE_SIZE, stdin);
        if (line[strlen(line)-1] == '\n') line[strlen(line)-1] = '\0';
        sscanf(line, "%s", token);
        if (!strcmp(token, "exit")) break;
        else if (!strcmp(line, "help")) {
            printf(
                "--SDN deamon--\n"
                "help\n"
                "list\n"
                "add [LOCALPORT] [IP]:[PORT]\n"
                "remove [ROUTE_ID]\n"
                "exit\n");
        }
        else if (!strcmp(token, "list")) {
            printf("Listing current routes:\n");
            for (int i = 0; i < MAX_RRU; i++) {
                if (routes[i].status == 0) continue;
                printf("[%d] port %s to %s:%s\n", i, routes[i].localport, routes[i].ip, routes[i].port);
            }
        }
        else if (!strcmp(token, "add")) {
            while (routes[index].status) index++; // find empty index
            sscanf(line, "%*s %s %[^:]:%s", routes[index].localport, routes[index].ip, routes[index].port);
            routes[index].status = 1;
            // run command
            printf("ROUTE_ID [%d]: forwarding port %s to %s:%s\n",
                index,
                routes[index].localport,
                routes[index].ip,
                routes[index].port);
        }
        else if (!strcmp(token, "remove")) {
            int delete_index;
            scanf("%d", &delete_index);
            // run command
            printf("stopped forwarding port %s to %s:%s\n",
                routes[delete_index].localport,
                routes[delete_index].ip,
                routes[delete_index].port);
            memset(&routes[delete_index], 0, sizeof(struct route));
        }
        else {
            printf("invalid buffer: %s\n", line);
        }
    }
    printf("Shutting Down\n");
    exit(0);
}