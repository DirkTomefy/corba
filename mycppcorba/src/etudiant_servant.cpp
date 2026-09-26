#include "etudiant_servant.hpp"
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

static std::string historize(MYSQL* conn, const std::string& action, const std::string& details) {
    std::string q = "INSERT INTO historique (action, details) VALUES ('"
                  + escape(conn, action.c_str()) + "', '"
                  + escape(conn, details.c_str()) + "')";
    if (mysql_query(conn, q.c_str())) {
        std::cerr << "Erreur INSERT historique : " << mysql_error(conn) << std::endl;
        return "";
    }
    return std::to_string(mysql_insert_id(conn));
}



static EtudiantApp::Etudiant convertsqlRowToEtudiant(MYSQL_ROW row){
    EtudiantApp::Etudiant e;
    e.id     = row[0] ? std::atoi(row[0]) : 0;
    e.numEtu = CORBA::string_dup(row[1] ? row[1] : "");
    e.nom    = CORBA::string_dup(row[2] ? row[2] : "");
    e.prenom = CORBA::string_dup(row[3] ? row[3] : "");
    e.email  = CORBA::string_dup(row[4] ? row[4] : "");
    return e;
}

EtudiantApp::EtudiantList* EtudiantServant::getAll() {
    EtudiantApp::EtudiantList* list = new EtudiantApp::EtudiantList();
    DbConfig cfg;
    MYSQL* conn = connectToDatabase(cfg);
    if (!conn) { list->length(0); return list; }
    historize(conn, "getAll", "Récupération de tous les étudiants");
    if (mysql_query(conn, "SELECT id, num_etu, nom, prenom, email FROM etudiant ORDER BY id")) {
        mysql_close(conn);
        list->length(0);
        return list;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while (res && (row = mysql_fetch_row(res))) {
        EtudiantApp::Etudiant e = convertsqlRowToEtudiant(row);
        CORBA::ULong idx = list->length();
        list->length(idx + 1);
        (*list)[idx] = e;
    }

    if (res) mysql_free_result(res);
    mysql_close(conn);
    return list;
}

EtudiantApp::Etudiant* EtudiantServant::getByNumEtu(const char* numEtu) {
    EtudiantApp::Etudiant* r = new EtudiantApp::Etudiant();
    r->id = 0;
    r->numEtu = CORBA::string_dup("");
    r->nom = CORBA::string_dup("");
    r->prenom = CORBA::string_dup("");
    r->email = CORBA::string_dup("");

    DbConfig cfg;
    MYSQL* conn = connectToDatabase(cfg);
    if (!conn) return r;
    historize(conn, "getByNumEtu", "Récupération d'un étudiant par son numéro");
    std::string q = "SELECT id, num_etu, nom, prenom, email FROM etudiant "
                    "WHERE num_etu = '" + escape(conn, numEtu) + "' LIMIT 1";
    if (mysql_query(conn, q.c_str())) { mysql_close(conn); return r; }

    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row = res ? mysql_fetch_row(res) : nullptr;
    if (row) {
        *r = convertsqlRowToEtudiant(row);
    }

    if (res) mysql_free_result(res);
    mysql_close(conn);
    return r;
}

CORBA::Long EtudiantServant::addEtudiant(const EtudiantApp::Etudiant& e) {
    DbConfig cfg;
    MYSQL* conn = connectToDatabase(cfg);
    if (!conn) return -1;
    historize(conn, "addEtudiant", "Ajout d'un nouvel étudiant");
    std::string q = "INSERT INTO etudiant (num_etu, nom, prenom, email) VALUES ('"
                  + escape(conn, e.numEtu) + "', '"
                  + escape(conn, e.nom)    + "', '"
                  + escape(conn, e.prenom) + "', '"
                  + escape(conn, e.email)  + "')";
    if (mysql_query(conn, q.c_str())) {
        std::cerr << "Erreur INSERT : " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return -1;
    }
    CORBA::Long id = static_cast<CORBA::Long>(mysql_insert_id(conn));
    mysql_close(conn);
    return id;
}

CORBA::Long EtudiantServant::count() {
    DbConfig cfg;
    MYSQL* conn = connectToDatabase(cfg);
    if (!conn) return -1;
    historize(conn, "count", "Comptage du nombre d'étudiants");
    if (mysql_query(conn, "SELECT COUNT(*) FROM etudiant")) { mysql_close(conn); return -1; }
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row = res ? mysql_fetch_row(res) : nullptr;
    CORBA::Long n = (row && row[0]) ? std::atol(row[0]) : -1;
    if (res) mysql_free_result(res);
    mysql_close(conn);
    return n;
}


EtudiantApp::EtudiantList* EtudiantServant::filtrer(const char* colonne,const bool isasc) {
     EtudiantApp::EtudiantList* list = new EtudiantApp::EtudiantList();
    DbConfig cfg;
    MYSQL* conn = connectToDatabase(cfg);
    historize(conn, "filtrer", "Filtrer les étudiants");
    if (!conn) { list->length(0); return list; }
    std::string query ="SELECT id, num_etu, nom, prenom, email FROM etudiant ORDER BY " + escape(conn, colonne) + " " + (isasc ? "ASC" : "DESC");
    if (mysql_query(conn,query.c_str()))  {
        mysql_close(conn);
        list->length(0);
        return list;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while (res && (row = mysql_fetch_row(res))) {
        EtudiantApp::Etudiant e = convertsqlRowToEtudiant(row);
        CORBA::ULong idx = list->length();
        list->length(idx + 1);
        (*list)[idx] = e;
    }

    if (res) mysql_free_result(res);
    mysql_close(conn);
    return list;
}
