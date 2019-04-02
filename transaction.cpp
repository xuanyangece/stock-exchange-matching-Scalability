#include "transaction.h"

#include <fstream>
#include <sstream>
#include <string>

#include "execution.h"

using std::string;

void Transaction::createTable(connection * C) {
  string dropExistingTableSql = "DROP TABLE IF EXISTS TRANSACTION CASCADE;";

  string createTableSql = "CREATE TABLE TRANSACTION ("
                          "TRANSACTION_ID  SERIAL  PRIMARY KEY NOT NULL, "
                          "ACCOUNT_ID      TEXT    NOT NULL, "
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
int Transaction::addTransaction(connection * C,
                                const string & account_id,
                                const string & symbol_name,
                                double limited,
                                int num_open) {
  /* Create a transactional object. */
  work W(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "Insert INTO TRANSACTION (TRANSACTION_ID, ACCOUNT_ID, SYMBOL_NAME, "
         "LIMITED, NUM_OPEN, NUM_CANCELED, OPEN_TIME, CANCEL_TIME) ";
  sql << "VALUES (DEFAULT, ";
  sql << W.quote(account_id) << ", ";
  sql << W.quote(symbol_name) << ", ";
  sql << W.quote(limited) << ", ";
  sql << W.quote(num_open) << ", ";
  sql << W.quote(0) << ", ";
  sql << W.quote(getEpoch()) << ", DEFAULT) RETURNING TRANSACTION_ID;";

  result R(W.exec(sql.str()));
  W.commit();

  return R[0][0].as<int>();
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

int Transaction::getOpenSharesByWork(work & W, int trans_id) {
  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT NUM_OPEN FROM TRANSACTION WHERE TRANSACTION_ID=";
  sql << W.quote(trans_id) << ";";

  /* Execute SQL query */
  result R(W.exec(sql.str()));

  return R[0][0].as<int>();
}

string Transaction::getAccountID(connection * C, int trans_id) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT ACCOUNT_ID FROM TRANSACTION WHERE TRANSACTION_ID=";
  sql << N.quote(trans_id) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  return R[0][0].as<string>();
}

double Transaction::getLimited(connection * C, int trans_id) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT LIMITED FROM TRANSACTION WHERE TRANSACTION_ID=";
  sql << N.quote(trans_id) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  return R[0][0].as<double>();
}

string Transaction::getSymbolName(connection * C, int trans_id) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT SYMBOL_NAME FROM TRANSACTION WHERE TRANSACTION_ID=";
  sql << N.quote(trans_id) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  return R[0][0].as<string>();
}

const std::string Transaction::queryExecuted(connection * C, int trans_id) {
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "SELECT BUYER_TRANS_ID, SELLER_TRANS_ID, AMOUNT, PRICE, TIME FROM EXECUTION WHERE "
         "BUYER_TRANS_ID=";
  sql << N.quote(trans_id) << " ";
  sql << "OR SELLER_TRANS_ID=" << N.quote(trans_id) << ";";

  /* Execute SQL query */
  result R(N.exec(sql.str()));

  std::stringstream ss;

  for (result::const_iterator c = R.begin(); c != R.end(); ++c) {
    ss << "    <executed shares=\"";
    if (trans_id == c[0].as<int>()) {
      ss << c[2].as<int>() << "\" ";
    }
    else {
      ss << 0 - c[2].as<int>() << "\" ";
    }
    ss << "price=\"" << c[3].as<double>() << "\" ";
    ss << "time=\"" << c[4].as<long>() << "\"/>\n";
  }

  return ss.str();
}

// void Transaction::queryTransaction() {}

