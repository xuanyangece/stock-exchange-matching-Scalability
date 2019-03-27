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
  int transaction_id;
  int account_id;
  string symbol_name;
  double limited;
  int num_open;
  // int num_executed;
  int num_canceled;
  long open_time;
  long cancel_time;

 public:
  static void createTable(connection * C);
  static void buildForeignKeys(connection * C);
  static int addTransaction(connection * C,
                            int _account_id,
                            const string & _symbol_name,
                            double _limited,
                            int _num_open);
  // 更新买家的钱，卖家的 amount position

  static bool tryMatch(connection * C, int trans_id);
  /*
    获得所有订单并排序
    检测匹配
    若匹配：确定价格，尽可能多地交易，更新 balance 和 position amount
    更新 executed 和 transaction 表
    若不匹配，结束
   */

  static bool isTransExists(connection * C, int trans_id);

  static bool isTransCompleted(connection * C, int trans_id);

  static bool isTransCanceled(connection * C, int trans_id);

  // static void queryTransaction();

  static void cancelTransaction(connection * C, int trans_id);

  static long getCanceledTime(connection * C, int trans_id);

  static int getAccountID(connection * C, int trans_id);

  static double getLimited(connection * C, int trans_id);

  static string getSymbolName(connection * C, int trans_id);

  static int getCanceledShares(connection * C, int trans_id);

  static int getOpenShares(connection * C, int trans_id);

  static void setOpenShares(connection * C, int trans_id, int amount);

  static void setCanceledShares(connection * C, int trans_id, int amount);

  static void setCanceledTime(connection * C, int trans_id, long time);

  static const std::string queryExecuted(connection * C, int trans_id);
};

#endif
