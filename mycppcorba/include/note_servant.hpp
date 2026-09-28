#ifndef ETUDIANT_SERVANT_HPP
#define ETUDIANT_SERVANT_HPP

#include "etudiant.h"   
class NoteServant : public POA_EtudiantApp::NoteService {
public:
    EtudiantApp::NoteList* getAllNotes();
    EtudiantApp::NoteList* getNotesByEtu(const char* numEtu);
    void                     addNote(const EtudiantApp::Note& n);
    CORBA::Double            getMoyenne(const char* numEtu);
    EtudiantApp::EtudiantList* filtrer(const char* colonne,const bool isasc);
};

#endif