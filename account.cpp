#include "account.h"

#include <fstream>
#include <sstream>
#include <string>

using std::string;

void Account::createTable(connection * C) {
  string dropExistingTableSql = "DROP TABLE IF EXISTS ACCOUNT CASCADE;";

  string createTableSql = "CREATE TABLE ACCOUNT ("
                          "ACCOUNT_ID      INT     PRIMARY KEY NOT NULL, "
                          "BALANCE         REAL    NOT NULL);";

  Table::createTable(C, dropExistingTableSql, createTableSql);
}

/* Build foreign keys */
void Account::buildForeignKeys(connection * C) {
  string buildForeignKeysSql = ";";
  Table::buildForeignKeys(C, buildForeignKeysSql);
}

/* Add a new entry to the table */
void Account::addEntry(connection * C, int _account_id, double _balance) {
  /* Create a transactional object. */
  work W(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "Insert INTO ACCOUNT (ACCOUNT_ID, BALANCE) ";
  sql << "VALUES (";
  sql << W.quote(_account_id) << ", ";
  sql << W.quote(_balance) << ");";

  W.exec(sql.str());
  W.commit();
}
