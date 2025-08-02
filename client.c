#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define PORT 1234

int main() {
    char buffer[1024];
    char text[1024];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket failed");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("Connection failed");
        return -1;
    }

    printf("Connected to server.\n");

    // پروسس فرزند برای خواندن پیام سرور
    if (fork() == 0) {
        while (1) {
            memset(buffer, 0, sizeof(buffer));
            int recv_len = recv(sock, buffer, sizeof(buffer), 0);
            if (recv_len <= 0) {
                printf("Server disconnected.\n");
                exit(0);
            }
            printf("\nServer: %s\n", buffer);
            if (strcmp(buffer, "exit") == 0) {
                printf("Server closed the chat.\n");
                exit(0);
            }
        }
    }

    // پروسس والد برای ارسال پیام کاربر
    while (1) {
        printf("You: ");
        fflush(stdout);
        if (fgets(text, sizeof(text), stdin) == NULL)
            continue;
        text[strcspn(text, "\n")] = 0; // حذف newline
        send(sock, text, strlen(text), 0);
        if (strcmp(text, "exit") == 0) {
            printf("Exiting...\n");
            shutdown(sock, 2);
            exit(0);
        }
    }

    shutdown(sock, 2);
    return 0;
}

