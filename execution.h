#ifndef EXECUTION_H
#define EXECUTION_H

#include <pqxx/pqxx>
#include <string>

#include "functions.h"
#include "table.h"

using std::string;
using namespace pqxx;

class Execution
{
 private:
  // int execution_id;
  // int buyer_trans_id;
  // int seller_trans_id;
  // int amount;
  // double price;
  // long time;

 public:
  static void createTable(connection * C);

  static void buildForeignKeys(connection * C);

  static void addExecution(connection * C,
                           int buyer_trans_id,
                           int seller_trans_id,
                           int amount,
                           double price);
};

#endif
