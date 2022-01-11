//
// Created by dmitars on 8.01.22.
//

#include "sqlite_manager.h"

using namespace std;

void sqlite_manager::init_database() {
    int ret;
    if (SQLITE_OK != (ret = sqlite3_initialize())) {
        printf("Failed to initialize library: %d\n", ret);
        return;
    }

    if (SQLITE_OK !=
        (ret = sqlite3_open_v2("documents.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr))) {
        printf("Failed to open conn: %d\n", ret);
        close_database();
        return;
    }

    auto sql = "create table if not exists docs(id integer PRIMARY KEY autoincrement NOT NULL, doc TEXT NOT NULL);";

    if (SQLITE_OK != (ret = sqlite3_exec(db, sql, nullptr, nullptr, nullptr))) {
        printf("Failed to create table: %d\n", ret);
        close_database();
        return;
    }
}


long long int sqlite_manager::insert_doc(const std::string &doc) {
    auto sql = "insert into docs(doc) values('" + doc + "');";
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
    return sqlite3_last_insert_rowid(db);
}


void sqlite_manager::update_by_id(int id, const std::string &doc) {
    auto sql = "update docs set doc = '"+doc+"' where id = "+std::to_string(id)+";";
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
}

void sqlite_manager::delete_by_id(int id) {
    auto sql = "delete from docs where id = "+std::to_string(id)+";";
    sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
}

void sqlite_manager::checked_execute(const char *sql) {
    int ret;
    if (SQLITE_OK != (ret = sqlite3_exec(db, sql, nullptr, nullptr, nullptr))) {
        printf("Failed checked_execute: %d %s\n", ret, sql);
    }
}


string sqlite_manager::get_doc_by_id(int id) {
    sqlite3_stmt *stmt;
    auto sql = "SELECT doc from docs where id = " + std::to_string(id) + ";";
    if (SQLITE_OK != (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr))) {
        throw runtime_error("Failed to prepare select of doc");
    }

    if (SQLITE_ROW != (sqlite3_step(stmt))) {
        return std::string();
    }

    auto result = sqlite3_column_text(stmt, 0);
    auto result_str = string(reinterpret_cast<const char *>(result));
    sqlite3_finalize(stmt);
    return result_str;
}

std::vector<std::string> sqlite_manager::get_docs() {
    sqlite3_stmt *stmt;
    std::vector<std::string> result;
    auto sql = "SELECT doc from docs;";
    if (SQLITE_OK != (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr))) {
        throw runtime_error("Failed to prepare select of doc");
    }

    while (SQLITE_ROW == (sqlite3_step(stmt))) {
        result.emplace_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
    }
    sqlite3_finalize(stmt);
    return result;
}


void sqlite_manager::close_database() {
    if (db != nullptr) sqlite3_close(db);
}