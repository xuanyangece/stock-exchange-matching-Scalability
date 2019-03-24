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
    Parse create and dispatch different request.
*/
void create(std::string xml) {
    // Parse by line, greedy
    while (1) {
        // Get line by line break's position, xml stands for the remaining content to be parsed
        size_t linebreak = xml.find('\n');
        if (linebreak == std::string::npos) break;

        std::string line = xml.substr(0, linebreak);

        if (line.find("account") != std::string::npos) {
            // Create account: parse one line is enough
            std::string id = getAttribute(line, ID);
            std::string balance = getAttribute(line, BALANCE);
            createAccount(id, balance);

            // Skip one line
            xml = xml.substr(linebreak + 1);
        }
        else if (line.find("symbol") != std::string::npos) {
            // Create symbol: need to parse the whole symbol tag
            std::string symbol = getAttribute(line, SYM);
            
            // Get the info inside symbol tag
            size_t start = xml.find('\n') + 1;      // skip the <symbol ...> line
            size_t end = xml.find("</symbol>");     // end before </symbol>, containing line break!
            std::string accounts = xml.substr(start, end - start);

            parseSymbol(accounts, symbol);

            // Skip the entire symbol tag
            xml = xml.substr(end + 10);
        }
        else {
            // Otherwise skip one line
            xml = xml.substr(linebreak + 1);
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
        // NUM is between first '>' and second '<'
        size_t pos = remain.find('>') + 1;
        while (remain[pos] != '<') {
            ans += remain[pos++];
        }
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

/*
    Parse the entire symbol body:
    Do greedy to find all acounts and number of symbols to be added.
*/
void parseSymbol(std::string accounts, std::string symbol) {
    // Basically access each line and call createSymbol
    while (true) {
        size_t linebreak = accounts.find('\n');
        if (linebreak == std::string::npos) break; // end of parsing

        std::string id = getAttribute(accounts, ID);
        std::string amount = getAttribute(accounts, NUM);
        createSymbol(id, symbol, amount);

        // skip current acount
        accounts = accounts.substr(linebreak + 1);
    }
}

/*
    Parse transactions and dispatch different request.
*/
void transactions(std::string xml) {

}