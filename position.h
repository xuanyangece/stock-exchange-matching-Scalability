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
  int position_id;
  string symbol_name;
  int account_id;
  int num_share;

 public:
  static void createTable(connection * C);

  static void buildForeignKeys(connection * C);

  static void addPosition(connection * C,
                          const string & _symbol_name,
                          int _account_id,
                          int _num_share);

  static bool isSymbolExists(connection * C, int _account_id, const string & _symbol_name);

  static int getSymbolAmount(connection * C, int _account_id, const string & _symbol_name);

  static void setSymbolAmount(connection * C,
                              int _account_id,
                              const string & _symbol_name,
                              int amount);
};

#endif
