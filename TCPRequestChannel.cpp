#include "TCPRequestChannel.h"
#include <string.h>  // for bzero
#include <unistd.h>  // for close

using namespace std;


TCPRequestChannel::TCPRequestChannel (const std::string _ip_address, const std::string _port_no) {
    struct sockaddr_in server_address;
    // for safety 
    bzero((char*)&server_address, sizeof(struct sockaddr_in));
    // make the address
    server_address.sin_addr.s_addr = !_ip_address.length() ? INADDR_ANY: inet_addr(_ip_address.c_str());
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(stoi(_port_no));
    // make the socket 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(_ip_address.length() == 0){
        // server socket
        if(bind(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1){
            perror("An error occured while binding server address");
            exit(1);
        }
        // start listening for incoming requests
        listen(sockfd, SOMAXCONN);
    }else{
        // client socket
        if(connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1){
            perror("An error occured while connecting...");
            exit(1);
        }

    }


}

TCPRequestChannel::TCPRequestChannel (int _sockfd) {
    // Request Chan Established
    sockfd = _sockfd;
}

TCPRequestChannel::~TCPRequestChannel () {
    close(sockfd);
    
}

int TCPRequestChannel::accept_conn () {
    // not needed but good for debugging
    struct sockaddr_in client_address;
    // for safety 
    bzero((char*)&client_address, sizeof(client_address));
    // make the address
    //socklen_t client_address_len = sizeof(client_address);
    int cSockFd = accept(sockfd, NULL, NULL);
    if (cSockFd == -1) {
        perror("ERROR on accept");
        exit(1);
    }
    return cSockFd;
}

int TCPRequestChannel::cread (void* msgbuf, int msgsize, bool fileTransfer) {
    int bytes = 0;
    if(fileTransfer){
        bytes = recv(sockfd, msgbuf, msgsize, MSG_WAITALL);
    }else{
        bytes = recv(sockfd, msgbuf, msgsize, 0);
    }
    if(bytes < 0){
        perror("something bad happend during the reading");
        exit(1);
    }
    return bytes;
}

int TCPRequestChannel::cwrite (void* msgbuf, int msgsize) {
    int bytes = send(sockfd, msgbuf, msgsize, 0);  
    if(bytes < 0){                                                                              
        perror("something bad happend during the writing");                                     
        exit(1);
    }                                                                                                                                                                                                                                                                                    
    return bytes;
}
