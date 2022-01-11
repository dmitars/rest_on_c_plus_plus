//
// Created by dmitars on 8.01.22.
//

#ifndef LAB43_SQLITE_MANAGER_H
#define LAB43_SQLITE_MANAGER_H

#include<bits/stdc++.h>
#include <sqlite3.h>


class sqlite_manager {
private:
    sqlite3* db;

    void init_database();

    void checked_execute(const char *sql);

    void close_database();

public:
    sqlite_manager(){
        db = nullptr;
        sqlite_manager::init_database();
    }

    long long int insert_doc(const std::string &word);

    std::string get_doc_by_id(int id);

    std::vector<std::string> get_docs();

    void delete_by_id(int id);

    void update_by_id(int id, const std::string &word);

    ~sqlite_manager(){
        sqlite_manager::close_database();
    }
};
#endif //LAB43_SQLITE_MANAGER_H
