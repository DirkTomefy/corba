# Guide : Ajouter 2 nouvelles entités (une côté C++, une côté Java)

> Exemple concret : on ajoute **`Enseignant`** (implémenté en **C++ / MySQL**) et **`Cours`** (implémenté en **Java / fichier txt**).
> Deux entités **distinctes**, chacune avec son service, son servant, son asker.
> À suivre **dans l'ordre**.

---

## 0. Vue d'ensemble

```
┌──────────────────┐          ┌──────────────────┐
│  Enseignant      │          │  Cours           │
│  côté C++        │          │  côté Java       │
│                  │          │                  │
│  BDD MySQL       │          │  cours.txt       │
│  enseignant      │          │                  │
└────────┬─────────┘          └────────┬─────────┘
         │                             │
         ▼                             ▼
   EnseignantService            CoursService
   (enregistré par C++)          (enregistré par Java)
         │                             │
         └────────────┬────────────────┘
                      ▼
              NameService (orbd)
                      │
         ┌────────────┴────────────┐
         ▼                         ▼
    main.cpp (C++)             App.java (Java)
    - sert Enseignant          - sert Cours
    - appelle Cours            - appelle Enseignant
```

**Règle d'or** :
- 1 entité = 1 service = 1 servant = 1 asker.
- Chaque service est enregistré par **le camp qui l'implémente**.
- L'autre camp l'appelle via le NameService.

---

## 1. Script SQL — Base C++ complète

**Fichier à exécuter :** `notes_cpp_db.sql` (ou dans le client MySQL)

Ce script crée **toute** la base C++, y compris les tables existantes et les nouvelles. Tu peux le lancer sur un serveur MySQL neuf ou réutiliser les parties qui t'intéressent.

```sql
-- =====================================================
--  Base de données C++ : notes_cpp_db
--  Contient : Note, Enseignant, Historique
-- =====================================================

DROP DATABASE IF EXISTS notes_cpp_db;
CREATE DATABASE notes_cpp_db
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;
USE notes_cpp_db;

-- -----------------------------------------------------
--  Table note (existante)
-- -----------------------------------------------------
CREATE TABLE note (
    id        INT AUTO_INCREMENT PRIMARY KEY,
    num_etu   VARCHAR(50)  NOT NULL,
    matiere   VARCHAR(100) NOT NULL,
    valeur    DOUBLE       NOT NULL,
    INDEX idx_num_etu (num_etu)
) ENGINE=InnoDB;

-- -----------------------------------------------------
--  Table enseignant (NOUVELLE)
-- -----------------------------------------------------
CREATE TABLE enseignant (
    id      INT AUTO_INCREMENT PRIMARY KEY,
    nom     VARCHAR(100) NOT NULL,
    matiere VARCHAR(100) NOT NULL,
    email   VARCHAR(150),
    INDEX idx_nom (nom)
) ENGINE=InnoDB;

-- -----------------------------------------------------
--  Table historique (schéma inchangé)
-- -----------------------------------------------------
CREATE TABLE historique (
    id         BIGINT AUTO_INCREMENT PRIMARY KEY,
    action     VARCHAR(100) NOT NULL,
    details    TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_created_at (created_at)
) ENGINE=InnoDB;

-- -----------------------------------------------------
--  Jeu de données de test
-- -----------------------------------------------------

INSERT INTO note (num_etu, matiere, valeur) VALUES
('etu003948', 'Mathématiques', 15.5),
('etu003948', 'Physique',      12.0),
('etu003948', 'Informatique',  17.5),
('etu001234', 'Mathématiques', 10.0),
('etu001234', 'Informatique',  14.0);

INSERT INTO enseignant (nom, matiere, email) VALUES
('Dupont',   'Mathématiques', 'dupont@univ.fr'),
('Martin',   'Physique',      'martin@univ.fr'),
('Bernard',  'Informatique',  'bernard@univ.fr');
```

### Utilisateur MySQL (optionnel, bonne pratique)

Pour éviter d'utiliser `root` sans mot de passe :

