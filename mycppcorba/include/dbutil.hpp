#ifndef DBUTIL_HPP
#define DBUTIL_HPP

#include <string>
#include <mysql/mysql.h>

// Configuration de la connexion
struct DbConfig {
    std::string host     = "127.0.0.1";
    std::string user     = "tomefy";
    std::string password = "etu003948";
    std::string database = "corbatest";
    unsigned int port    = 3306;
};

// Ouvre une connexion MySQL. Retourne nullptr en cas d'échec.
MYSQL* connectToDatabase(const DbConfig& cfg);

// Exécute une requête SQL. Retourne true si succès.
bool executeQuery(MYSQL* conn, const std::string& query);

void displayEtudiant(MYSQL* conn) ;

#endif // DBUTIL_HPP
