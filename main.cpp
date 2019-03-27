#include <fstream>
#include <iostream>
#include <pqxx/pqxx>

#include "account.h"
#include "execution.h"
#include "functions.h"
#include "position.h"
#include "table.h"
#include "transaction.h"

using namespace std;
using namespace pqxx;

int main(int argc, char * argv[]) {
  // Allocate & initialize a Postgres connection object
  connection * C;

  try {
    // Establish a connection to the database
    // Parameters: database name, user name, user password
    C = new connection("dbname=ACC_BBALL user=postgres password=passw0rd");
    if (C->is_open()) {
      // cout << "Opened database successfully: " << C->dbname() << endl;
    }
    else {
      cout << "Can't open database" << endl;
      return 1;
    }
  }
  catch (const std::exception & e) {
    cerr << e.what() << std::endl;
    return 1;
  }

  try {
    /* Drop existing tables and create new tables */
    Account::createTable(C);
    Position::createTable(C);
    Transaction::createTable(C);
    Execution::createTable(C);

    Account::buildForeignKeys(C);
    Position::buildForeignKeys(C);
    Transaction::buildForeignKeys(C);
    Execution::buildForeignKeys(C);
  }
  catch (const std::exception & e) {
    cerr << e.what() << std::endl;
    return 1;
  }

  // exercise(C);

  std::cout << createAccount(C, "1001", "1000");
  std::cout << createAccount(C, "1002", "1000");
  std::cout << createAccount(C, "1003", "1000");

  std::cout << createAccount(C, "1004a", "1000");
  std::cout << createAccount(C, "1004", "10a00");
  std::cout << createAccount(C, "1004", "-1000");
  std::cout << createAccount(C, "1004", "1000");

  std::cout << createAccount(C, "1001", "1000");

  std::cout << createSymbol(C, "1001", "tengxun", "100");
  std::cout << Position::getSymbolAmount(C, "1001", "tengxun") << endl;

  std::cout << createSymbol(C, "1001", "baidu", "50");
  std::cout << Position::getSymbolAmount(C, "1001", "baidu") << endl;
  std::cout << createSymbol(C, "1001", "baidu", "50");
  std::cout << Position::getSymbolAmount(C, "1001", "baidu") << endl;
  std::cout << createSymbol(C, "1008", "baidu", "100");
  std::cout << createSymbol(C, "1001", "baidu", "-10");
  std::cout << createSymbol(C, "1001", "baidu0_", "50");
  std::cout << createSymbol(C, "1001", "baidu", "0");

  std::cout << "Epoch: " << getEpoch() << std::endl;
  std::cout << Transaction::addTransaction(C, "1001", "baidu", 127.3, 100) << std::endl;
  std::cout << Transaction::addTransaction(C, "1001", "baidu", 127.3, 100) << std::endl;
  std::cout << Transaction::addTransaction(C, "1001", "baidu", 127.3, 100) << std::endl;

  // Close database connection
  C->disconnect();
  delete C;

  return 0;
}
