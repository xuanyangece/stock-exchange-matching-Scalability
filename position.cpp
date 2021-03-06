#include "position.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using std::string;

/* Create the table without foreign keys */
void Position::createTable(connection * C) {
  string dropExistingTableSql = "DROP TABLE IF EXISTS POSITION CASCADE;";

  string createTableSql = "CREATE TABLE POSITION ("
                          "SYMBOL_NAME     TEXT    NOT NULL, "
                          "ACCOUNT_ID      TEXT    NOT NULL, "
                          "NUM_SHARE       INT     NOT NULL, "
                          "CONSTRAINT POSITION_PKEY PRIMARY KEY (SYMBOL_NAME, ACCOUNT_ID));";

  Table::createTable(C, dropExistingTableSql, createTableSql);
}

/* Build foreign keys */
void Position::buildForeignKeys(connection * C) {
  string buildForeignKeysSql = "ALTER TABLE POSITION "
                               "ADD CONSTRAINT POSITION_ACCOUNT_ID_FKEY FOREIGN KEY "
                               "(ACCOUNT_ID) REFERENCES ACCOUNT(ACCOUNT_ID);";

  Table::buildForeignKeys(C, buildForeignKeysSql);
}

/* Add a new entry to the table */
void Position::addPosition(connection * C,
                           const string & symbol_name,
                           const string & account_id,
                           int num_share) {
  /* Create a transactional object. */
  work W(*C);

  /* Create SQL statement */

  std::stringstream sql;
  sql << "Insert INTO POSITION (SYMBOL_NAME, ACCOUNT_ID, NUM_SHARE) ";
  sql << "VALUES (";
  sql << W.quote(symbol_name) << ", ";
  sql << W.quote(account_id) << ", ";
  sql << W.quote(num_share) << ") ";
  sql << "ON CONFLICT ON CONSTRAINT POSITION_PKEY ";
  sql << "DO UPDATE SET NUM_SHARE = POSITION.NUM_SHARE + ";
  sql << W.quote(num_share) << ";";

  W.exec(sql.str());
  W.commit();
}

/* Check if the given symbol already exists */
bool Position::isSymbolExists(connection * C,
                              const string & account_id,
                              const string & symbol_name) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT * FROM POSITION WHERE ACCOUNT_ID=";
  sql << N.quote(account_id) << " ";
  sql << "AND symbol_name=" << N.quote(symbol_name) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  return R.size() != 0;
}

/* Get the symbol amount of the given account_id and symbol_name */
int Position::getSymbolAmount(connection * C,
                              const string & account_id,
                              const string & symbol_name) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT NUM_SHARE FROM POSITION WHERE ACCOUNT_ID=";
  sql << N.quote(account_id) << " ";
  sql << "AND SYMBOL_NAME=" << N.quote(symbol_name) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));
  if (R.size() == 0) {
    return 0;
  }

  return R[0][0].as<int>();
}

/* Set the symbol amount of the given account_id and symbol_name */
void Position::setSymbolAmount(connection * C,
                               const string & account_id,
                               const string & symbol_name,
                               int amount) {
  /* Create a non-transactional object. */
  work W(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "UPDATE POSITION SET NUM_SHARE=";
  sql << W.quote(amount) << " ";
  sql << "WHERE ACCOUNT_ID=";
  sql << W.quote(account_id) << " ";
  sql << "AND SYMBOL_NAME=" << W.quote(symbol_name) << ";";

  /* Execute SQL query */
  W.exec(sql.str());
  W.commit();
}

/* Reduce the symbol amount of the given position atomically */
bool Position::reduceSymbolAmount(connection * C,
                                  const string & account_id,
                                  const string & symbol_name,
                                  int requiredAmount) {
  work W(*C);

  // Read
  std::stringstream sql1;
  sql1 << "SELECT NUM_SHARE FROM POSITION WHERE ACCOUNT_ID = ";
  sql1 << W.quote(account_id) << " ";
  sql1 << "AND SYMBOL_NAME = " << W.quote(symbol_name) << " FOR UPDATE;";

  result R(W.exec(sql1.str()));
  if (R.size() == 0) {
    return false;
  }

  // Modify
  int num_share = R[0]["NUM_SHARE"].as<int>();
  int remain = num_share - requiredAmount;

  if (remain < 0) {
    W.commit();
    return false;
  }

  // Write
  std::stringstream sql2;
  sql2 << "UPDATE POSITION SET NUM_SHARE = ";
  sql2 << W.quote(remain) << " ";
  sql2 << "WHERE ACCOUNT_ID = ";
  sql2 << W.quote(account_id) << " ";
  sql2 << "AND SYMBOL_NAME = " << W.quote(symbol_name) << ";";

  W.exec(sql2.str());
  W.commit();

  return true;
}

/* Add the symbol amount of the given position atomically */
void Position::addSymbolAmount(work & W,
                               const string & account_id,
                               const string & symbol_name,
                               int requiredAmount) {
  std::stringstream sql;
  sql << "Insert INTO POSITION (SYMBOL_NAME, ACCOUNT_ID, NUM_SHARE) ";
  sql << "VALUES (";
  sql << W.quote(symbol_name) << ", ";
  sql << W.quote(account_id) << ", ";
  sql << W.quote(requiredAmount) << ") ";
  sql << "ON CONFLICT ON CONSTRAINT POSITION_PKEY ";
  sql << "DO UPDATE SET NUM_SHARE = POSITION.NUM_SHARE + ";
  sql << W.quote(requiredAmount) << ";";

  W.exec(sql.str());
}
