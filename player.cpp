#include "player.h"

#include <fstream>
#include <sstream>
#include <string>

using std::string;

int Player::player_id = 1;

/* Default constructor */
Player::Player() {
  dropExistingTableSql = "DROP TABLE IF EXISTS PLAYER;";

  // PLAYER (PLAYER_ID, TEAM_ID, UNIFORM_NUM, FIRST_NAME,
  // LAST_NAME, MPG, PPG,RPG, APG, SPG, BPG)
  // 1 1 1 Jerome Robinson 34 19 4 3 1.7 0.4
  createTableSql = "CREATE TABLE PLAYER("
                   "PLAYER_ID       INT     PRIMARY KEY NOT NULL,"
                   "TEAM_ID         INT     NOT NULL,"
                   "UNIFORM_NUM     INT     NOT NULL,"
                   "FIRST_NAME      TEXT    NOT NULL,"
                   "LAST_NAME       TEXT    NOT NULL,"
                   "MPG             INT     NOT NULL,"
                   "PPG             INT     NOT NULL,"
                   "RPG             INT     NOT NULL,"
                   "APG             INT     NOT NULL,"
                   "SPG             REAL    NOT NULL,"
                   "BPG             REAL    NOT NULL);";

  // Build foreign keys
  buildForeignKeysSql =
      "ALTER TABLE PLAYER "
      "ADD CONSTRAINT PLAYER_TEAM_ID_FKEY FOREIGN KEY (TEAM_ID) REFERENCES TEAM(TEAM_ID);";
}

/* Load table from file: each line is an entey */
void Player::loadTableFromFile(connection * C, const string & fileName) {
  std::ifstream is(fileName);
  string line;

  while (std::getline(is, line)) {
    std::stringstream ss(line);
    ss >> player_id;
    ss >> team_id;
    ss >> uniform_num;
    ss >> first_name;
    ss >> last_name;
    ss >> mpg;
    ss >> ppg;
    ss >> rpg;
    ss >> apg;
    ss >> spg;
    ss >> bpg;

    addEntry(C);
  }

  is.close();
}

/* Add a new entry to the table */
void Player::addEntry(connection * C) {
  /* Create a transactional object. */
  work W(*C);

  /* Create SQL statement */
  std::stringstream sql;
  sql << "Insert INTO PLAYER (PLAYER_ID, TEAM_ID, UNIFORM_NUM, ";
  sql << "FIRST_NAME, LAST_NAME, MPG, PPG, RPG, APG, SPG, BPG) ";
  sql << "VALUES (";
  sql << W.quote(player_id) << ", ";
  sql << W.quote(team_id) << ", ";
  sql << W.quote(uniform_num) << ", ";
  sql << W.quote(first_name) << ", ";
  sql << W.quote(last_name) << ", ";
  sql << W.quote(mpg) << ", ";
  sql << W.quote(ppg) << ", ";
  sql << W.quote(rpg) << ", ";
  sql << W.quote(apg) << ", ";
  sql << W.quote(spg) << ", ";
  sql << W.quote(bpg) << ");";

  W.exec(sql.str());
  W.commit();
}
