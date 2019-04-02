#include "account.h"

#include <fstream>
#include <sstream>
#include <string>

using std::string;

void Account::createTable(connection * C) {
  string dropExistingTableSql = "DROP TABLE IF EXISTS ACCOUNT CASCADE;";

  string createTableSql = "CREATE TABLE ACCOUNT ("
                          "ACCOUNT_ID      TEXT    PRIMARY KEY NOT NULL, "
                          "BALANCE         REAL    NOT NULL);";

  Table::createTable(C, dropExistingTableSql, createTableSql);
}

/* Build foreign keys */
void Account::buildForeignKeys(connection * C) {
  string buildForeignKeysSql = ";";
  Table::buildForeignKeys(C, buildForeignKeysSql);
}

/* Add a new entry to the table, return true if succeed */
bool Account::addAccount(connection * C, const string & account_id, double balance) {
  /* Create a transactional object. */
  work W(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "Insert INTO ACCOUNT (ACCOUNT_ID, BALANCE) ";
  sql << "VALUES (";
  sql << W.quote(account_id) << ", ";
  sql << W.quote(balance) << ");";

  try {
    W.exec(sql.str());
    W.commit();
  }
  catch (const std::exception & e) {
    if (DEBUG) {
      std::cerr << e.what() << std::endl;
    }
    W.abort();
    return false;
  }

  return true;
}

/* Check if the given account already exists */
bool Account::isAccountExists(connection * C, const string & account_id) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT * FROM ACCOUNT WHERE ACCOUNT_ID=";
  sql << N.quote(account_id) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  return R.size() != 0;
}

double Account::getBalance(connection * C, const string & account_id) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT_ID=";
  sql << N.quote(account_id) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  return R[0][0].as<double>();
}

void Account::setBalance(connection * C, const string & account_id, double balance) {
  /* Create a non-transactional object. */
  work W(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "UPDATE ACCOUNT SET BALANCE=";
  sql << W.quote(balance) << " ";
  sql << "WHERE ACCOUNT_ID=";
  sql << W.quote(account_id) << ";";

  /* Execute SQL query */
  W.exec(sql.str());
  W.commit();
}

bool Account::reduceBalance(connection * C, const string & account_id, double payment) {
  work W(*C);

  std::stringstream sql1;
  sql1 << "SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT_ID=";
  sql1 << W.quote(account_id) << " FOR UPDATE;";

  result R(W.exec(sql1.str()));

  double amount = R[0]["BALANCE"].as<double>();
  double remain = amount - payment;
  if (remain < 0) {
    W.commit();
    return false;
  }

  std::stringstream sql2;
  sql2 << "UPDATE ACCOUNT SET BALANCE = ";
  sql2 << W.quote(remain) << " ";
  sql2 << "WHERE ACCOUNT_ID=";
  sql2 << W.quote(account_id) << ";";

  W.exec(sql2.str());
  W.commit();

  return true;
}
