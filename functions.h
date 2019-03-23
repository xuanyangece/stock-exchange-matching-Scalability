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

void createAccount(int id, int balance);

void createSymbol(int id, std::string symbol, int amount);

void transactions(std::string xml); // deal with transactions

void order(int client_fd);

void cancel(int client_fd);

void query(int client_fd);