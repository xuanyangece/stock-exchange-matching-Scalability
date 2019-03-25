#include "transaction.h"

#include <fstream>
#include <sstream>
#include <string>

using std::string;

void Transaction::createTable(connection * C) {
  string dropExistingTableSql = "DROP TABLE IF EXISTS TRANSACTION CASCADE;";

  string createTableSql = "CREATE TABLE TRANSACTION ("
                          "TRANSACTION_ID  SERIAL  PRIMARY KEY NOT NULL, "
                          "ACCOUNT_ID      INT     NOT NULL, "
                          "SYMBOL_NAME     TEXT    NOT NULL, "
                          "LIMITED         REAL    NOT NULL, "
                          "NUM_OPEN        INT     NOT NULL, "
                          "NUM_EXECUTED    INT     NOT NULL, "
                          "NUM_CANCELED    INT     NOT NULL);";

  Table::createTable(C, dropExistingTableSql, createTableSql);
}

/* Build foreign keys */
void Transaction::buildForeignKeys(connection * C) {
  string buildForeignKeysSql = "ALTER TABLE TRANSACTION "
                               "ADD CONSTRAINT TRANSACTION_ACCOUNT_ID_FKEY FOREIGN KEY "
                               "(ACCOUNT_ID) REFERENCES ACCOUNT(ACCOUNT_ID);";

  Table::buildForeignKeys(C, buildForeignKeysSql);
}

/* Add a new entry to the table */
void Transaction::addTransaction(connection * C,
                                 int _account_id,
                                 const string & _symbol_name,
                                 double _limited,
                                 int _num_open) {
  /* Create a transactional object. */
  work W(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "Insert INTO TRANSACTION (TRANSACTION_ID, ACCOUNT_ID, SYMBOL_NAME, "
         "LIMIT, NUM_OPEN, NUM_EXECUTED, NUM_CANCELED) ";
  sql << "VALUES (";
  sql << W.quote("DEFAULT") << ", ";
  sql << W.quote(_account_id) << ", ";
  sql << W.quote(_symbol_name) << ", ";
  sql << W.quote(_limited) << ", ";
  sql << W.quote(_num_open) << ", ";
  sql << W.quote(0) << ", ";
  sql << W.quote(0) << ");";

  W.exec(sql.str());
  W.commit();
}