```sql
CREATE USER 'corba_user'@'localhost' IDENTIFIED BY 'corba_pass';
GRANT ALL PRIVILEGES ON notes_cpp_db.* TO 'corba_user'@'localhost';
FLUSH PRIVILEGES;
```

Puis dans `dbutil.hpp` :

```cpp
struct DbConfig {
    std::string  host     = "localhost";
    std::string  user     = "corba_user";
    std::string  password = "corba_pass";
    std::string  database = "notes_cpp_db";
    unsigned int port     = 3306;
};
```

### Exécution

```bash
# Depuis un shell
mysql -u root -p < notes_cpp_db.sql

# Ou dans le client interactif
mysql -u root -p
source notes_cpp_db.sql;
```

### Vérification

```sql
USE notes_cpp_db;
SHOW TABLES;
-- Résultat attendu : enseignant, historique, note

SELECT COUNT(*) FROM enseignant;
SELECT * FROM enseignant;
```

---

## 2. Côté IDL — Définir les 2 contrats

**Fichier :** `idl/etudiant.idl`

### 2.1 Entité `Enseignant` (côté C++)

```idl
struct Enseignant {
    long    id;
    string  nom;
    string  matiere;
    string  email;
};

typedef sequence<Enseignant> EnseignantList;

interface EnseignantService {
    EnseignantList getAll();
    Enseignant     getById(in long id);
    long           add(in Enseignant e);
};
```

### 2.2 Entité `Cours` (côté Java)

```idl
struct Cours {
    long    id;
    string  intitule;
    long    credits;
    string  enseignant;
};

typedef sequence<Cours> CoursList;

interface CoursService {
    CoursList getAll();
    Cours     getById(in long id);
    long      add(in Cours c);
};
```

### 2.3 Régénérer

```bash
# C++
omniidl -bcxx -Wbh=.h -Wbs=.cpp idl/etudiant.idl -C src/generated/

# Java
idlj -fall -td src/main/java idl/etudiant.idl
```

---

## 3. Côté C++ — Entité `Enseignant`

### 3.1 Header

**Fichier :** `include/enseignant_servant.hpp`

```cpp
#ifndef ENSEIGNANT_SERVANT_HPP
#define ENSEIGNANT_SERVANT_HPP

#include <etudiant.h>

class EnseignantServant : public POA_EtudiantApp::EnseignantService {
public:
    EnseignantServant() = default;
    virtual ~EnseignantServant() = default;

    EtudiantApp::EnseignantList* getAll();
    EtudiantApp::Enseignant*     getById(CORBA::Long id);
    CORBA::Long                  add(const EtudiantApp::Enseignant& e);
};

#endif
```

### 3.2 Implémentation

**Fichier :** `src/enseignant_servant.cpp`

