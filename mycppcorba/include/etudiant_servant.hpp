#ifndef ETUDIANT_SERVANT_HPP
#define ETUDIANT_SERVANT_HPP

#include "etudiant.h"   
class EtudiantServant : public POA_EtudiantApp::EtudiantService {
public:
    EtudiantApp::EtudiantList* getAll();
    EtudiantApp::Etudiant*     getByNumEtu(const char* numEtu);
    CORBA::Long                addEtudiant(const EtudiantApp::Etudiant& e);
    CORBA::Long                count();
};

#endif