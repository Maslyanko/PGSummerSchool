#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <arpa/inet.h>

int main() {
    struct sockaddr_in proxySocket;
    proxySocket.sin_family = AF_INET;
    proxySocket.sin_port = htons(8080);
    proxySocket.sin_addr.s_addr = inet_addr("127.0.0.1");

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    connect(sockfd, (struct sockaddr *)&proxySocket, sizeof(proxySocket));
    
    char buffer[1024];
    while (1) {
        ssize_t length;
        if ((length = read(0, buffer, 1024)) == -1) {
            perror("Error reading from socket");
            exit(EXIT_FAILURE);
        }

        buffer[length] = '\0';
        
        write(sockfd, buffer, length);

        if ((length = read(sockfd, buffer, length)) == -1) {
            perror("Error reading from proxy");
            exit(EXIT_FAILURE);
        }

        buffer[length] = '\0';

        write(1, buffer, length);
        write(1, "\n", 1);
    }
}