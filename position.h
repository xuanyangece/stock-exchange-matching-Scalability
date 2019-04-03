#ifndef POSITION_H
#define POSITION_H

#include <pqxx/pqxx>
#include <string>

#include "table.h"

using std::string;
using namespace pqxx;

class Position
{
 private:
  /* The fields of this table:
     
     string symbol_name;
     string account_id;
     int num_share;
  */

 public:
  /* Create the table without foreign keys */
  static void createTable(connection * C);

  /* Build foreign keys */
  static void buildForeignKeys(connection * C);

  /* Add a new entry to the table */
  static void addPosition(connection * C,
                          const string & symbol_name,
                          const string & account_id,
                          int num_share);

  /* Check if the given symbol already exists */
  static bool isSymbolExists(connection * C, const string & account_id, const string & symbol_name);

  /* Get the symbol amount of the given account_id and symbol_name */
  static int getSymbolAmount(connection * C, const string & account_id, const string & symbol_name);

  /* Set the symbol amount of the given account_id and symbol_name */
  static void setSymbolAmount(connection * C,
                              const string & account_id,
                              const string & symbol_name,
                              int amount);

  /* Reduce the symbol amount of the given position atomically */
  static bool reduceSymbolAmount(connection * C,
                                 const string & account_id,
                                 const string & symbol_name,
                                 int requiredAmount);

  /* Add the symbol amount of the given position atomically */
  static void addSymbolAmount(work & W,
                              const string & account_id,
                              const string & symbol_name,
                              int requiredAmount);
};

#endif
