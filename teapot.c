#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

const char *httpResponse[] = {
    "HTTP/1.1 418 I'm a teapot\n",
    "Date: Unknown I'm a teapot\n",
    "Server: Toy teapot server\n",
    "Content-length: 0\n",
    "Content-type: text/html\n",
    "Connection: Closed\n",
    "\n",
    "\0"
};

/**
 * error: print an error message to stderr and exit the program
 * @param   msg the message to display to the user
 * @return  to the operating system
 */
void *error(const char *msg) {
    perror(msg);
    exit(1);
}

/**
 * main program execution for our teapot server
 */
int main(int argc, char *argv[]) {
    int socketFileDescriptor, newSocketFileDescriptor, portNumber, clientAddrSz, charsTransferred;
    char transferBuffer[256];
    struct sockaddr_in serverAddress, clientAddress;

    /* make sure the user passed in a port number */
    if (argc < 2) {
        fprintf(stderr, "You must provide a port number to start the server\n");
        exit(1);
    }

    fprintf(stdout,"Creating server socket\n");
    socketFileDescriptor = socket(AF_INET,SOCK_STREAM,0);
    if (socketFileDescriptor < 0) {
        error("The server socket could not be created");
    }

    // zero out a few data structures
    bzero((char *) &serverAddress,sizeof(serverAddress));
    portNumber = atoi(argv[1]);

    fprintf(stdout,"Configuring server address\n");
    // configure the server address
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    fprintf(stdout,"Binding server socket\n");
    // try to bind the socket!! :)
    if (bind(socketFileDescriptor,(struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        error("Error binding server socket");
    }

    fprintf(stdout,"Listening to server socket\n");
    // listen on the socket if we made it this far
    listen(socketFileDescriptor,5);

    // wait for a response (indefinitely?)
    for (;;) {
        clientAddrSz = sizeof(clientAddress);
        newSocketFileDescriptor  = accept(socketFileDescriptor,(struct sockaddr *) &clientAddress, &clientAddrSz);
        fprintf(stdout,"Got new connection!");
        if (newSocketFileDescriptor < 0) {
            error("Error accepting incoming connection");
        }

        // just write data to the new socket when it is ready
        bzero(transferBuffer,256);
        charsTransferred = recv(newSocketFileDescriptor,transferBuffer,255,0);
        fprintf(stdout,"Read %i characters in (%s)\n",charsTransferred,transferBuffer);

        if (charsTransferred < 0) {
            error("Could not read from socket!");
        }

        fprintf(stdout,"Beginning transmission of http response:\n");
        // write the http response from above
        for (size_t i = 0; strncmp(httpResponse[i],"\0",strlen(httpResponse[i])); i++) {
            charsTransferred = send(newSocketFileDescriptor,httpResponse[i],strlen(httpResponse[i]),0);
            if (charsTransferred < 0) {
                error("Error during transmission of http response");
            }

            fprintf(stdout,"Transferred http response line %li (%s), %i bytes written", i,httpResponse[i],charsTransferred);
        }

        // close the client socket!
        shutdown(newSocketFileDescriptor,SHUT_RDWR);
    }
}