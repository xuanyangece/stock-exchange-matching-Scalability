#ifndef PLAYER_H
#define PLAYER_H

#include <pqxx/pqxx>
#include <string>

#include "table.h"

using std::string;
using namespace pqxx;

class Player : public Table
{
 private:
  static int player_id;

  int team_id;
  int uniform_num;
  string first_name;
  string last_name;
  int mpg;
  int ppg;
  int rpg;
  int apg;
  double spg;
  double bpg;

 public:
  explicit Player();

  void loadTableFromFile(connection * C, const string & fileName);

  void addEntry(connection * C);
};

// int Player::player_id = 1;

#endif
