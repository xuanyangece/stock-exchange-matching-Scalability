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
  /* The fields of this table:
     
     int transaction_id;
     string account_id;
     string symbol_name;
     double limited;
     int num_open;
     int num_executed;
     int num_canceled;
     long open_time;
     long cancel_time;
  */

 public:
  /* Create the table without foreign keys */
  static void createTable(connection * C);

  /* Build foreign keys */
  static void buildForeignKeys(connection * C);

  /* Add a new entry to the table, return the transaction_id */
  static int addTransaction(connection * C,
                            const string & account_id,
                            const string & symbol_name,
                            double limited,
                            int num_open);

  /* Try to match a transaction,
     return true if another matching can still be achieved */
  static bool tryMatch(connection * C, int trans_id);

  /* Check if the given transaction already exists */
  static bool isTransExists(connection * C, int trans_id);

  /* Check if the given transaction is completed: matched/canceled */
  static bool isTransCompleted(connection * C, int trans_id);

  /* Check if the given transaction is canceled */
  static bool isTransCanceled(connection * C, int trans_id);

  /* Cancel a transaction, return true if succeed */
  static bool cancelTransaction(connection * C, int trans_id);

  /* Get the cancel_time of the given trans_id */
  static long getCanceledTime(connection * C, int trans_id);

  /* Get the account_id of the given trans_id */
  static string getAccountID(connection * C, int trans_id);

  /* Get the limited price of the given trans_id */
  static double getLimited(connection * C, int trans_id);

  /* Get the symbol_name of the given trans_id */
  static string getSymbolName(connection * C, int trans_id);

  /* Get the number of canceled shares of the given trans_id */
  static int getCanceledShares(connection * C, int trans_id);

  /* Get the number of open shares of the given trans_id */
  static int getOpenShares(connection * C, int trans_id);

  /* Get the number of canceled shares of the given trans_id
     The parameter is work &, so it's part of another transaction */
  static int getOpenSharesByWork(work & W, int trans_id);

  /* Set the number of open shares of the given trans_id */
  static void setOpenShares(connection * C, int trans_id, int amount);

  /* Set the number of open shares of the given trans_id        
     The parameter is work &, so it's part of another transaction */
  static void setOpenSharesByWork(work & W, int trans_id, int amount);

  /* Set the number of canceled shares of the given trans_id */
  static void setCanceledShares(connection * C, int trans_id, int amount);

  /* Get the canceled time of the given trans_id */
  static void setCanceledTime(connection * C, int trans_id, long time);

  /* Query the executions associated with the given transaction */
  static const std::string queryExecuted(connection * C, int trans_id);

  /* Query the given transaction, return the response xml */
  static const std::string doQuery(connection * C, int trans_id);
};

#endif
