#include "note_servant.hpp"
#include "dbutil.hpp"
#include <mysql/mysql.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>

static std::string escape(MYSQL* conn, const char* s) {
    if (!s) return "";
    std::string out;
    out.resize(std::strlen(s) * 2 + 1);
    unsigned long len = mysql_real_escape_string(conn, &out[0], s, std::strlen(s));
    out.resize(len);
    return out;
}

static std::string historize(MYSQL* conn,
                             const std::string& action,
                             const std::string& details) {
    std::string q = "INSERT INTO historique (action, details) VALUES ('"
                  + escape(conn, action.c_str()) + "', '"
                  + escape(conn, details.c_str()) + "')";
    if (mysql_query(conn, q.c_str())) {
        std::cerr << "Erreur INSERT historique : " << mysql_error(conn) << std::endl;
        return "";
    }
    return std::to_string(mysql_insert_id(conn));
}

static EtudiantApp::Note convertRowToNote(MYSQL_ROW row) {
    EtudiantApp::Note n;
    n.numEtu  = CORBA::string_dup(row[0] ? row[0] : "");
    n.matiere = CORBA::string_dup(row[1] ? row[1] : "");
    n.valeur  = row[2] ? std::atof(row[2]) : 0.0;
    return n;
}

EtudiantApp::NoteList* NoteServant::getAllNotes() {
    EtudiantApp::NoteList* list = new EtudiantApp::NoteList();
    DbConfig cfg;
    MYSQL* conn = connectToDatabase(cfg);
    if (!conn) { list->length(0); return list; }

    historize(conn, "getAllNotes", "Récupération de toutes les notes");

    if (mysql_query(conn, "SELECT num_etu, matiere, valeur FROM note")) {
        mysql_close(conn);
        list->length(0);
        return list;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while (res && (row = mysql_fetch_row(res))) {
        EtudiantApp::Note n = convertRowToNote(row);
        CORBA::ULong idx = list->length();
        list->length(idx + 1);
        (*list)[idx] = n;
    }
    if (res) mysql_free_result(res);
    mysql_close(conn);
    return list;
}

EtudiantApp::NoteList* NoteServant::getNotesByEtu(const char* numEtu) {
    EtudiantApp::NoteList* list = new EtudiantApp::NoteList();
    DbConfig cfg;
    MYSQL* conn = connectToDatabase(cfg);
    if (!conn) { list->length(0); return list; }

    historize(conn, "getNotesByEtu", std::string("numEtu=") + numEtu);

    std::string q = "SELECT num_etu, matiere, valeur FROM note "
                    "WHERE num_etu = '" + escape(conn, numEtu) + "'";
    if (mysql_query(conn, q.c_str())) { mysql_close(conn); list->length(0); return list; }

    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while (res && (row = mysql_fetch_row(res))) {
        EtudiantApp::Note n = convertRowToNote(row);
        CORBA::ULong idx = list->length();
        list->length(idx + 1);
        (*list)[idx] = n;
    }
    if (res) mysql_free_result(res);
    mysql_close(conn);
    return list;
}

void NoteServant::addNote(const EtudiantApp::Note& n) {
    DbConfig cfg;
    MYSQL* conn = connectToDatabase(cfg);
    if (!conn) return;

    historize(conn, "addNote",
              std::string("numEtu=") + std::string(n.numEtu)
            + ", matiere=" + std::string(n.matiere)
            + ", valeur="  + std::to_string(n.valeur));

    std::string q = "INSERT INTO note (num_etu, matiere, valeur) VALUES ('"
                  + escape(conn, n.numEtu)  + "', '"
                  + escape(conn, n.matiere) + "', "
                  + std::to_string(n.valeur) + ")";
    if (mysql_query(conn, q.c_str())) {
        std::cerr << "Erreur INSERT note : " << mysql_error(conn) << std::endl;
    }
    mysql_close(conn);
}

CORBA::Double NoteServant::getMoyenne(const char* numEtu) {
    DbConfig cfg;
    MYSQL* conn = connectToDatabase(cfg);
    if (!conn) return 0.0;

    historize(conn, "getMoyenne", std::string("numEtu=") + numEtu);

    std::string q = "SELECT AVG(valeur) FROM note WHERE num_etu = '"
                  + escape(conn, numEtu) + "'";
    if (mysql_query(conn, q.c_str())) { mysql_close(conn); return 0.0; }

    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row = res ? mysql_fetch_row(res) : nullptr;
    double moyenne = (row && row[0]) ? std::atof(row[0]) : 0.0;
    if (res) mysql_free_result(res);
    mysql_close(conn);
    return moyenne;
}