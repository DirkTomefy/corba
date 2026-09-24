## Remarques sur les types : 
* le suffix `_var` : c'est une type intélligente qui gére automatiquement la durée de vie et la libération de mémoire
## Qu'est ce que c'est que l'IOR :  
IOR : 
## Démarage de corba : 

### initialisation de l'orb
```cpp
CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);
/*
C'est la fonction qui crée et initialise l'ORB (Object Request Broker). L'ORB est le cœur de CORBA : c'est le "bus" qui permet à différents objets de communiquer entre eux, même s'ils sont sur des machines différentes ou écrits dans des langages différents. Elle prend en paramètre argc et argv pour pouvoir lire d'éventuels arguments passés en ligne de commande (comme l'adresse réseau ou le port à utiliser)
*/
```

### initialisation du poa 
```cpp
CORBA::Object_var objPOA = orb->resolve_initial_references("RootPOA");
//! ROOTPOA : POA racine (Portable Object Adapter racine)
/*
Le POA racine : c'est le premier poa tous les autres poa sont ses enfants
*/


PortableServer::POA_var poa = PortableServer::POA::_narrow(objPOA);
/*
#PortableServer::POA
C’est l’interface CORBA qui représente un POA (Portable Object Adapter).
Une référence CORBA de type PortableServer::POA permet d’appeler des opérations comme :
    the_POAManager()
    servant_to_reference()
    reference_to_servant()
    create_POA()

#_narrow :
- elle vérifie si l’objet distant ou local implémente bien l’interface PortableServer::POA 
-(si oui) , elle retourne un PortableServer::POA_ptr 
- (si non) , elle retourne PortableServer::POA::_nil()
*/

PortableServer::POAManager_var pman = poa->the_POAManager();
pman->activate();
/*
poa->the_POAManager() : C'est une méthode de l'interface PortableServer::POA. Elle retourne une référence vers le POAManager associé à ce POA.
Qu'est-ce qu'un POAManager ? C'est le composant qui contrôle le cycle de vie et l'état du POA. Il gère les états suivants :
    HOLDING : Le POA met les requêtes en attente (état par défaut).

    ACTIVE : Le POA traite les requêtes normalement.

    DISCARDING : Le POA rejette les nouvelles requêtes.

    INACTIVE : Le POA ne traite plus rien et détruit les objets

pman->activate() : change l'étape du poa racine de holding à active 
*/
```

### initialisation du servant :
```cpp
//! Instancier le Servant Etudiant et l'enregistrer dans le POA
    EtudiantServant* etudiant_servant = new EtudiantServant();
    EtudiantApp::EtudiantService_var etudiant_ref = etudiant_servant->_this();
/*
-Elle récupère le POA associé au servant
-Si le servant n’est pas encore activé ,  auprès de ce POA
-Elle retourne une référence d’objet CORBA que les clients peuvent utiliser pour appeler des méthodes à distance = celle ci est différent de l'IOR (Interoperable Object Reference)
*/
```
