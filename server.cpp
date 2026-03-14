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

// assume that the math operation is valid, and the end result can be large
double calculateMath(char* buffer){ // input string here must be null-terminated, else need to have another input as the string length
    double num1 = 0, num2 = 0;
    char op = 0;
    int i = 0;
    if (buffer[i] == '-') i++;
    while (buffer[i] && !(buffer[i] == '+' || buffer[i] == '-' || buffer[i] == '*' || buffer[i] == '/')) {
        i++;
    }

    if (buffer[i]) {
        op = buffer[i];
        buffer[i] = '\0'; 
        num1 = atof(buffer);
        num2 = atof(buffer + i + 1);
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
    1. Open [name]
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

        // blocking operation
        ssize_t bytesRead = recvfrom(socketfd, (char *) buffer, MAXBUF, MSG_WAITALL, (sockaddr*) &clientaddr, &len);
        buffer[bytesRead] = '\0';

        int id = -1;

        for (int i = 0; i < (int)clients.size(); i++){
            // if the address is in the vector and is valid (has not been closed)
            client c = clients[i];
            if (c.addr.sin_family == clientaddr.sin_family && c.addr.sin_port == clientaddr.sin_port && c.addr.sin_addr.s_addr == clientaddr.sin_addr.s_addr && c.valid){
                id = i;
                break;
            }
        }


        if (id == -1){
            if (strncmp(buffer, "Open", 4) == 0) {
                // create new entry for a new client
                chrono::system_clock::time_point now = chrono::system_clock::now();
                time_t now_c = chrono::system_clock::to_time_t(now);

                clients.emplace_back(buffer + 5, now_c, clientaddr);

                // send acknowledgement and log out

                const char*successMessage = "Server has set up a connection!\n";
                sendto(socketfd, successMessage, strlen(successMessage), MSG_CONFIRM, (sockaddr *) &clientaddr, len);

                char* timeString = std::ctime(&now_c); 

                cout << "Client " << clients.back().name << " opened a connection at " << timeString;
            } else {
                const char*invalidMessage = "Client has not set up connection with server!\n";
                sendto(socketfd, invalidMessage, strlen(invalidMessage), MSG_CONFIRM, (sockaddr *) &clientaddr, len);
            }
            continue;
        }

        // log out detail of the request
        cout << "Client " << clients[id].name << " sent a request: " << buffer << "\n";

        if (strncmp(buffer, "Open", 4) == 0){
            // if the client has already opened a connection then send an error message
            const char*invalidMessage = "Already opened a connection!\n";
            sendto(socketfd, invalidMessage, strlen(invalidMessage), MSG_CONFIRM, (sockaddr *) &clientaddr, len);

        } else if (strcmp(buffer, "Close") == 0) {
            clients[id].valid = false;
            const char*closeMessage = "Server has closed the connection!\n";
            sendto(socketfd, closeMessage, strlen(closeMessage), MSG_CONFIRM, (sockaddr *) &clientaddr, len);

        } else if (isdigit(buffer[0]) || buffer[0] == '-'){
            double result = calculateMath(buffer);
            stringstream ss;
            ss << "Result is " << result << "\n";
            string message = ss.str();

            sendto(socketfd, (char *)message.c_str(), message.length(), MSG_CONFIRM, (sockaddr *) &clientaddr, len);
        } else {
            const char*invalidMessage = "Request is invalid, try again!\n";
            sendto(socketfd, invalidMessage, strlen(invalidMessage), MSG_CONFIRM, (sockaddr *) &clientaddr, len);
        }
        
    }


}
