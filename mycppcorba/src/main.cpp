#include <iostream>
#include <string>
#include <limits>

#include <omniORB4/CORBA.h>
#include <omniORB4/Naming.hh>

#include "etudiant_servant.hpp"
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


//! Helpers de saisie (style Java Scanner)

std::string readLine() {
    std::string line;
    std::getline(std::cin, line);
    return line;
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


//! Question : consulter les notes (service Java)

void askNoteQuestions(EtudiantApp::NoteService_ptr noteService) {
    while (true) {
        std::cout << "\n--- Menu Notes (Java) ---" << std::endl;
        std::cout << "1. Afficher toutes les notes" << std::endl;
        std::cout << "2. Afficher les notes d'un étudiant" << std::endl;
        std::cout << "3. Calculer la moyenne d'un étudiant" << std::endl;
        std::cout << "4. Retour" << std::endl;
        std::cout << "Votre choix : ";

        std::string choix = readLine();

        if (choix == "4" || choix == "q" || choix == "quit") {
            std::cout << "Fin du menu Notes." << std::endl;
            break;
        }

        if (choix == "1") {
            EtudiantApp::NoteList_var liste = noteService->getAllNotes();
            std::cout << "[C++] Toutes les notes reçues depuis Java :" << std::endl;
            for (CORBA::ULong i = 0; i < liste->length(); ++i) {
                const EtudiantApp::Note& n = liste[i];
                std::cout << "   - NumEtu: " << n.numEtu
                          << " | Matiere: " << n.matiere
                          << " | Valeur: " << n.valeur
                          << std::endl;
            }
        }
        else if (choix == "2") {
            std::cout << "Entrez le numéro étudiant : ";
            std::string numEtu = readLine();

            EtudiantApp::NoteList_var liste =
                noteService->getNotesByEtu(numEtu.c_str());

            std::cout << "[C++] Notes de l'étudiant " << numEtu << " :" << std::endl;
            for (CORBA::ULong i = 0; i < liste->length(); ++i) {
                const EtudiantApp::Note& n = liste[i];
                std::cout << "   - Matiere: " << n.matiere
                          << " | Valeur: " << n.valeur
                          << std::endl;
            }
        }
        else if (choix == "3") {
            std::cout << "Entrez le numéro étudiant : ";
            std::string numEtu = readLine();

            try {
                double moyenne = noteService->getMoyenne(numEtu.c_str());
                std::cout << "[C++] Moyenne de l'étudiant " << numEtu
                          << " : " << moyenne << "/20" << std::endl;
            }
            catch (const CORBA::Exception& ex) {
                std::cerr << "[C++] Erreur lors de l'appel à getMoyenne : "
                          << ex._name() << std::endl;
            }
        }
        else {
            std::cout << "Choix invalide." << std::endl;
        }
    }
}


//! Menu principal

void askQuestions(
    CosNaming::NamingContext_ptr namingContext
) {
    try {
        //! Résolution des services Java
        CORBA::Object_var histoJavaObj = resolveService(namingContext, "HistoriqueJavaService");
        CORBA::Object_var noteObj      = resolveService(namingContext, "NoteService");

        EtudiantApp::HistoriqueJavaService_var historiqueJavaService =
            EtudiantApp::HistoriqueJavaService::_narrow(histoJavaObj);
        EtudiantApp::NoteService_var          noteService          =
            EtudiantApp::NoteService::_narrow(noteObj);

        if (CORBA::is_nil(historiqueJavaService) ||
            CORBA::is_nil(noteService)) {
            std::cerr << "[C++] Impossible de se connecter aux services Java." << std::endl;
            return;
        }

        //! Menu interactif
        while (true) {
            std::cout << "\n================= MENU =================" << std::endl;
            std::cout << "1. Afficher l'historique Java (historique.txt)" << std::endl;
            std::cout << "2. Consulter les notes (service Java)" << std::endl;
            std::cout << "3. Quitter" << std::endl;
            std::cout << "Votre choix : ";

            std::string choix = readLine();

            if (choix == "1") {
                askHistoriqueJavaQuestions(historiqueJavaService);
            }
            else if (choix == "2") {
                askNoteQuestions(noteService);
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

        //! Création des servants C++
        EtudiantServant*       etudiantServant   = new EtudiantServant();
        HistoriqueCppServant*  historiqueServant = new HistoriqueCppServant();

        //! Création des références CORBA
        EtudiantApp::EtudiantService_var      etudiantRef   = etudiantServant->_this();
        EtudiantApp::HistoriqueCppService_var historiqueRef = historiqueServant->_this();

        //! Résolution du NameService
        CORBA::Object_var nsObj =
            orb->resolve_initial_references("NameService");
        CosNaming::NamingContext_var namingContext =
            CosNaming::NamingContext::_narrow(nsObj);

        //! Enregistrement des services C++
        registerService(namingContext, "EtudiantService",      etudiantRef);
        registerService(namingContext, "HistoriqueCppService", historiqueRef);

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