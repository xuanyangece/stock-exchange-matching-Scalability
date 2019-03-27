#include "position.h"

#include <fstream>
#include <sstream>
#include <string>

using std::string;

void Position::createTable(connection * C) {
  string dropExistingTableSql = "DROP TABLE IF EXISTS POSITION CASCADE;";

  string createTableSql = "CREATE TABLE POSITION ("
                          "POSITION_ID     SERIAL  PRIMARY KEY NOT NULL, "
                          "SYMBOL_NAME     TEXT    NOT NULL, "
                          "ACCOUNT_ID      TEXT    NOT NULL, "
                          "NUM_SHARE       INT     NOT NULL);";

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
  sql << "Insert INTO POSITION (POSITION_ID, SYMBOL_NAME, ACCOUNT_ID, NUM_SHARE) ";
  sql << "VALUES ( DEFAULT, ";
  sql << W.quote(symbol_name) << ", ";
  sql << W.quote(account_id) << ", ";
  sql << W.quote(num_share) << ");";

  W.exec(sql.str());
  W.commit();
}

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

  return R[0][0].as<int>();
}

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
