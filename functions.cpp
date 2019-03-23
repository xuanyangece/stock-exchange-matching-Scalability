#include "functions.h"

#define DEBUG 1
#define BUFFSIZE 40960

/*
    Receive XML from client and keep connection until invalid format.
*/
void handleXML(int client_fd) {
    // loop and dispatch
    char buffer[BUFFSIZE];

    while (1) {
        // Reset buffer
        memset(buffer, '\0', sizeof(buffer));

        // Recv
        int totalsize = recv(client_fd, buffer, BUFFSIZE, 0);
        if (totalsize < 0) std::cout<<"Error receive buffer from client."<<std::endl;

        std::string xml(buffer);

        if (DEBUG) std::cout<<"Buffer received from client: "<<std::endl<<xml<<std::endl;

        // Assume it's correct
        if (xml.find("<create>") != std::string::npos) {
            create(xml);
        }
        else if (xml.find("<transactions>") != std::string::npos) {
            transactions(xml);
        }
        else {
            // Close connection
            close(client_fd);
            break;
        }
    }
}