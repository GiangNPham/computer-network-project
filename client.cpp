#include <algorithm>
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <regex>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

#define MAXBUF 1024
#define PORT 6767

int main(int argc, char *argv[]) {
  int sockfd;
  char buffer[MAXBUF];
  string openn = "Open ";

  openn += argv[1];

  const char *openMsg = openn.c_str();
  // create a socket to communcate with the server
  struct sockaddr_in servaddr;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));

  // Fill server address info
  servaddr.sin_family = AF_INET;                     // IPv4
  servaddr.sin_port = htons(PORT);                   // Server port
  servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP

  socklen_t len = sizeof(servaddr);

  // Send message to server
  sendto(sockfd, openMsg, strlen(openMsg), MSG_CONFIRM,
         (const struct sockaddr *)&servaddr, sizeof(servaddr));

  // Receive reply from server
  int n = recvfrom(sockfd, buffer, MAXBUF, MSG_WAITALL,
                   (struct sockaddr *)&servaddr, &len);

  buffer[n] = '\0'; // Null terminate received data
  printf("%s\n", buffer);

  regex expr_regex("^-?[0-9]+([.][0-9]+)?[-+*/]-?[0-9]+([.][0-9]+)?$");
  string input;

  while (true) {
    cout << "Enter math expression (or 'quit' to exit): ";
    if (!getline(cin, input))
      break;

    // Clean input: remove spaces
    input.erase(remove(input.begin(), input.end(), ' '), input.end());

    if (input == "quit") {
      break;
    }

    // Verify format
    if (!regex_match(input, expr_regex)) {
      cout << "Invalid format. Please use a+b, a-b, a*b, or a/b (e.g., 2+3)."
           << endl;
      continue;
    }

    // Send message to server
    sendto(sockfd, input.c_str(), input.length(), MSG_CONFIRM,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));

    // Receive reply from server
    memset(buffer, 0, MAXBUF);
    n = recvfrom(sockfd, buffer, MAXBUF, MSG_WAITALL,
                 (struct sockaddr *)&servaddr, &len);

    buffer[n] = '\0'; // Null terminate received data
    cout << buffer;
  }

  const char *cl = "Close";
  // Send message to server
  sendto(sockfd, cl, strlen(cl), MSG_CONFIRM,
         (const struct sockaddr *)&servaddr, sizeof(servaddr));

  // Receive reply from server
  memset(buffer, 0, MAXBUF);
  n = recvfrom(sockfd, buffer, MAXBUF, MSG_WAITALL,
               (struct sockaddr *)&servaddr, &len);

  buffer[n] = '\0'; // Null terminate received data
  printf("%s\n", buffer);

  // Close socket
  close(sockfd);

  return 0;
}