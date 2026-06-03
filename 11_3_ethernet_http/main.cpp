#include "EthernetInterface.h"

#define IP_ADDRESS "192.0.2.60"
#define REQUEST "GET / HTTP/1.1\r\nHost: " IP_ADDRESS "\r\nConnection: close\r\n\r\n"

int main() {
    EthernetInterface ethernet_interface;
    TCPSocket tcp_socket;
    printf("Ethernet interface is connecting.\n");
    assert(ethernet_interface.connect() == 0);
    printf("TCP socket is opening.\n");
    assert(tcp_socket.open(&ethernet_interface) == 0);
    printf("TCP socket is connecting: %s\n", IP_ADDRESS);
    assert(tcp_socket.connect({IP_ADDRESS, 80}) == 0);
    printf("TCP socket is sending request:\n%s", REQUEST);
    assert(tcp_socket.send(REQUEST, sizeof(REQUEST) - 1) == (sizeof(REQUEST) - 1));
    printf("TCP socket is receiving response:\n");
    int number_of_bytes;
    char buffer[8];
    while ((number_of_bytes = tcp_socket.recv(buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, number_of_bytes, stdout);
    }
    assert(number_of_bytes == 0);
    printf("TCP socket is closing.\n");
    assert(tcp_socket.close() == 0);
    printf("Ethernet interface is disconnecting.\n");
    assert(ethernet_interface.disconnect() == 0);
    printf("The main function is returning 0.\n");
    return 0;
}
