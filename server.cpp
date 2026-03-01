#include <iostream>
#include <sys/socket.h> 
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <cstring>
#include <vector>
#include <sstream>
#include <chrono>

using namespace std;

#define MAXBUF 1024
#define PORT 6767

struct client {
    string name;
    time_t connectedTime;
    sockaddr_in addr;
    bool valid;
    // how long the user is attached to the server???
    
    client(string n, time_t t, sockaddr_in a) : name(n), connectedTime(t), addr(a), valid(true) {}
};

// assume that the math operation is valid, between 2 integers, and the end result is within 32 bits
int calculateMath(char* buffer){ // input string here must be null-terminated, else need to have another input as the string length
    int num1 = 0, num2 = 0;
    char op = 0;
    int i = 0;
    while (buffer[i] && !(buffer[i] == '+' || buffer[i] == '-' || buffer[i] == '*' || buffer[i] == '/')) {
        i++;
    }

    if (buffer[i]) {
        op = buffer[i];
        buffer[i] = '\0'; 
        num1 = atoi(buffer);
        num2 = atoi(buffer + i + 1);
        buffer[i] = op; 
    } else {
        // Invalid format
        return 0;
    }
    switch (op) {
        case '+': return num1 + num2;
        case '-': return num1 - num2;
        case '*': return num1 * num2;
        case '/': return num2 != 0 ? num1 / num2 : 0;
        default: return 0;
    }
}

// the bind function binds a file descriptor to the SERVER address 
// when the packet is received, the system can get the server address 
// then see which file descriptor that it corresponds to 
// the server will read normally from a file descriptor
int main(){
    int socketfd;
    char buffer[MAXBUF];

    sockaddr_in serveraddr;

    // create the socket fd
    if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("creating socket failed\n");
        exit(1);
    }


    memset(&serveraddr, 0, sizeof(serveraddr)); 
      
    serveraddr.sin_family    = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY; 
    serveraddr.sin_port = htons(PORT);

    if (bind(socketfd, (const sockaddr*) &serveraddr, sizeof(serveraddr)) < 0){
        perror("bind failed\n");
        exit(1);
    }

    // use a queue to answer the request in FIFO order
    // 3 types of request: open connection, math operation, close connection

    /*
    Request format:
    1. Open
    2. Math operation
    3. Close
    */

    // use a vector to keep track of all the users
    // create a struct

    sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    vector<client> clients;

    while (1){
        memset(&clientaddr, 0, sizeof(clientaddr)); 

        ssize_t bytesRead = recvfrom(socketfd, (char *) buffer, MAXBUF, MSG_WAITALL, (sockaddr*) &clientaddr, &len);
        buffer[bytesRead] = '\0';
        cout << "here\n";

        int id = -1;

        for (int i = 0; i < clients.size(); i++){
            // if the address is in the vector and is valid (has not been closed)
            client c = clients[i];
            if (c.addr.sin_family == clientaddr.sin_family && c.addr.sin_port == clientaddr.sin_port && c.addr.sin_addr.s_addr == clientaddr.sin_addr.s_addr && c.valid){
                id = i;
                break;
            }
        }

        if (id == -1){
            if (buffer[0] == 'O'){
                // create new entry for a new client
                chrono::system_clock::time_point now = chrono::system_clock::now();
                time_t now_c = chrono::system_clock::to_time_t(now);
                clients.emplace_back(buffer, now_c, clientaddr);

                // send acknowledgement and log out

                string successMessage = "Server has set up a connection!\n";
                sendto(socketfd, (char *)successMessage.c_str(), successMessage.length(), MSG_CONFIRM, (sockaddr *) &clientaddr, len);

                cout << "Client " << buffer << " opened a connection at " << now_c << "\n";
            } else {
                string invalidMessage = "Client has not set up connection with server!\n";
                sendto(socketfd, (char *)invalidMessage.c_str(), invalidMessage.length(), MSG_CONFIRM, (sockaddr *) &clientaddr, len);
            }
            continue;
        }

        if (buffer[0] == 'O'){
            // if the client has already opened a connection then send an error message
            string invalidMessage = "Already opened a connection!\n";
            sendto(socketfd, (char *)invalidMessage.c_str(), invalidMessage.length(), MSG_CONFIRM, (sockaddr *) &clientaddr, len);

        } else if (buffer[0] == 'C') {
            clients[id].valid = false;

        } else if (isdigit(buffer[0])){
            int result = calculateMath(buffer);
            stringstream ss;
            ss << "Result is " << result << "\n";
            string message = ss.str();

            sendto(socketfd, (char *)message.c_str(), message.length(), MSG_CONFIRM, (sockaddr *) &clientaddr, len);
        } else {
            string invalidMessage = "Request is invalid, try again!\n";
            sendto(socketfd, (char *)invalidMessage.c_str(), invalidMessage.length(), MSG_CONFIRM, (sockaddr *) &clientaddr, len);
        }
        
    }


}
