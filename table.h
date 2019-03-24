#ifndef TABLE_H
#define TABLE_H

#include <pqxx/pqxx>
#include <string>

using std::string;
using namespace pqxx;

class Table
{
 public:
  /* Create table in the database connected by W
     If it already exists, delete it first */
  static void createTable(connection * C,
                          const string & dropExistingTableSql,
                          string & createTableSql) {
    /* Create a transactional object. */
    work W(*C);

    // Drop existing table
    W.exec(dropExistingTableSql);

    // Create new table
    W.exec(createTableSql);

    W.commit();
  }

  /* Build foreign keys */
  static void buildForeignKeys(connection * C, const string & buildForeignKeysSql) {
    /* Create a transactional object. */
    work W(*C);

    W.exec(buildForeignKeysSql);
    W.commit();
  }
};

#endif
