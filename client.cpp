#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>


#define PORT 6767
#define MAXBUF 1024

int main() {
    int sockfd;
    char buffer[MAXBUF];
    const char *hello = "Open";
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Fill server address info
    servaddr.sin_family = AF_INET;              // IPv4
    servaddr.sin_port   = htons(PORT);          // Server port
    servaddr.sin_addr.s_addr = inet_addr("11.29.6.82"); // Server IP

    socklen_t len = sizeof(servaddr);

    // Send message to server
    sendto(sockfd, hello, strlen(hello), MSG_CONFIRM,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Hello message sent.\n");

    // Receive reply from server
    int n = recvfrom(sockfd, buffer, MAXBUF, MSG_WAITALL,
                     (struct sockaddr *)&servaddr, &len);

    buffer[n] = '\0';   // Null terminate received data
    printf("Server: %s\n", buffer);

    // Close socket
    close(sockfd);

    return 0;
}