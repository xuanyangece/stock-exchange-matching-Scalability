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
                          "NUM_CANCELED    INT     NOT NULL, "
                          "OPEN_TIME       BIGINT  NOT NULL, "
                          "CANCEL_TIME     BIGINT  NOT NULL DEFAULT 0);";

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
         "LIMITED, NUM_OPEN, NUM_CANCELED, OPEN_TIME, CANCEL_TIME) ";
  sql << "VALUES (DEFAULT, ";
  sql << W.quote(_account_id) << ", ";
  sql << W.quote(_symbol_name) << ", ";
  sql << W.quote(_limited) << ", ";
  sql << W.quote(_num_open) << ", ";
  sql << W.quote(0) << ", ";
  sql << W.quote(getEpoch()) << ", DEFAULT);";

  W.exec(sql.str());
  W.commit();
}

bool Transaction::isTransExists(connection * C, int trans_id) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT * FROM TRANSACTION WHERE TRANSACTION_ID=";
  sql << N.quote(trans_id) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  return R.size() != 0;
}

bool Transaction::isTransCompleted(connection * C, int trans_id) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT NUM_OPEN FROM TRANSACTION WHERE TRANSACTION_ID=";
  sql << N.quote(trans_id) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  return R[0][0].as<int>() == 0;
}

bool Transaction::isTransCanceled(connection * C, int trans_id) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT NUM_CANCELED FROM TRANSACTION WHERE TRANSACTION_ID=";
  sql << N.quote(trans_id) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  return R[0][0].as<int>() != 0;
}

long Transaction::getCanceledTime(connection * C, int trans_id) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT CANCEL_TIME FROM TRANSACTION WHERE TRANSACTION_ID=";
  sql << N.quote(trans_id) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  return R[0][0].as<long>();
}

int Transaction::getCanceledShares(connection * C, int trans_id) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT NUM_CANCELED FROM TRANSACTION WHERE TRANSACTION_ID=";
  sql << N.quote(trans_id) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  return R[0][0].as<int>();
}

int Transaction::getOpenShares(connection * C, int trans_id) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT NUM_OPEN FROM TRANSACTION WHERE TRANSACTION_ID=";
  sql << N.quote(trans_id) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  return R[0][0].as<int>();
}

const std::string Transaction::queryExecuted(connection * C, int trans_id) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT AMOUNT, PRICE, TIME FROM EXECUTION WHERE BUYER_TRANS_ID=";
  sql << N.quote(trans_id) << " ";
  sql << "OR SELLER_TRANS_ID=" << N.quote(trans_id) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  std::stringstream ss;

  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    ss << "    <executed shares=" << c[0].as<int>() << " ";
    ss << "price=" << c[1].as<double>() << " ";
    ss << "time=" << c[2].as<long>() << "/>\n";
  }

  return ss.str();
}

// void Transaction::queryTransaction() {}

void Transaction::cancelTransaction(connection * C, int trans_id) {
  int open_shares = Transaction::getOpenShares(C, trans_id);
  Transaction::setOpenShares(C, trans_id, 0);
  Transaction::setCanceledShares(C, trans_id, open_shares);
  Transaction::setCanceledTime(C, trans_id, getEpoch());
}

void Transaction::setOpenShares(connection * C, int trans_id, int amount) {
  /* Create a non-transactional object. */
  work W(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "UPDATE TRANSACTION SET NUM_OPEN=";
  sql << W.quote(amount) << " ";
  sql << "WHERE TRANSACTION_ID=";
  sql << W.quote(trans_id) << ";";

  /* Execute SQL query */
  W.exec(sql.str());
  W.commit();
}

void Transaction::setCanceledShares(connection * C, int trans_id, int amount) {
  /* Create a non-transactional object. */
  work W(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "UPDATE TRANSACTION SET NUM_CANCELED=";
  sql << W.quote(amount) << " ";
  sql << "WHERE TRANSACTION_ID=";
  sql << W.quote(trans_id) << ";";

  /* Execute SQL query */
  W.exec(sql.str());
  W.commit();
}

void Transaction::setCanceledTime(connection * C, int trans_id, long time) {
  /* Create a non-transactional object. */
  work W(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "UPDATE TRANSACTION SET CANCEL_TIME=";
  sql << W.quote(time) << " ";
  sql << "WHERE TRANSACTION_ID=";
  sql << W.quote(trans_id) << ";";

  /* Execute SQL query */
  W.exec(sql.str());
  W.commit();
}
