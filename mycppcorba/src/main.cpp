#include <iostream>
#include <omniORB4/CORBA.h>
#include "etudiant_servant.hpp"


int main(int argc, char* argv[]) {
    try{
    //! initilaiser l'orb 
    CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

    //! activer le POA
    CORBA::Object_var objPOA = orb->resolve_initial_references("RootPOA");
        PortableServer::POA_var poa = PortableServer::POA::_narrow(objPOA);
        PortableServer::POAManager_var pman = poa->the_POAManager();
        pman->activate(); // changer l'état du POA en "active" (et non en holding)

    //! Instancier le Servant Etudiant et l'enregistrer dans le POA
    EtudiantServant* etudiant_servant = new EtudiantServant();
    EtudiantApp::EtudiantService_var etudiant_ref = etudiant_servant->_this();

    //*Type ID : IDL:EtudiantApp/EtudiantService:1.0 */
    //*IOD : IOR:000000000000002b49444c3a4574756469616e4170702f4574756469616e74536572766963653a312e3000... */

    //! Se connecter au Service de Nommage (CosNaming)
        CORBA::Object_var nsObj = orb->resolve_initial_references("NameService");
        CosNaming::NamingContext_var nc = CosNaming::NamingContext::_narrow(nsObj);    

    
    //! Enregistrer "EtudiantService"
    CosNaming::Name name;
        name.length(1);
        name[0].id = CORBA::string_dup("EtudiantService");
        name[0].kind = CORBA::string_dup("");
        nc->rebind(name, etudiant_ref);
        std::cout << "[C++] EtudiantService enregistre dans le NameService." << std::endl;
    
    //? PAUSE INTERACTIVE : 
    //TODO : mila atao thread:sleep @ manaraka (autonome)
        std::cout << "\n====================================================" << std::endl;
        std::cout << "Assurez-vous que le serveur Java est AUSSI lance." << std::endl;
        std::cout << "Appuyez sur ENTRÉE pour executer la requete vers Java..." << std::endl;
        std::cout << "====================================================\n" << std::endl;
        std::cin.get();
    
    //? Rechercher le service Java "NoteService" via le NameService    
        std::cout << "[C++] Recherche du service Java 'NoteService'..." << std::endl;
        CosNaming::Name noteName;
        noteName.length(1);
        noteName[0].id = CORBA::string_dup("NoteService");
        noteName[0].kind = CORBA::string_dup("");

        CORBA::Object_var noteObj = nc->resolve(noteName);
        EtudiantApp::NoteService_var noteService = EtudiantApp::NoteService::_narrow(noteObj);

        if (!CORBA::is_nil(noteService)) {
            std::string numEtuTest = "etu003948";
            double moyenne = noteService->getMoyenne(numEtuTest.c_str());
            std::cout << "[C++] Moyenne reçue depuis Java pour l'etudiant " 
                      << numEtuTest << " : " << moyenne << "/20" << std::endl;
        }

        // Attendre les requêtes entrantes
        std::cout << "[C++] Serveur C++ pret et en ecoute..." << std::endl;
        orb->run();

    } catch (const CORBA::Exception& ex) {
        std::cerr << "Erreur CORBA en C++ : " << ex._name() << std::endl;
    }

    
        
    

    return 0;
}
