#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>

using namespace std;

#define PORT 6767
#define MAXBUF 1024

int main(int argc, char* argv[]) {
    int sockfd;
    char buffer[MAXBUF];
    string openn = "Open ";
    
    openn += argv[1];
    
    const char* openMsg = openn.c_str();
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
    servaddr.sin_addr.s_addr = inet_addr("***********"); // Server IP

    socklen_t len = sizeof(servaddr);

    // Send message to server
    sendto(sockfd, openMsg, strlen(openMsg), MSG_CONFIRM,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));

    // Receive reply from server
    int n = recvfrom(sockfd, buffer, MAXBUF, MSG_WAITALL,
                     (struct sockaddr *)&servaddr, &len);

    buffer[n] = '\0';   // Null terminate received data
    printf("%s\n", buffer);
        
    for (int i = 0; i < 10; i++){
        const char* math = "2*4";
        // Send message to server
        sendto(sockfd, math, strlen(math), MSG_CONFIRM,
            (const struct sockaddr *)&servaddr, sizeof(servaddr));

        // Receive reply from server
        memset(buffer, 0, MAXBUF);
        n = recvfrom(sockfd, buffer, MAXBUF, MSG_WAITALL,
                        (struct sockaddr *)&servaddr, &len);

        buffer[n] = '\0';   // Null terminate received data
        printf("%s\n", buffer);
        sleep(1);
    }

    const char* cl = "Close";
    // Send message to server
    sendto(sockfd, cl, strlen(cl), MSG_CONFIRM,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));

    // Receive reply from server
    memset(buffer, 0, MAXBUF);
    n = recvfrom(sockfd, buffer, MAXBUF, MSG_WAITALL,
                     (struct sockaddr *)&servaddr, &len);

    buffer[n] = '\0';   // Null terminate received data
    printf("%s\n", buffer);
    

    // Close socket
    close(sockfd);

    return 0;
}