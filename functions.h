#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>

#include <cstring>
#include <exception>
#include <iostream>
#include <mutex>
#include <pqxx/pqxx>
#include <string>
#include <thread>

#include "account.h"
#include "position.h"
#include "transaction.h"

using namespace pqxx;

void handleXML(int client_fd);  // parse

void create(std::string xml);  // create account or symbol

void parseSymbol(std::string accounts, std::string symbol);

void transactions(std::string xml);  // deal with transactions

void order(std::string account_id, std::string symbol, std::string amount, std::string limit);

void cancel(std::string account_id, std::string trans_id);

void query(std::string account_id, std::string trans_id);

std::string getAttribute(std::string remain, std::string attribute);

const std::string createAccount(connection * C,
                                const std::string & account_id_str,
                                const std::string & balance_str);

bool isDigits(const std::string & str);

bool isAccountExists(connection * C, int account_id);

bool isDouble(const std::string & str);

const std::string getCreateError(const std::string & account_id_str, const std::string & msg);

const std::string createSymbol(connection * C,
                               std::string id,
                               std::string symbol,
                               std::string amount);

#endif
