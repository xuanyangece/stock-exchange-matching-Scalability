#include <iostream>
#include <exception>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <thread>
#include <unistd.h>
#include <string>

void handleXML(int client_fd); // parse

void create(std::string xml); // create account or symbol

void createAccount(std::string id, std::string balance);

void createSymbol(std::string id, std::string symbol, std::string amount);

void transactions(std::string xml); // deal with transactions

void order(int client_fd);

void cancel(int client_fd);

void query(int client_fd);

std::string getAttribute(std::string remain, std::string attribute);