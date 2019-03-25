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

  static void addAccount(connection * C, int _account_id, double _balance);

  static bool isAccountExists(connection * C, int _account_id);

  static double getBalance(connection * C, int _account_id);

  static void setBalance(connection * C, int _account_id, double _balance);
};

#endif
