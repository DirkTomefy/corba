package myjavacorba;

import EtudiantApp.*;

import java.util.Scanner;

import org.omg.CORBA.ORB;
import org.omg.CosNaming.*;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAHelper;
import myjavacorba.server.NoteServantImpl;
import myjavacorba.server.HistoriqueJavaServantImpl;

public class App {

    //! Questions posées côté Java aux services C++
    public static void askHistoriqueQuestions(HistoriqueCppService historiqueService, Scanner scanner) {
        while (true) {
            System.out.print("Voulez-vous afficher l'historique des actions (C++) ? (oui/non) : ");
            String reponse = scanner.nextLine();

            if (reponse.equalsIgnoreCase("oui")) {
                HistoriqueCpp[] liste = historiqueService.getAllHistorique();
                System.out.println("[Java] Liste de l'historique reçue depuis C++ (MySQL) :");
                for (HistoriqueCpp h : liste) {
                    System.out.println("   - ID: " + h.id + " | Action: " + h.action
                            + " | Details: " + h.details + " | Created At: " + h.created_at);
                }
            } else if (reponse.equalsIgnoreCase("non")) {
                System.out.println("Fin de l'affichage de l'historique C++.");
                break;
            } else {
                System.out.println("Réponse invalide. Veuillez répondre par 'oui' ou 'non'.");
            }
        }
    }

    public static void askFilterQuestions(EtudiantService etudiantService, Scanner scanner) {
        while (true) {
            System.out.print("Voulez-vous filtrer les étudiants ? (oui/non) : ");
            String reponse = scanner.nextLine();
            if (reponse.equalsIgnoreCase("non")) {
                System.out.println("Fin de l'affichage des étudiants.");
                break;
            } else if (!reponse.equalsIgnoreCase("oui")) {
                System.out.println("Réponse invalide. Veuillez répondre par 'oui' ou 'non'.");
                continue;
            }
            System.out.print("Entrez la colonne pour filtrer (id, numEtu, nom, prenom, email) : ");
            String colonne = scanner.nextLine();

            System.out.print("Entrez l'ordre (true pour ascendant, false pour descendant) : ");
            boolean isasc = Boolean.parseBoolean(scanner.nextLine());

            Etudiant[] liste = etudiantService.filtrer(colonne, isasc);
            System.out.println("[Java] Liste des etudiants reçue depuis C++ (MySQL) :");
            for (Etudiant e : liste) {
                System.out.println("   - ID: " + e.id + " | Num: " + e.numEtu
                        + " | " + e.nom + " " + e.prenom + " (" + e.email + ")");
            }
        }
    }

    //! Questions posées côté Java aux services C++
    public static void askQuestions(Scanner scanner, NamingContextExt ncRef) {
        try {
            //! Recherche du service C++ "EtudiantService"
            org.omg.CORBA.Object etuObj = ncRef.resolve_str("EtudiantService");
            EtudiantService etudiantService = EtudiantServiceHelper.narrow(etuObj);

            //! Recherche du service C++ "HistoriqueCppService"
            org.omg.CORBA.Object histoObj = ncRef.resolve_str("HistoriqueCppService");
            HistoriqueCppService historiqueService = HistoriqueCppServiceHelper.narrow(histoObj);

            if (etudiantService != null && historiqueService != null) {
                askFilterQuestions(etudiantService, scanner);
                askHistoriqueQuestions(historiqueService, scanner);
            } else {
                System.out.println("[Java] Impossible de se connecter aux services C++.");
            }
        } catch (Exception e) {
            System.err.println("[Java] Erreur lors de la communication avec les services C++ : " + e.getMessage());
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        try {
            //! Initialiser l'ORB Java
            ORB orb = ORB.init(args, null);

            //! Activer le RootPOA
            POA rootpoa = POAHelper.narrow(orb.resolve_initial_references("RootPOA"));
            rootpoa.the_POAManager().activate();

            //! Se connecter au Service de Nommage (CosNaming)
            org.omg.CORBA.Object objRef = orb.resolve_initial_references("NameService");
            NamingContextExt ncRef = NamingContextExtHelper.narrow(objRef);

            //! Instancier et enregistrer NoteService
            NoteServantImpl noteServant = new NoteServantImpl();
            org.omg.CORBA.Object refNote = rootpoa.servant_to_reference(noteServant);
            NoteService hrefNote = NoteServiceHelper.narrow(refNote);
            NameComponent pathNote[] = ncRef.to_name("NoteService");
            ncRef.rebind(pathNote, hrefNote);
            System.out.println("[Java] NoteService enregistre dans le NameService.");

            //! Instancier et enregistrer HistoriqueJavaService
            HistoriqueJavaServantImpl histoServant = new HistoriqueJavaServantImpl();
            org.omg.CORBA.Object refHisto = rootpoa.servant_to_reference(histoServant);
            HistoriqueJavaService hrefHisto = HistoriqueJavaServiceHelper.narrow(refHisto);
            NameComponent pathHisto[] = ncRef.to_name("HistoriqueJavaService");
            ncRef.rebind(pathHisto, hrefHisto);
            System.out.println("[Java] HistoriqueJavaService enregistre dans le NameService.");

            //! PAUSE INTERACTIVE
            System.out.println("\n====================================================");
            System.out.println("Assurez-vous que le serveur C++ est AUSSI lance.");
            System.out.println("Appuyez sur ENTRÉE pour executer la requete vers C++...");
            System.out.println("====================================================\n");
            System.in.read();

            //! Interroger les services C++
            askQuestions(new Scanner(System.in), ncRef);

            System.out.println("[Java] Serveur Java pret et en ecoute...");
            orb.run();

        } catch (Exception e) {
            System.err.println("Erreur CORBA en Java : " + e.getMessage());
            e.printStackTrace();
        }
    }
}