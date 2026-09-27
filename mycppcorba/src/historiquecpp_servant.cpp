#include "historiquecpp_servant.hpp"
#include "dbutil.hpp"
#include <mysql/mysql.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>
#include <etudiant.h>

EtudiantApp::HistoriqueCppList* HistoriqueCppServant::getAllHistorique() {
    EtudiantApp::HistoriqueCppList* list = new EtudiantApp::HistoriqueCppList();
    DbConfig cfg;
    MYSQL* conn = connectToDatabase(cfg);
    if (!conn) { list->length(0); return list; }
    if (mysql_query(conn, "SELECT id, action, details, created_at FROM historique ORDER BY created_at DESC")) {
        mysql_close(conn);
        list->length(0);
        return list;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while (res && (row = mysql_fetch_row(res))) {
        EtudiantApp::HistoriqueCpp h;
        h.id         = row[0] ? std::atoi(row[0]) : 0;
        h.action     = CORBA::string_dup(row[1] ? row[1] : "");
        h.details    = CORBA::string_dup(row[2] ? row[2] : "");
        h.created_at = row[3] ? std::atoll(row[3]) : 0;

        CORBA::ULong idx = list->length();
        list->length(idx + 1);
        (*list)[idx] = h;
    }

    if (res) mysql_free_result(res);
    mysql_close(conn);
    return list;
}