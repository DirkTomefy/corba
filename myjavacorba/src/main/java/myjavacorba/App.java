package myjavacorba;
import EtudiantApp.*;

import java.util.Scanner;

import org.omg.CORBA.ORB;
import org.omg.CosNaming.*;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAHelper;
import myjavacorba.server.NoteServantImpl;

public class App {
   public static void main(String[] args) {
        try {
            //! Initialiser l'ORB Java
            ORB orb = ORB.init(args, null);

            //! Activer le RootPOA
            POA rootpoa = POAHelper.narrow(orb.resolve_initial_references("RootPOA"));
            rootpoa.the_POAManager().activate();

            //! Instancier le Servant Java Note
            NoteServantImpl noteServant = new NoteServantImpl();
            org.omg.CORBA.Object ref = rootpoa.servant_to_reference(noteServant);
            NoteService href = NoteServiceHelper.narrow(ref);

            //! Se connecter au Service de Nommage (CosNaming)
            org.omg.CORBA.Object objRef = orb.resolve_initial_references("NameService");
            NamingContextExt ncRef = NamingContextExtHelper.narrow(objRef);

            //!Enregistrer "NoteService"
            NameComponent path[] = ncRef.to_name("NoteService");
            ncRef.rebind(path, href);
            System.out.println("[Java] NoteService enregistre dans le NameService.");

            //? PAUSE INTERACTIVE 
            System.out.println("\n====================================================");
            System.out.println("Assurez-vous que le serveur C++ est AUSSI lance.");
            System.out.println("Appuyez sur ENTRÉE pour executer la requete vers C++...");
            System.out.println("====================================================\n");
            System.in.read();


            System.out.println("[Java] Recherche du service C++ 'EtudiantService'...");
            org.omg.CORBA.Object etuObj = ncRef.resolve_str("EtudiantService");
            EtudiantService etudiantService = EtudiantServiceHelper.narrow(etuObj);

            if (etudiantService != null) {
                Scanner scanner = new Scanner(System.in);
                while(true){
                System.out.print("Entrez la colonne pour filtrer (id, numEtu, nom, prenom, email) : ");
                String colonne = scanner.nextLine();

                System.out.print("Entrez l'ordre (true pour ascendant, false pour descendant) : ");
                boolean isasc = Boolean.parseBoolean(scanner.nextLine());

                Etudiant[] liste = etudiantService.filtrer(colonne,isasc);
                System.out.println("[Java] Liste des etudiants reçue depuis C++ (MySQL) :");
                for (Etudiant e : liste) {
                    System.out.println("   - ID: " + e.id + " | Num: " + e.numEtu 
                                       + " | " + e.nom + " " + e.prenom + " (" + e.email + ")");
                }
                }
            }

            System.out.println("[Java] Serveur Java pret et en ecoute...");
            orb.run();

        } catch (Exception e) {
            System.err.println("Erreur CORBA en Java : " + e.getMessage());
            e.printStackTrace();
        }
    }
}