```cpp
#include "enseignant_servant.hpp"
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

EtudiantApp::EnseignantList* EnseignantServant::getAll() {
    auto* list = new EtudiantApp::EnseignantList();
    DbConfig cfg;
    MYSQL* conn = connectToDatabase(cfg);
    if (!conn) { list->length(0); return list; }

    historize(conn, "getAll_Enseignant", "Recuperation complete");

    if (mysql_query(conn, "SELECT id, nom, matiere, email FROM enseignant")) {
        mysql_close(conn); list->length(0); return list;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row;
    while (res && (row = mysql_fetch_row(res))) {
        EtudiantApp::Enseignant e;
        e.id      = row[0] ? std::atoi(row[0]) : 0;
        e.nom     = CORBA::string_dup(row[1] ? row[1] : "");
        e.matiere = CORBA::string_dup(row[2] ? row[2] : "");
        e.email   = CORBA::string_dup(row[3] ? row[3] : "");
        CORBA::ULong idx = list->length();
        list->length(idx + 1);
        (*list)[idx] = e;
    }
    if (res) mysql_free_result(res);
    mysql_close(conn);
    return list;
}

EtudiantApp::Enseignant* EnseignantServant::getById(CORBA::Long id) {
    auto* r = new EtudiantApp::Enseignant();
    r->id = 0;
    r->nom = CORBA::string_dup("");
    r->matiere = CORBA::string_dup("");
    r->email = CORBA::string_dup("");

    DbConfig cfg;
    MYSQL* conn = connectToDatabase(cfg);
    if (!conn) return r;

    historize(conn, "getById_Enseignant", "id=" + std::to_string(id));

    std::string q = "SELECT id, nom, matiere, email FROM enseignant WHERE id = "
                  + std::to_string(id) + " LIMIT 1";
    if (mysql_query(conn, q.c_str())) { mysql_close(conn); return r; }

    MYSQL_RES* res = mysql_store_result(conn);
    MYSQL_ROW row = res ? mysql_fetch_row(res) : nullptr;
    if (row) {
        r->id      = row[0] ? std::atoi(row[0]) : 0;
        r->nom     = CORBA::string_dup(row[1] ? row[1] : "");
        r->matiere = CORBA::string_dup(row[2] ? row[2] : "");
        r->email   = CORBA::string_dup(row[3] ? row[3] : "");
    }
    if (res) mysql_free_result(res);
    mysql_close(conn);
    return r;
}

CORBA::Long EnseignantServant::add(const EtudiantApp::Enseignant& e) {
    DbConfig cfg;
    MYSQL* conn = connectToDatabase(cfg);
    if (!conn) return -1;

    historize(conn, "add_Enseignant",
              std::string("nom=") + std::string(e.nom));

    std::string q = "INSERT INTO enseignant (nom, matiere, email) VALUES ('"
                  + escape(conn, e.nom)     + "', '"
                  + escape(conn, e.matiere) + "', '"
                  + escape(conn, e.email)   + "')";
    if (mysql_query(conn, q.c_str())) {
        std::cerr << "Erreur INSERT : " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return -1;
    }
    CORBA::Long id = static_cast<CORBA::Long>(mysql_insert_id(conn));
    mysql_close(conn);
    return id;
}
```

### 3.3 CMakeLists.txt

```cmake
add_executable(mycppcorba
    src/main.cpp
    src/dbutil.cpp
    src/note_servant.cpp
    src/historiquecpp_servant.cpp
    src/enseignant_servant.cpp    # ← nouveau
    src/generated/etudiant.cpp
)
```

---

## 4. Côté Java — Entité `Cours`

### 4.1 Classe servant

**Fichier :** `src/main/java/myjavacorba/server/CoursServantImpl.java`

