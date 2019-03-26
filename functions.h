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
#include <chrono>

#include "account.h"
#include "position.h"
#include "transaction.h"

using namespace pqxx;
using namespace std::chrono;

void handleXML(connection * C, int client_fd);  // parse

void create(connection * C, std::string xml);  // create account or symbol

void parseSymbol(connection * C, std::string accounts, std::string symbol);

void transactions(connection * C, std::string xml);  // deal with transactions

const std::string order(connection * C,
                        const std::string & account_id_str,
                        const std::string & symbol,
                        const std::string & amount_str,
                        const std::string & limit_str);

const std::string cancel(connection * C,
                         const std::string & account_id_str,
                         const std::string & trans_id_str);

const std::string query(connection * C,
                        const std::string & account_id_str,
                        const std::string & trans_id_str);

const std::string getAttribute(std::string remain, std::string attribute);

const std::string createAccount(connection * C,
                                const std::string & account_id_str,
                                const std::string & balance_str);

bool isDigits(const std::string & str);

bool isAlphaDigits(const std::string & str);

bool isDouble(const std::string & str);

bool isNonZeroInt(const std::string & str);

const std::string getCreateAccountError(const std::string & account_id_str,
                                        const std::string & msg);

const std::string createSymbol(connection * C,
                               const std::string & account_id_str,
                               const std::string & symbol_name,
                               const std::string & num_share_str);

const std::string getCreateSymbolError(const std::string & account_id_str,
                                       const std::string & symbol_name,
                                       const std::string & msg);

const std::string getOrderError(const std::string & symbol_name, 
                                        const std::string & amount, 
                                        const std::string & limit,
                                        const std::string & msg);

const std::string getTransIDError(const std::string & trans_id_str, 
                                        const std::string & msg);


long getEpoch();

#endif
