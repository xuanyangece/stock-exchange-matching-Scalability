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
  // string symbol_name;
  // string account_id;
  // int num_share;

 public:
  static void createTable(connection * C);

  static void buildForeignKeys(connection * C);

  static void addPosition(connection * C,
                          const string & symbol_name,
                          const string & account_id,
                          int num_share);

  static bool isSymbolExists(connection * C, const string & account_id, const string & symbol_name);

  static int getSymbolAmount(connection * C, const string & account_id, const string & symbol_name);

  static void setSymbolAmount(connection * C,
                              const string & account_id,
                              const string & symbol_name,
                              int amount);

  static bool reduceSymbolAmount(connection * C,
                                 const string & account_id,
                                 const string & symbol_name,
                                 int requiredAmount);

  static void addSymbolAmount(work & W,
                              const string & account_id,
                              const string & symbol_name,
                              int requiredAmount);
};

#endif