```java
package myjavacorba.server;

import EtudiantApp.*;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.CopyOnWriteArrayList;

public class CoursServantImpl extends CoursServicePOA {

    private static final String FICHIER = "cours.txt";

    private final List<Cours> cours = new CopyOnWriteArrayList<>();

    public CoursServantImpl() throws IOException {
        ensureFileExists();
        loadFromFile();
    }

    //! Retourne target/classes/cours.txt
    private static File getFichier() {
        try {
            File root = new File(
                CoursServantImpl.class
                    .getProtectionDomain()
                    .getCodeSource()
                    .getLocation()
                    .toURI()
            );
            return new File(root, FICHIER);
        } catch (Exception e) {
            System.err.println("Impossible de localiser le classpath : " + e.getMessage());
            return new File(FICHIER);
        }
    }

    private void ensureFileExists() throws IOException {
        File f = getFichier();
        if (f.exists()) return;

        try (InputStream is = getClass().getResourceAsStream("/" + FICHIER)) {
            if (is == null) throw new IOException("Ressource introuvable : /" + FICHIER);
            try (OutputStream os = new FileOutputStream(f)) {
                is.transferTo(os);
            }
        }
        System.out.println("Fichier initialise : " + f.getAbsolutePath());
    }

    private void loadFromFile() throws IOException {
        try (BufferedReader br = new BufferedReader(
                new InputStreamReader(new FileInputStream(getFichier()),
                                      StandardCharsets.UTF_8))) {
            String ligne;
            while ((ligne = br.readLine()) != null) {
                ligne = ligne.trim();
                if (ligne.isEmpty() || ligne.startsWith("#")) continue;
                String[] parts = ligne.split(";");
                if (parts.length < 4) continue;

                Cours c = new Cours();
                c.id         = Integer.parseInt(parts[0].trim());
                c.intitule   = parts[1].trim();
                c.credits    = Long.parseLong(parts[2].trim());
                c.enseignant = parts[3].trim();
                cours.add(c);
            }
        }
        System.out.println("Cours charges : " + cours.size());
    }

    private void saveAll() {
        try (BufferedWriter bw = new BufferedWriter(
                new OutputStreamWriter(new FileOutputStream(getFichier()),
                                       StandardCharsets.UTF_8))) {
            for (Cours c : cours) {
                bw.write(c.id + ";" + c.intitule + ";" + c.credits + ";" + c.enseignant);
                bw.newLine();
            }
        } catch (IOException ex) {
            System.err.println("Erreur ecriture : " + ex.getMessage());
        }
    }

    @Override
    public Cours[] getAll() {
        HistoriqueJavaServantImpl.historize(
            "GET_ALL_COURS", "Nombre=" + cours.size()
        );
        return cours.toArray(new Cours[0]);
    }

    @Override
    public Cours getById(int id) {
        for (Cours c : cours) {
            if (c.id == id) {
                HistoriqueJavaServantImpl.historize("GET_COURS", "id=" + id);
                return c;
            }
        }
        HistoriqueJavaServantImpl.historize("GET_COURS", "id=" + id + " introuvable");

        Cours vide = new Cours();
        vide.id = 0; vide.intitule = ""; vide.credits = 0; vide.enseignant = "";
        return vide;
    }

    @Override
    public int add(Cours c) {
        int maxId = 0;
        for (Cours x : cours) if (x.id > maxId) maxId = x.id;
        c.id = maxId + 1;
        cours.add(c);
        saveAll();

        HistoriqueJavaServantImpl.historize("ADD_COURS",
            "id=" + c.id + ", intitule=" + c.intitule);
        return c.id;
    }
}
```

### 4.2 Ressource par défaut

**Fichier :** `src/main/resources/cours.txt`

```
# Fichier par defaut des cours
1;Mathématiques;6;Dupont
2;Physique;4;Martin
3;Informatique;8;Bernard
```

---

## 5. `main.cpp` — Sert `Enseignant`, appelle `Cours`

### 5.1 Includes

```cpp
#include <cstdlib>
#include "enseignant_servant.hpp"
```

### 5.2 Modifications dans `main()`

**Création des servants :**
```cpp
EnseignantServant* enseignantServant = new EnseignantServant();
```

**Références CORBA :**
```cpp
EtudiantApp::EnseignantService_var enseignantRef = enseignantServant->_this();
```

**Enregistrement :**
```cpp
registerService(namingContext, "EnseignantService", enseignantRef);
```

### 5.3 Askers

Ajoute `askEnseignantQuestions` et `askCoursQuestions` dans `main.cpp` (voir version précédente du guide pour le code complet). Puis branche-les dans `askQuestions` :

```cpp
// Résolution
CORBA::Object_var enseignantObj = resolveService(namingContext, "EnseignantService");
EtudiantApp::EnseignantService_var enseignantService =
    EtudiantApp::EnseignantService::_narrow(enseignantObj);

CORBA::Object_var coursObj = resolveService(namingContext, "CoursService");
EtudiantApp::CoursService_var coursService =
    EtudiantApp::CoursService::_narrow(coursObj);

// Menu
if (choix == "1")      askEnseignantQuestions(enseignantService);
else if (choix == "2") askCoursQuestions(coursService);
else if (choix == "3") break;
```

---

## 6. `App.java` — Sert `Cours`, appelle `Enseignant`

### 6.1 Import

```java
import myjavacorba.server.CoursServantImpl;
```

### 6.2 Enregistrement dans `main()`

```java
CoursServantImpl coursServant = new CoursServantImpl();
org.omg.CORBA.Object refCours = rootpoa.servant_to_reference(coursServant);
CoursService hrefCours = CoursServiceHelper.narrow(refCours);
NameComponent pathCours[] = ncRef.to_name("CoursService");
ncRef.rebind(pathCours, hrefCours);
System.out.println("[Java] CoursService enregistre dans le NameService.");
```

