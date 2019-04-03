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
  /* The fields of this table:
     
     int execution_id;
     int buyer_trans_id;
     int seller_trans_id;
     int amount;
     double price;
     long time;
  */

 public:
  /* Create the table without foreign keys */
  static void createTable(connection * C);

  /* Build foreign keys */
  static void buildForeignKeys(connection * C);

  /* Add a new entry to the table */
  static void addExecution(connection * C,
                           int buyer_trans_id,
                           int seller_trans_id,
                           int amount,
                           double price);

  /* Add a new entry to the table
     The parameter is work &, so it's part of another transaction */
  static void addExecutionByWork(work & W,
                                 int buyer_trans_id,
                                 int seller_trans_id,
                                 int amount,
                                 double price);
};

#endif
