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
  // string account_id;
  // double balance;

 public:
  static void createTable(connection * C);

  static void buildForeignKeys(connection * C);

  static void addAccount(connection * C, string account_id, double balance);

  static bool isAccountExists(connection * C, string account_id);

  static double getBalance(connection * C, string account_id);

  static void setBalance(connection * C, string account_id, double balance);
};

#endif
