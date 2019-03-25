#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <pqxx/pqxx>
#include <string>

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
  int num_executed;
  int num_canceled;

 public:
  static void createTable(connection * C);
  static void buildForeignKeys(connection * C);
  static void addTransaction(connection * C,
                             int _account_id,
                             const string & _symbol_name,
                             double _limited,
                             int _num_open);
  // 更新买家的钱，卖家的 amount position

  static void trtMatch();
  /*
    获得所有订单并排序
    检测匹配
    若匹配：确定价格，尽可能多地交易，更新 balance 和 position amount
    更新 executed 和 transaction 表
    若不匹配，结束
   */

  static void matchExecution();

  static void queryTransaction();

  static void cancelTransaction();
};

#endif
