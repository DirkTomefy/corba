#include <iostream>
#include <string>
#include <limits>

#include <omniORB4/CORBA.h>
#include <omniORB4/Naming.hh>

#include "note_servant.hpp"
#include "historiquecpp_servant.hpp"


//! Enregistrement / résolution dans le NameService

void registerService(
    CosNaming::NamingContext_ptr namingContext,
    const char* serviceName,
    CORBA::Object_ptr service
) {
    CosNaming::Name name;
    name.length(1);

    name[0].id   = CORBA::string_dup(serviceName);
    name[0].kind = CORBA::string_dup("");

    namingContext->rebind(name, service);

    std::cout << "[C++] " << serviceName
              << " enregistré dans le NameService." << std::endl;
}


CORBA::Object_var resolveService(
    CosNaming::NamingContext_ptr namingContext,
    const char* serviceName
) {
    CosNaming::Name name;
    name.length(1);

    name[0].id   = CORBA::string_dup(serviceName);
    name[0].kind = CORBA::string_dup("");

    return namingContext->resolve(name);
}


//! Helpers de saisie

std::string readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

bool readBoolean() {
    std::string s = readLine();
    return (s == "true" || s == "1" || s == "oui" || s == "OUI" || s == "Oui");
}


//! Question : filtrer les étudiants (service Java)

void askFilterQuestions(EtudiantApp::EtudiantService_ptr etudiantService) {
    while (true) {
        std::cout << "Voulez-vous filtrer les étudiants ? (oui/non) : ";
        std::string reponse = readLine();

        if (reponse == "non" || reponse == "NON" || reponse == "Non") {
            std::cout << "Fin de l'affichage des étudiants." << std::endl;
            break;
        }
        if (reponse != "oui" && reponse != "OUI" && reponse != "Oui") {
            std::cout << "Réponse invalide. Veuillez répondre par 'oui' ou 'non'." << std::endl;
            continue;
        }

        std::cout << "Entrez la colonne pour filtrer (id, numEtu, nom, prenom, email) : ";
        std::string colonne = readLine();

        std::cout << "Entrez l'ordre (true pour ascendant, false pour descendant) : ";
        bool isasc = readBoolean();

        EtudiantApp::EtudiantList_var liste =
            etudiantService->filtrer(colonne.c_str(), isasc);

        std::cout << "[C++] Liste des étudiants reçue depuis Java (etudiants.txt) :" << std::endl;
        for (CORBA::ULong i = 0; i < liste->length(); ++i) {
            const EtudiantApp::Etudiant& e = liste[i];
            std::cout << "   - ID: " << e.id
                      << " | Num: " << e.numEtu
                      << " | " << e.nom << " " << e.prenom
                      << " (" << e.email << ")" << std::endl;
        }
    }
}


//! Question : afficher l'historique Java (fichier historique.txt)

void askHistoriqueJavaQuestions(EtudiantApp::HistoriqueJavaService_ptr historiqueJavaService) {
    while (true) {
        std::cout << "Voulez-vous afficher l'historique Java des actions ? (oui/non) : ";
        std::string reponse = readLine();

        if (reponse == "oui" || reponse == "OUI" || reponse == "Oui") {
            EtudiantApp::HistoriqueJavaList_var liste =
                historiqueJavaService->getAllHistorique();

            std::cout << "[C++] Liste de l'historique Java (historique.txt) :" << std::endl;
            for (CORBA::ULong i = 0; i < liste->length(); ++i) {
                const EtudiantApp::HistoriqueJava& h = liste[i];
                std::cout << "   - ID: " << h.id
                          << " | Action: " << h.action
                          << " | Details: " << h.details
                          << " | Created At: " << h.created_at
                          << std::endl;
            }
        }
        else if (reponse == "non" || reponse == "NON" || reponse == "Non") {
            std::cout << "Fin de l'affichage de l'historique Java." << std::endl;
            break;
        }
        else {
            std::cout << "Réponse invalide. Veuillez répondre par 'oui' ou 'non'." << std::endl;
        }
    }
}


