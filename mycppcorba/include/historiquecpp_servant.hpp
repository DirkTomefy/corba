#ifndef HISTORIQUECPP_SERVANT_HPP
#define HISTORIQUECPP_SERVANT_HPP

#include "etudiant.h"
class HistoriqueCppServant : public POA_EtudiantApp::HistoriqueCppService {
public:
    EtudiantApp::HistoriqueCppList* getAllHistorique();
};

#endif