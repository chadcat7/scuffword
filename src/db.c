#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

sqlite3 *DB;
static const char *DB_FILE = "./private/data.db";

struct user {
  unsigned long long int id;
  char *name;
  int level;
};

void seed_db() {
  char *err_msg = 0;
  sqlite3_open(DB_FILE, &DB);
  int rc = sqlite3_exec(DB,
                        "CREATE TABLE IF NOT EXISTS users (id BIGINT PRIMARY "
                        "KEY, name TEXT, level INTEGER);",
                        0, 0, &err_msg);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to create table: %s\n", err_msg);
    sqlite3_free(err_msg);
  }
}

// get profile by id
struct user *get_profile(int id) {
  sqlite3_stmt *stmt;
  struct user *result = NULL;

  sqlite3_open(DB_FILE, &DB);
  int rc = sqlite3_prepare_v2(
      DB, "SELECT id, name, level FROM users WHERE id = ?", -1, &stmt, NULL);

  sqlite3_bind_int(stmt, 1, id);
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) {
    // If the query returns a row, allocate memory for the result
    result = (struct user *)malloc(sizeof(struct user));
    if (result == NULL) {
      fprintf(stderr, "Failed to allocate memory for user\n");
      sqlite3_finalize(stmt); // Clean up the statement
      return NULL;
    }

    // Retrieve data from the result set
    result->id = sqlite3_column_int(stmt, 0);
    result->name = strdup(
        (const char *)sqlite3_column_text(stmt, 1)); // Allocate memory for name
    result->level = sqlite3_column_int(stmt, 2);
  } else {
    // make profile if not found
  }

  // Finalize the statement
  sqlite3_finalize(stmt);

  return result;
}

// create profile for the user
struct user *create_user(int id, char *username) {
  sqlite3_stmt *stmt;
  struct user *result = NULL;

  sqlite3_open(DB_FILE, &DB);
  int rc = sqlite3_prepare_v2(DB, "INSERT INTO users VALUES(?, ?, 0)", -1,
                              &stmt, NULL);

  sqlite3_bind_int(stmt, 1, id);
  sqlite3_bind_text(stmt, 2, username, -1, SQLITE_STATIC);
  rc = sqlite3_step(stmt);

  if (rc != SQLITE_DONE) {
    fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(DB));
    sqlite3_finalize(stmt);
    sqlite3_close(DB);
    return NULL;
  }

  sqlite3_finalize(stmt);
  sqlite3_close(DB);
  result = malloc(sizeof(struct user));
  result->id = id;
  result->name = strdup(username);
  result->level = 0;

  return result;
}