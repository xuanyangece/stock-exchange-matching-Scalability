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
  double limit;
  int num_open;
  int num_executed;
  int num_canceled;

 public:
  static void createTable(connection * C);
  static void buildForeignKeys(connection * C);
  static void addEntry(connection * C,
                       int _account_id,
                       const string & _symbol_name,
                       double _limited,
                       int _num_open,
                       int _num_executed,
                       int _num_canceled);
};

#endif
