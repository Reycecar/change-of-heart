#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#pragma comment(lib, "Rpcrt4.lib")
using namespace std;

int main(int argc, char* argv[]) {
    int server_socket, client_socket, port_number, backlog_size = 5;
    struct sockaddr_in server_address, client_address;
    socklen_t client_length = sizeof(client_address);

    // check command line arguments
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <port number>" << endl;
        return EXIT_FAILURE;
    }
    port_number = atoi(argv[1]);

    // create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        cerr << "Error: could not create socket" << endl;
        return EXIT_FAILURE;
    }

    // prepare server address
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port_number);

    // bind socket to server address
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        cerr << "Error: could not bind socket to address" << endl;
        return EXIT_FAILURE;
    }

    // listen for client connections
    listen(server_socket, backlog_size);

    // accept client connections
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_length);
        if (client_socket < 0) {
            cerr << "Error: could not accept client connection" << endl;
            continue;
        }

        // handle client communication
        // [insert specific steps here to handle client communication, depending on your application requirements]

        // close client socket
        close(client_socket);
    }

    // close server socket
    close(server_socket);

    return EXIT_SUCCESS;
}