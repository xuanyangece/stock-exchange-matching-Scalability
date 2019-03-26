#include "execution.h"

#include <fstream>
#include <sstream>
#include <string>

using std::string;

void Execution::createTable(connection * C) {
  string dropExistingTableSql = "DROP TABLE IF EXISTS EXECUTION CASCADE;";

  string createTableSql = "CREATE TABLE EXECUTION ("
                          "EXECUTION_ID    SERIAL  PRIMARY KEY NOT NULL, "
                          "BUYER_TRANS_ID  INT     NOT NULL, "
                          "SELLER_TRANS_ID INT     NOT NULL, "
                          "AMOUNT          INT     NOT NULL, "
                          "PRICE           REAL    NOT NULL, "
                          "TIME            BIGINT  NOT NULL);";

  Table::createTable(C, dropExistingTableSql, createTableSql);
}

/* Build foreign keys */
void Account::buildForeignKeys(connection * C) {
  string buildForeignKeysSql = "ALTER TABLE EXECUTION "
                               "ADD CONSTRAINT EXECUTION_TRANSACTION_ID_FKEY FOREIGN KEY "
                               "(BUYER_TRANS_ID) REFERENCES TRANSACTION(TRANSACTION_ID); "
                               "ALTER TABLE EXECUTION "
                               "ADD CONSTRAINT EXECUTION_TRANSACTION_ID_FKEY FOREIGN KEY "
                               "(SELLER_TRANS_ID) REFERENCES TRANSACTION(TRANSACTION_ID);";

  Table::buildForeignKeys(C, buildForeignKeysSql);
}

/* Add a new entry to the table */
void Execution::addExecution(connection * C,
                             int _buyer_trans_id,
                             int _seller_trans_id,
                             int _amount,
                             double _price) {
  /* Create a transactional object. */
  work W(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "Insert INTO EXECUTION (EXECUTION_ID, BUYER_TRANS_ID, SELLER_TRANS_ID, ";
  sql << "AMOUNT, PRICE, TIME) " sql << "VALUES (DEFAULT, ";
  sql << W.quote(_buyer_trans_id) << ", ";
  sql << W.quote(_seller_trans_id) << ", ";
  sql << W.quote(_amount) << ", ";
  sql << W.quote(_price) << ", ";
  sql << W.quote(getEpoch()) << ");";

  W.exec(sql.str());
  W.commit();
}

/* Check if the given account already exists */
bool Account::isAccountExists(connection * C, int _account_id) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT * FROM ACCOUNT WHERE ACCOUNT_ID=";
  sql << N.quote(_account_id) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  return R.size() != 0;
}

double Account::getBalance(connection * C, int _account_id) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT_ID=";
  sql << N.quote(_account_id) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  return R[0][0].as<double>();
}

void Account::setBalance(connection * C, int _account_id, double _balance) {
  /* Create a non-transactional object. */
  work W(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "UPDATE ACCOUNT SET BALANCE=";
  sql << W.quote(_balance) << " ";
  sql << "WHERE ACCOUNT_ID=";
  sql << W.quote(_account_id) << ";";

  /* Execute SQL query */
  W.exec(sql.str());
  W.commit();
}
