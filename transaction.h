#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <pqxx/pqxx>
#include <string>

#include "execution.h"
#include "functions.h"
#include "table.h"

using std::string;
using namespace pqxx;

class Transaction
{
 private:
  // int transaction_id;
  // string account_id;
  // string symbol_name;
  // double limited;
  // int num_open;
  // int num_executed;
  // int num_canceled;
  // long open_time;
  // long cancel_time;

 public:
  static void createTable(connection * C);
  static void buildForeignKeys(connection * C);
  static int addTransaction(connection * C,
                            const string & account_id,
                            const string & symbol_name,
                            double limited,
                            int num_open);

  static bool tryMatch(connection * C, int trans_id);

  static bool isTransExists(connection * C, int trans_id);

  static bool isTransCompleted(connection * C, int trans_id);

  static bool isTransCanceled(connection * C, int trans_id);

  static bool cancelTransaction(connection * C, int trans_id);

  static long getCanceledTime(connection * C, int trans_id);

  static string getAccountID(connection * C, int trans_id);

  static double getLimited(connection * C, int trans_id);

  static string getSymbolName(connection * C, int trans_id);

  static int getCanceledShares(connection * C, int trans_id);

  static int getOpenShares(connection * C, int trans_id);

  static void setOpenShares(connection * C, int trans_id, int amount);

  static void setCanceledShares(connection * C, int trans_id, int amount);

  static void setCanceledTime(connection * C, int trans_id, long time);

  static const std::string queryExecuted(connection * C, int trans_id);

  static const std::string doQuery(connection * C, int trans_id);
};

#endif