//! Menu principal

void askQuestions(CosNaming::NamingContext_ptr namingContext) {
    try {
        //! Résolution des services Java
        CORBA::Object_var etuObj       = resolveService(namingContext, "EtudiantService");
        CORBA::Object_var histoJavaObj = resolveService(namingContext, "HistoriqueJavaService");

        EtudiantApp::EtudiantService_var      etudiantService      =
            EtudiantApp::EtudiantService::_narrow(etuObj);
        EtudiantApp::HistoriqueJavaService_var historiqueJavaService =
            EtudiantApp::HistoriqueJavaService::_narrow(histoJavaObj);

        if (CORBA::is_nil(etudiantService) ||
            CORBA::is_nil(historiqueJavaService)) {
            std::cerr << "[C++] Impossible de se connecter aux services Java." << std::endl;
            return;
        }

        //! Menu interactif
        while (true) {
            std::cout << "\n================= MENU =================" << std::endl;
            std::cout << "1. Filtrer les étudiants (service Java)" << std::endl;
            std::cout << "2. Afficher l'historique Java (historique.txt)" << std::endl;
            std::cout << "3. Quitter" << std::endl;
            std::cout << "Votre choix : ";

            std::string choix = readLine();

            if (choix == "1") {
                askFilterQuestions(etudiantService);
            }
            else if (choix == "2") {
                askHistoriqueJavaQuestions(historiqueJavaService);
            }
            else if (choix == "3" || choix == "q" || choix == "quit") {
                std::cout << "Fin du menu." << std::endl;
                break;
            }
            else {
                std::cout << "Choix invalide." << std::endl;
            }
        }
    }
    catch (const CORBA::Exception& e) {
        std::cerr << "[C++] Erreur lors de la communication avec les services : "
                  << e._name() << std::endl;
    }
}


//! main

int main(int argc, char* argv[]) {
    try {
        //! Initialisation de l'ORB
        CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

        //! Activation du RootPOA
        CORBA::Object_var objPOA =
            orb->resolve_initial_references("RootPOA");
        PortableServer::POA_var poa =
            PortableServer::POA::_narrow(objPOA);
        PortableServer::POAManager_var poaManager = poa->the_POAManager();
        poaManager->activate();

        //! Création des servants C++ (Note + HistoriqueCpp)
        NoteServant*           noteServant       = new NoteServant();
        HistoriqueCppServant*  historiqueServant = new HistoriqueCppServant();

        //! Création des références CORBA
        EtudiantApp::NoteService_var          noteRef       = noteServant->_this();
        EtudiantApp::HistoriqueCppService_var historiqueRef = historiqueServant->_this();

        //! Résolution du NameService
        CORBA::Object_var nsObj =
            orb->resolve_initial_references("NameService");
        CosNaming::NamingContext_var namingContext =
            CosNaming::NamingContext::_narrow(nsObj);

        //! Enregistrement des services C++
        registerService(namingContext, "NoteService",          noteRef);
        registerService(namingContext, "HistoriqueCppService",  historiqueRef);

        //! Pause : on attend que le serveur Java soit lancé
        std::cout << "\n====================================================" << std::endl;
        std::cout << "Assurez-vous que le serveur Java est AUSSI lancé." << std::endl;
        std::cout << "Appuyez sur ENTRÉE pour continuer..." << std::endl;
        std::cout << "====================================================\n" << std::endl;
        std::cin.get();

        //! Menu interactif (services Java uniquement)
        askQuestions(namingContext);

        //! Lancement de l'ORB
        std::cout << "[C++] Serveur prêt et en écoute..." << std::endl;
        orb->run();
    }
    catch (const CORBA::Exception& ex) {
        std::cerr << "[C++] Erreur CORBA : " << ex._name() << std::endl;
        return 1;
    }

    return 0;
}