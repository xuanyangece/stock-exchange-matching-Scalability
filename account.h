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
  /* The fields of this table:
     
     string account_id;
     double balance;
  */

 public:
  /* Create the table without foreign keys */
  static void createTable(connection * C);

  /* Build foreign keys */
  static void buildForeignKeys(connection * C);

  /* Add a new entry to the table, return true if succeed */
  static bool addAccount(connection * C, const string & account_id, double balance);

  /* Check if the given account already exists */
  static bool isAccountExists(connection * C, const string & account_id);

  /* Get the balance of the given account_id */
  static double getBalance(connection * C, const string & account_id);

  /* Set the balance of the given account_id */
  static void setBalance(connection * C, const string & account_id, double balance);

  /* Reduce the balance of the given account_id atomically */
  static bool reduceBalance(connection * C, const string & account_id, double payment);

  /* Add the balance of the given account_id atomically */
  static void addBalance(work & W, const string & account_id, double payment);
};

#endif
