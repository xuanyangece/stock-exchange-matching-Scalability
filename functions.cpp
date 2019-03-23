#include "functions.h"

#define DEBUG 1
#define BUFFSIZE 40960
#define ID "id"
#define BALANCE "balance"
#define SYM "sym"
#define NUM "num"

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

/*
    Create symbol or account.
*/
void create(std::string xml) {
    // Parse by line, greedy
    while (1) {
        // Get line by line break's position
        size_t linebreak = xml.find('\n');

        if (linebreak == std::string::npos) break;

        std::string line = xml.substr(0, linebreak);

        if (line.find("account") != std::string::npos) {
            // Create account: pass line is enough
            std::string id = getAttribute(line, ID);
            std::string balance = getAttribute(line, BALANCE);
            createAccount(id, balance);

            // Skip one line
            xml = xml.substr(linebreak + 1);
        }
        else if (line.find("symbol") != std::string::npos) {
            // Create symbol: need to pass remain xml because of multiple lines
            std::string id = getAttribute(xml, ID);
            std::string symbol = getAttribute(xml, SYM);
            std::string amount = getAttribute(xml, NUM);
            createSymbol(id, symbol, amount);

            // Skil multiple lines
            size_t end_of_sym = xml.find("</symbol>");
            xml = xml.substr(end_of_sym + 10); // skip the word "</symbol>"
        }
    }
}

/*
    Access attribute from giving string.
    In our deisn, it accesses the first one.
*/
std::string getAttribute(std::string remain, std::string attribute) {
    std::string ans;
    
    if (attribute == NUM) {

    }
    else {
        size_t pos = remain.find(attribute);
        
        // try to find open parenthesis
        while (remain[pos] != '\"') {
            pos++;
        }

        pos++; // skip open parenthesis
        
        while (remain[pos] != '\"') {
            ans += remain[pos++];
        }
    }

    return ans;
}