### 6.3 Askers et menu

Ajoute `askEnseignantQuestions` et `askCoursQuestions` dans `App.java`, puis branche-les dans `askQuestions` :

```java
org.omg.CORBA.Object enseignantObj = ncRef.resolve_str("EnseignantService");
EnseignantService enseignantService = EnseignantServiceHelper.narrow(enseignantObj);

org.omg.CORBA.Object coursObj = ncRef.resolve_str("CoursService");
CoursService coursService = CoursServiceHelper.narrow(coursObj);

if (choix.equals("1"))      askEnseignantQuestions(enseignantService, scanner);
else if (choix.equals("2")) askCoursQuestions(coursService, scanner);
else if (choix.equals("3")) break;
```

---

## 7. Tableau récapitulatif — Qui sert quoi

| Entité | Implémenté par | Enregistré par | Stockage |
|---|---|---|---|
| `Enseignant` | **C++** | `main.cpp` | MySQL `notes_cpp_db.enseignant` |
| `Cours` | **Java** | `App.java` | `target/classes/cours.txt` |
| `Etudiant` | Java | `App.java` | `target/classes/etudiants.txt` |
| `Note` | C++ | `main.cpp` | MySQL `notes_cpp_db.note` |
| `HistoriqueCpp` | C++ | `main.cpp` | MySQL `notes_cpp_db.historique` |
| `HistoriqueJava` | Java | `App.java` | `target/classes/historique.txt` |

---

## 8. Fichiers à créer / modifier — Récap

### À créer

| Fichier | Côté |
|---|---|
| `notes_cpp_db.sql` | SQL (à exécuter une fois) |
| `include/enseignant_servant.hpp` | C++ |
| `src/enseignant_servant.cpp` | C++ |
| `src/main/java/myjavacorba/server/CoursServantImpl.java` | Java |
| `src/main/resources/cours.txt` | Java |

### À modifier

| Fichier | Modification |
|---|---|
| `idl/etudiant.idl` | + struct `Enseignant`, + struct `Cours`, + 2 sequences, + 2 interfaces |
| `src/main.cpp` | + include, + servant C++, + `registerService`, + 2 askers |
| `CMakeLists.txt` | + `src/enseignant_servant.cpp` |
| `src/main/java/myjavacorba/App.java` | + import, + `rebind` `CoursService`, + 2 askers |

### À regénérer

```bash
omniidl -bcxx -Wbh=.h -Wbs=.cpp idl/etudiant.idl -C src/generated/
idlj -fall -td src/main/java idl/etudiant.idl
```

---

## 9. Pièges classiques

| Piège | Solution |
|---|---|
| `undefined reference` C++ | Fichier `.cpp` oublié dans `CMakeLists.txt` |
| `cannot find symbol` Java | IDL pas regénéré → `mvn clean` |
| `NotFound` au `resolve` | Service pas encore `rebind` → ordre de lancement |
| `std::string + CORBA::String_member` | Convertir : `std::string(e.nom)` |
| Fichier `.txt` non trouvé | Bien dans `src/main/resources/` |
| Modification non persistée | `saveAll()` après chaque écriture |
| `id` toujours à 0 | Recalculer `maxId + 1` |
| Asker pas appelé | Vérifier le `resolve` **avant** la boucle du menu |
| `std::atol` manquant | `#include <cstdlib>` |
| **Accès MySQL refusé** | Vérifier `DbConfig` (user/password/database) et que la BDD `notes_cpp_db` existe |

---

## 10. Ordre de lancement

1. **MySQL** démarré + `notes_cpp_db.sql` exécuté
2. **`tnameserv`** ou **`orbd`**
3. **Serveur Java** → enregistre `EtudiantService`, `HistoriqueJavaService`, `CoursService`
4. **Serveur C++** → enregistre `NoteService`, `HistoriqueCppService`, `EnseignantService`
5. `ENTRÉE` dans les deux fenêtres
6. Utiliser les menus

---

**Fin du guide.** 🎯