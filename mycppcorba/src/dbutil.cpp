#include "dbutil.hpp"
#include <iostream>


MYSQL* connectToDatabase(const DbConfig& cfg) {
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "mysql_init a échoué" << std::endl;
        return nullptr;
    }

    if (!mysql_real_connect(conn,
                            cfg.host.c_str(),
                            cfg.user.c_str(),
                            cfg.password.c_str(),
                            cfg.database.c_str(),
                            cfg.port,
                            nullptr, 0)) {
        std::cerr << "Erreur de connexion : " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return nullptr;
    }

    return conn;
}


bool executeQuery(MYSQL* conn, const std::string& query) {
    if (mysql_query(conn, query.c_str())) {
        std::cerr << "Erreur de requête : " << mysql_error(conn) << std::endl;
        return false;
    }
    return true;
}

void displayEtudiant(MYSQL* conn) {
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        std::cerr << "mysql_store_result a échoué : " << mysql_error(conn) << std::endl;
        return;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res)) != nullptr) {
          std::cout << "[" << row[0] << "] "
                  << (row[2] ? row[2] : "?") << " "   
                  << (row[1] ? row[1] : "?")          
                  << " (" << (row[3] ? row[3] : "-") << ")" << std::endl;
    }

    mysql_free_result(res);
}
