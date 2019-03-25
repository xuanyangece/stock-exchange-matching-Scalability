#include <fstream>
#include <iostream>
#include <pqxx/pqxx>

#include "account.h"
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

    Account::buildForeignKeys(C);
    Position::buildForeignKeys(C);
    Transaction::buildForeignKeys(C);
  }
  catch (const std::exception & e) {
    cerr << e.what() << std::endl;
    return 1;
  }

  // exercise(C);

  // Close database connection
  C->disconnect();
  delete C;

  return 0;
}
