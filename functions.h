#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <unistd.h>

#include <chrono>
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
using namespace std::chrono;

extern std::mutex mymutex;

/*
    Lock using RAII
*/
//BEGIN_REF - https://www.youtube.com/watch?v=ojOUIg13g3I&t=543s
class MyLock
{
 private:
  std::mutex * mtx;

 public:
  explicit MyLock(std::mutex * temp) {
    temp->lock();
    mtx = temp;
  }

  ~MyLock() { mtx->unlock(); }
};
//END_REF

/*
    Receive XML from client and keep connection until invalid format.
*/
void handleXML(connection * C, int client_fd);  // parse

const connection * createConnection();

/*
    Parse create and dispatch different request.
*/
const std::string create(connection * C, std::string xml);  // create account or symbol

/*
    Create account, return response.
*/
const std::string createAccount(connection * C,
                                const std::string & account_id_str,
                                const std::string & balance_str);

/*
    Create a symbol associated with the account, return response.
*/
const std::string createSymbol(connection * C,
                               const std::string & account_id_str,
                               const std::string & symbol_name,
                               const std::string & num_share_str);

/*
    Parse the entire symbol body:
    Do greedy to find all acounts and number of symbols to be added.
*/
const std::string parseSymbol(connection * C, std::string accounts, std::string symbol);

/*
    Parse transactions and dispatch different request.
*/
const std::string transactions(connection * C, std::string xml);  // deal with transactions

/*
    Handle order request, return response.
*/
const std::string order(connection * C,
                        const std::string & account_id_str,
                        const std::string & symbol,
                        const std::string & amount_str,
                        const std::string & limit_str);

/*
    Handle cancel request, return response.
*/
const std::string cancel(connection * C,
                         const std::string & account_id_str,
                         const std::string & trans_id_str);

/*
    Handle query request, return response.
*/
const std::string query(connection * C,
                        const std::string & account_id_str,
                        const std::string & trans_id_str);

/*
    Access attribute from giving string.
    In our design, it accesses the first one.
*/
const std::string getAttribute(std::string remain, std::string attribute);

/*
    Check whether given string is all digits.
*/
bool isDigits(const std::string & str);

/*
    Check whether given string is alphanumeric to fit the symbol name pattern.
*/
bool isAlphaDigits(const std::string & str);

/*
    Check whether given string represents a positive double.
*/
bool isPositiveDouble(const std::string & str);

/*
    Check whether given string represents a non-zero integer.
*/
bool isNonZeroInt(const std::string & str);

/*
    Return error related to account.
*/
const std::string getCreateAccountError(const std::string & account_id_str,
                                        const std::string & msg);

/*
    Return error related to symbol.
*/
const std::string getCreateSymbolError(const std::string & account_id_str,
                                       const std::string & symbol_name,
                                       const std::string & msg);

/*
    Return error related to order.
*/
const std::string getOrderError(const std::string & symbol_name,
                                const std::string & amount,
                                const std::string & limit,
                                const std::string & msg);

/*
    Return error related to trans_id.
*/
const std::string getTransIDError(const std::string & trans_id_str, const std::string & msg);

/*
    Helper function to get current timestamp.
*/
long getEpoch();

#endif
