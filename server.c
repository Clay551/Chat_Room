#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>   // برای fork و shutdown

#define PORT 1234

int main() {
    char buffer[1024];
    char server_msg[1024];
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1) {
        perror("Socket failed");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Bind failed");
        return -1;
    }

    printf("Listening on port %d...\n", PORT);
    if (listen(sock, 1) == -1) {
        perror("Listen failed");
        return -1;
    }

    socklen_t addr_len = sizeof(addr);
    int client = accept(sock, (struct sockaddr*)&addr, &addr_len);
    if (client == -1) {
        perror("Accept failed");
        return -1;
    }
    printf("Client connected.\n");

    // پروسس فرزند برای ارسال پیام از سرور
    if (fork() == 0) {
        while (1) {
            printf("Server: ");
            fflush(stdout);
            if (fgets(server_msg, sizeof(server_msg), stdin) == NULL)
                continue;
            server_msg[strcspn(server_msg, "\n")] = 0; // حذف newline
            send(client, server_msg, strlen(server_msg), 0);
            if (strcmp(server_msg, "exit") == 0) {
                printf("Server exiting...\n");
                shutdown(client, 2);
                shutdown(sock, 2);
                exit(0);
            }
        }
    }

    // پروسس والد برای دریافت پیام‌های کلاینت
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int recv_len = recv(client, buffer, sizeof(buffer), 0);
        if (recv_len <= 0) {
            printf("Client disconnected.\n");
            break;
        }
        printf("\nClient: %s\n", buffer);
        if (strcmp(buffer, "exit") == 0) {
            printf("Client exited.\n");
            break;
        }
    }

    shutdown(client, 2);
    shutdown(sock, 2);
    return 0;
}
 
