#include <iostream>
#include <string>

#include <omniORB4/CORBA.h>
#include <omniORB4/Naming.hh>

#include "etudiant_servant.hpp"
#include "historiquecpp_servant.hpp"


void registerService(
    CosNaming::NamingContext_ptr namingContext,
    const char* serviceName,
    CORBA::Object_ptr service
) {
    CosNaming::Name name;
    name.length(1);

    name[0].id = CORBA::string_dup(serviceName);
    name[0].kind = CORBA::string_dup("");

    namingContext->rebind(name, service);

    std::cout
        << "[C++] "
        << serviceName
        << " enregistré dans le NameService."
        << std::endl;
}


CORBA::Object_var resolveService(
    CosNaming::NamingContext_ptr namingContext,
    const char* serviceName
) {
    CosNaming::Name name;
    name.length(1);

    name[0].id = CORBA::string_dup(serviceName);
    name[0].kind = CORBA::string_dup("");

    return namingContext->resolve(name);
}


int main(int argc, char* argv[]) {

    try {

        //!Initialisation de l'ORB
        CORBA::ORB_var orb =
            CORBA::ORB_init(argc, argv);


      
        //! Activation du RootPOA
        CORBA::Object_var objPOA =
            orb->resolve_initial_references("RootPOA");

        PortableServer::POA_var poa =
            PortableServer::POA::_narrow(objPOA);

        PortableServer::POAManager_var poaManager =
            poa->the_POAManager();

        poaManager->activate();


        //! Création des servants
        EtudiantServant* etudiantServant =
            new EtudiantServant();

        HistoriqueCppServant* historiqueServant =
            new HistoriqueCppServant();


        //! Création des références CORBA

        EtudiantApp::EtudiantService_var etudiantRef =
            etudiantServant->_this();

        EtudiantApp::HistoriqueCppService_var historiqueRef =
            historiqueServant->_this();

        //! Résolution du NameService
        CORBA::Object_var nsObj =
            orb->resolve_initial_references("NameService");

        CosNaming::NamingContext_var namingContext =
            CosNaming::NamingContext::_narrow(nsObj);


        //! Enregistrement des services dans le NameService
        registerService(
            namingContext,
            "EtudiantService",
            etudiantRef
        );

        registerService(
            namingContext,
            "HistoriqueCppService",
            historiqueRef
        );

         //! ne jamais enlever
        std::cout << "\n====================================================" << std::endl;
        std::cout << "Assurez-vous que le serveur Java est AUSSI lance." << std::endl;
        std::cout << "Appuyez sur ENTRÉE pour executer la requete vers Java..." << std::endl;
        std::cout << "====================================================\n" << std::endl;
        std::cin.get();


      //! Recherche du service Java "NoteService" dans le NameService
        std::cout
            << "[C++] Recherche du service Java "
            << "'NoteService'..."
            << std::endl;

        CORBA::Object_var noteObj =
            resolveService(
                namingContext,
                "NoteService"
            );

        EtudiantApp::NoteService_var noteService =
            EtudiantApp::NoteService::_narrow(noteObj);

         std::cout
            << "[C++] Recherche du service Java "
            << "'NoteService'..."
            << std::endl;
        
       

        //! Appel du service Java "NoteService" pour obtenir la moyenne d'un étudiant
        if (!CORBA::is_nil(noteService)) {

            std::string numeroEtudiant = "etu003948";

            double moyenne =
                noteService->getMoyenne(
                    numeroEtudiant.c_str()
                );

            std::cout
                << "[C++] Moyenne reçue depuis Java pour "
                << "l'étudiant "
                << numeroEtudiant
                << " : "
                << moyenne
                << "/20"
                << std::endl;
        }
        else {
            std::cerr
                << "[C++] Impossible de trouver "
                << "NoteService."
                << std::endl;
        }


      
        std::cout
            << "[C++] Serveur prêt et en écoute..."
            << std::endl;

        orb->run();


    }
    catch (const CORBA::Exception& ex) {

        std::cerr
            << "[C++] Erreur CORBA : "
            << ex._name()
            << std::endl;

        return 1;
    }

    return 0;
}
