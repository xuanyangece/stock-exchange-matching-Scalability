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
                          "ACCOUNT_ID      INT     NOT NULL, "
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
void Position::addEntry(connection * C,
                        const string & _symbol_name,
                        int _account_id,
                        int _num_share) {
  /* Create a transactional object. */
  work W(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "Insert INTO POSITION (POSITION_ID, SYMBOL_NAME, ACCOUNT_ID, NUM_SHARE) ";
  sql << "VALUES (";
  sql << W.quote("DEFAULT") << ", ";
  sql << W.quote(_symbol_name) << ", ";
  sql << W.quote(_account_id) << ", ";
  sql << W.quote(_num_share) << ");";

  W.exec(sql.str());
  W.commit();
}