bool Transaction::cancelTransaction(connection * C, int trans_id) {
  /* Create a transactional object. */
  work W1(*C);

  /* Create SQL statement */
  std::stringstream sql1;
  sql1 << "SELECT NUM_OPEN FROM TRANSACTION WHERE TRANSACTION_ID=";
  sql1 << W1.quote(trans_id) << " FOR UPDATE;";

  /* Execute SQL query */
  result R(W1.exec(sql1.str()));

  int num_open = R[0]["NUM_OPEN"].as<int>();

  // Trans already completed
  if (num_open == 0) {
    W1.commit();
    return false;
  }

  /* Create SQL statement */
  std::stringstream sql2;
  sql2 << "UPDATE TRANSACTION SET ";
  sql2 << "NUM_CANCEL = TRANSACTION.NUM_OPEN, ";
  sql2 << "NUM_OPEN = 0, ";
  sql2 << "CANCEL_TIME = " << W1.quote(getEpoch()) << ";";

  W1.exec(sql2);
  W1.commit();

  string account_id = Transaction::getAccountID(C, trans_id);

  bool isBuyer = (num_open > 0);

  work W2(*C);

  if (isBuyer) {  // return money
    double limited = Transaction::getLimited(C, trans_id);

    std::stringstream sql3;
    sql3 << "SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT_ID = ";
    sql3 << W2.quote(account_id) << " FOR UPDATE;";

    result R2(W2.exec(sql3.str()));
    double balance = R2[0]["BALANCE"].as<double>();

    balance = balance + num_open * limited;

    std::stringstream sql4;
    sql4 << "UPDATE ACCOUNT SET BALANCE = ";
    sql4 << W2.quote(balance) << " ";
    sql4 << "WHERE ACCOUNT_ID = ";
    sql4 << W2.quote(account_id) << ";";

    W2.exec(sql4);
    W2.commit();
  }
  else {  // return symbol
    string symbol_name = Transaction::getSymbolName(C, trans_id);

    /* Create SQL statement */
    std::stringstream sql5;
    sql5 << "UPDATE POSITION SET ";
    sql5 << "NUM_SHARE = POSITION.NUM_SHARE - " << W2.quote(num_open) << " ";
    sql5 << "WHERE ACCOUNT_ID = ";
    sql5 << W2.quote(account_id) << " ";
    sql5 << "AND SYMBOL_NAME = " << W2.quote(symbol_name) << ";";

    /* Execute SQL query */
    W2.exec(sql5.str());
    W2.commit();
  }

  return true;
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

void Transaction::setOpenSharesByWork(work & W, int trans_id, int amount) {
  /* Create SQL statement */
  std::stringstream sql;
  sql << "UPDATE TRANSACTION SET NUM_OPEN=";
  sql << W.quote(amount) << " ";
  sql << "WHERE TRANSACTION_ID=";
  sql << W.quote(trans_id) << ";";

  /* Execute SQL query */
  W.exec(sql.str());
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

bool Transaction::tryMatch(connection * C, int trans_id) {
  // Add mutex
  MyLock lk(&mymutex);

  /* Create a non-transactional object. */
  work W(*C);

  // Lock all rows related to current transaction's owner
  std::stringstream lockSql;
  lockSql << "SELECT * FROM ACCOUNT, POSITION, TRANSACTION WHERE ";
  lockSql << "TRANSACTION.TRANSACTION_ID = " << W.quote(trans_id) << " ";
  lockSql << "AND ACCOUNT.ACCOUNT_ID = TRANSACTION.ACCOUNT_ID ";
  lockSql << "AND POSITION.ACCOUNT_ID = TRANSACTION.ACCOUNT_ID ";
  lockSql << "AND POSITION.SYMBOL_NAME = TRANSACTION.SYMBOL_NAME ";
  lockSql << "FOR UPDATE;";

  W.exec(lockSql.str());

  // get info of trans_id
  std::stringstream getinfo;
  getinfo << "SELECT * FROM TRANSACTION WHERE TRANSACTION_ID = ";
  getinfo << W.quote(trans_id) << ";";

  result R1(W.exec(getinfo.str()));

  string account_id = R1[0]["ACCOUNT_ID"].as<string>();
  string symbol_name = R1[0]["SYMBOL_NAME"].as<string>();
  double limited = R1[0]["LIMITED"].as<double>();
  int num_open = R1[0]["NUM_OPEN"].as<int>();

  // Get the first matching transactions
  std::stringstream getTrans;
  getTrans << "SELECT * FROM TRANSACTION WHERE ACCOUNT_ID != ";
  getTrans << W.quote(account_id) << " ";
  getTrans << "AND SYMBOL_NAME = " << W.quote(symbol_name) << " ";

  bool isBuyer = (num_open > 0);
  int final_amount;
  double final_price;

  if (isBuyer) {
    getTrans << "AND LIMITED <= " << W.quote(limited) << " ";
    getTrans << "AND NUM_OPEN < 0 ";
    getTrans << "ORDER BY LIMITED ASC, OPEN_TIME ASC LIMIT 1 ";
    getTrans << "FOR UPDATE;";
  }
  else {
    getTrans << "AND LIMITED >= " << W.quote(limited) << " ";
    getTrans << "AND NUM_OPEN > 0 ";
    getTrans << "ORDER BY LIMITED DESC, OPEN_TIME ASC LIMIT 1 ";
    getTrans << "FOR UPDATE;";
  }

  /* Execute SQL query */
  result rst(W.exec(getTrans.str()));

  if (rst.size() == 0) {
    return false;
  }

  int other_trans_id = rst[0]["TRANSACTION_ID"].as<int>();
  string other_account_id = rst[0]["ACCOUNT_ID"].as<string>();
  string other_symbol_name = rst[0]["SYMBOL_NAME"].as<string>();
  double other_limited = rst[0]["LIMITED"].as<double>();
  int other_num_open = rst[0]["NUM_OPEN"].as<int>();

  // Get final price
  final_price = other_limited;

  // W.commit();

  if (isBuyer) {                           // num_open: 100
    if (num_open <= 0 - other_num_open) {  // other_num_open: -150, completed matching
      final_amount = num_open;             // 100
      // Update symbol amount
      Transaction::setOpenSharesByWork(W, trans_id, 0);
      Transaction::setOpenSharesByWork(W, other_trans_id, other_num_open + final_amount);  // -50
    }
    else {                                // other_num_open: -30, partial matchin
      final_amount = 0 - other_num_open;  // 30
                                          // Update symbol amount
      Transaction::setOpenSharesByWork(W, trans_id, num_open - final_amount);  // 70
      Transaction::setOpenSharesByWork(W, other_trans_id, 0);
    }

    // Return money to buyer
    Account::addBalance(W, account_id, final_amount * (limited - other_limited));

    // Give symbol to buyer
    Position::addSymbolAmount(W, account_id, symbol_name, final_amount);

    // Give money to seller
    Account::addBalance(W, other_account_id, final_amount * final_price);

    // Create execution
    Execution::addExecutionByWork(W, trans_id, other_trans_id, final_amount, final_price);
  }
  else {                                   // num_open: -100
    if (0 - num_open <= other_num_open) {  // other_num_open: 150, completed matching
      final_amount = -num_open;            // 100
      // Update symbol amount
      Transaction::setOpenSharesByWork(W, trans_id, 0);
      Transaction::setOpenSharesByWork(W, other_trans_id, other_num_open - final_amount);  // 50
    }
    else {                            // other_num_open: 30, partial matchin
      final_amount = other_num_open;  // 30
                                      // Update symbol amount
      Transaction::setOpenSharesByWork(W, trans_id, num_open + final_amount);  // -70
      Transaction::setOpenSharesByWork(W, other_trans_id, 0);
    }
    // Return money to buyer: nothing

    // Give symbol to buyer
    Position::addSymbolAmount(W, other_account_id, symbol_name, final_amount);

    // Give money to seller
    Account::addBalance(W, account_id, final_amount * final_price);

    // Create execution
    Execution::addExecutionByWork(W, other_trans_id, trans_id, final_amount, final_price);
  }

  int remain = Transaction::getOpenSharesByWork(W, trans_id);

  W.commit();

  if (remain == 0) {
    return false;
  }
  return true;
}

const std::string Transaction::doQuery(connection * C, int trans_id) {
  std::stringstream response;

  work W(*C);

  // Lock the transaction and execution table
  std::stringstream sql1;
  sql1 << "SELECT * FROM TRANSACTION, EXECUTION ";
  sql1 << "WHERE TRANSACTION.TRANSACTION_ID = " << W.quote(trans_id);
  sql1 << " AND (EXECUTION.BUYER_TRANS_ID = " << W.quote(trans_id);
  sql1 << " OR EXECUTION.SELLER_TRANS_ID = " << W.quote(trans_id);
  sql1 << ") FOR UPDATE;";
  W.exec(sql1.str());

  // Get open and canceled shares
  std::stringstream sql2;
  sql2 << "SELECT NUM_OPEN, NUM_CANCELED, CANCEL_TIME FROM TRANSACTION WHERE TRANSACTION_ID=";
  sql2 << W.quote(trans_id) << ";";

  result R1(W.exec(sql2.str()));

  int openShares = R1[0]["NUM_OPEN"].as<int>();
  int canceledShares = R1[0]["NUM_CANCELED"].as<int>();
  long canceledTime = R1[0]["CANCEL_TIME"].as<long>();

  // Get response for open shares
  if (openShares != 0) {  // !!!!!!!!!!! buy & sell
    response << "    <open shares=\"" << openShares << "\"/>\n";
  }

  // Get response for cancel shares
  if (canceledShares != 0) {
    response << "    <canceled ";
    response << "shares=\"" << canceledShares << "\" ";
    response << "time=\"" << canceledTime << "\"/>\n";
  }

  // Get response for execution
  std::stringstream sql3;
  sql3 << "SELECT BUYER_TRANS_ID, SELLER_TRANS_ID, AMOUNT, PRICE, TIME FROM EXECUTION WHERE ";
  sql3 << "BUYER_TRANS_ID = " << W.quote(trans_id) << " ";
  sql3 << "OR SELLER_TRANS_ID = " << W.quote(trans_id) << ";";

  /* Execute SQL query */
  result R2(W.exec(sql3.str()));

  for (result::const_iterator c = R2.begin(); c != R2.end(); ++c) {
    response << "    <executed shares=\"";
    if (trans_id == c["BUYER_TRANS_ID"].as<int>()) {
      response << c["AMOUNT"].as<int>() << "\" ";
    }
    else {
      response << 0 - c["AMOUNT"].as<int>() << "\" ";
    }
    response << "price=\"" << c["PRICE"].as<double>() << "\" ";
    response << "time=\"" << c["TIME"].as<long>() << "\"/>\n";
  }

  W.commit();

  return response.str();
}
