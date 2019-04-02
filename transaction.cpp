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
  /* Create a non-transactional object. */
  nontransaction N(*C);

  /* Create SQL statement: get info of trans_id */
  std::stringstream getinfo;
  getinfo << "SELECT * FROM TRANSACTION WHERE TRANSACTION_ID=";
  getinfo << N.quote(trans_id) << ";";

  result R(N.exec(getinfo.str()));

  string account_id = R[0]["ACCOUNT_ID"].as<string>();
  string symbol_name = R[0]["SYMBOL_NAME"].as<string>();
  double limited = R[0]["LIMITED"].as<double>();
  int num_open = R[0]["NUM_OPEN"].as<int>();

  /* Create SQL statement */
  std::stringstream getTrans;
  getTrans << "SELECT * FROM TRANSACTION WHERE ACCOUNT_ID!=";
  getTrans << N.quote(account_id) << " ";
  getTrans << "AND SYMBOL_NAME=" << N.quote(symbol_name) << " ";

  bool isBuyer = (num_open > 0);
  int final_amount;
  double final_price;

  if (isBuyer) {
    getTrans << "AND LIMITED<=" << N.quote(limited) << " ";
    getTrans << "AND NUM_OPEN<0 ";
    getTrans << "ORDER BY LIMITED ASC, OPEN_TIME ASC LIMIT 1;";
  }
  else {
    getTrans << "AND LIMITED>=" << N.quote(limited) << " ";
    getTrans << "AND NUM_OPEN>0 ";
    getTrans << "ORDER BY LIMITED DESC, OPEN_TIME ASC LIMIT 1;";
  }

  /* Execute SQL query */
  result rst(N.exec(getTrans.str()));

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

  N.commit();

  if (isBuyer) {                           // num_open: 100
    if (num_open <= 0 - other_num_open) {  // other_num_open: -150, completed matching
      final_amount = num_open;             // 100
      // Update symbol amount
      Transaction::setOpenShares(C, trans_id, 0);
      Transaction::setOpenShares(C, other_trans_id, other_num_open + final_amount);  // -50
    }
    else {                                // other_num_open: -30, partial matchin
      final_amount = 0 - other_num_open;  // 30
                                          // Update symbol amount
      Transaction::setOpenShares(C, trans_id, num_open - final_amount);  // 70
      Transaction::setOpenShares(C, other_trans_id, 0);
    }

    // Return money to buyer
    double oldBalance = Account::getBalance(C, account_id);
    Account::setBalance(C, account_id, oldBalance + final_amount * (limited - other_limited));

    // Give symbol to buyer
    if (Position::isSymbolExists(C, account_id, symbol_name)) {
      int oldNum = Position::getSymbolAmount(C, account_id, symbol_name);
      Position::setSymbolAmount(C, account_id, symbol_name, oldNum + final_amount);
    }
    else {
      Position::addPosition(C, symbol_name, account_id, final_amount);
    }

    // Give money to seller
    oldBalance = Account::getBalance(C, other_account_id);
    Account::setBalance(C, other_account_id, oldBalance + final_amount * final_price);

    // Create execution
    Execution::addExecution(C, trans_id, other_trans_id, final_amount, final_price);
  }
  else {                                   // num_open: -100
    if (0 - num_open <= other_num_open) {  // other_num_open: 150, completed matching
      final_amount = -num_open;            // 100
      // Update symbol amount
      Transaction::setOpenShares(C, trans_id, 0);
      Transaction::setOpenShares(C, other_trans_id, other_num_open - final_amount);  // 50
    }
    else {                            // other_num_open: 30, partial matchin
      final_amount = other_num_open;  // 30
                                      // Update symbol amount
      Transaction::setOpenShares(C, trans_id, num_open + final_amount);  // -70
      Transaction::setOpenShares(C, other_trans_id, 0);
    }
    // Return money to buyer: nothing

    // Give symbol to buyer
    if (Position::isSymbolExists(C, other_account_id, symbol_name)) {
      int oldNum = Position::getSymbolAmount(C, other_account_id, symbol_name);
      Position::setSymbolAmount(C, other_account_id, symbol_name, oldNum + final_amount);
    }
    else {
      Position::addPosition(C, symbol_name, other_account_id, final_amount);
    }

    // Give money to seller
    double oldBalance = Account::getBalance(C, account_id);
    Account::setBalance(C, account_id, oldBalance + final_amount * final_price);

    // Create execution
    Execution::addExecution(C, other_trans_id, trans_id, final_amount, final_price);
  }

  int remain = Transaction::getOpenShares(C, trans_id);
  if (remain == 0) {
    return false;
  }
  return true;
}
