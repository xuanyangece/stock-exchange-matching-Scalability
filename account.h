#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <pqxx/pqxx>
#include <string>

#include "table.h"

using std::string;
using namespace pqxx;

class Account
{
 private:
  int account_id;
  double balance;

 public:
  static void createTable(connection * C);
  static void buildForeignKeys(connection * C);
  static void addEntry(connection * C, int _account_id, double _balance);
};

#endif
