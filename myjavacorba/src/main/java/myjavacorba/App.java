package myjavacorba;

import EtudiantApp.*;

import java.util.Scanner;

import org.omg.CORBA.ORB;
import org.omg.CosNaming.*;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAHelper;
import myjavacorba.server.EtudiantServantImpl;
import myjavacorba.server.HistoriqueJavaServantImpl;

public class App {

    //! Questions posées côté Java aux services C++

    public static void askNoteQuestions(NoteService noteService, Scanner scanner) {
        while (true) {
            System.out.print("Voulez-vous consulter les notes (C++) ? (oui/non) : ");
            String reponse = scanner.nextLine();

            if (reponse.equalsIgnoreCase("non")) {
                System.out.println("Fin de la consultation des notes.");
                break;
            }
            if (!reponse.equalsIgnoreCase("oui")) {
                System.out.println("Réponse invalide. Veuillez répondre par 'oui' ou 'non'.");
                continue;
            }

            System.out.print("Entrez le numéro étudiant : ");
            String numEtu = scanner.nextLine();

            try {
                Note[] notes = noteService.getNotesByEtu(numEtu);
                System.out.println("[Java] Notes reçues depuis C++ (MySQL) :");
                for (Note n : notes) {
                    System.out.println("   - Matiere: " + n.matiere
                            + " | Valeur: " + n.valeur);
                }

                double moyenne = noteService.getMoyenne(numEtu);
                System.out.println("[Java] Moyenne pour " + numEtu
                        + " : " + moyenne + "/20");
            } catch (Exception ex) {
                System.err.println("[Java] Erreur getNotesByEtu/getMoyenne : "
                        + ex.getMessage());
            }
        }
    }

    public static void askHistoriqueCppQuestions(HistoriqueCppService historiqueService, Scanner scanner) {
        while (true) {
            System.out.print("Voulez-vous afficher l'historique C++ des actions ? (oui/non) : ");
            String reponse = scanner.nextLine();

            if (reponse.equalsIgnoreCase("oui")) {
                HistoriqueCpp[] liste = historiqueService.getAllHistorique();
                System.out.println("[Java] Liste de l'historique C++ (MySQL) :");
                for (HistoriqueCpp h : liste) {
                    System.out.println("   - ID: " + h.id + " | Action: " + h.action
                            + " | Details: " + h.details
                            + " | Created At: " + h.created_at);
                }
            } else if (reponse.equalsIgnoreCase("non")) {
                System.out.println("Fin de l'affichage de l'historique C++.");
                break;
            } else {
                System.out.println("Réponse invalide. Veuillez répondre par 'oui' ou 'non'.");
            }
        }
    }

    //! Menu Java : appelle NoteService (C++) et HistoriqueCppService (C++)

    public static void askQuestions(Scanner scanner, NamingContextExt ncRef) {
        try {
            //! Recherche des services C++
            org.omg.CORBA.Object noteObj = ncRef.resolve_str("NoteService");
            NoteService noteService = NoteServiceHelper.narrow(noteObj);

            org.omg.CORBA.Object histoObj = ncRef.resolve_str("HistoriqueCppService");
            HistoriqueCppService historiqueService = HistoriqueCppServiceHelper.narrow(histoObj);

            if (noteService == null || historiqueService == null) {
                System.out.println("[Java] Impossible de se connecter aux services C++.");
                return;
            }

            while (true) {
                System.out.println("\n================= MENU =================");
                System.out.println("1. Consulter les notes (service C++ / MySQL)");
                System.out.println("2. Afficher l'historique C++ (MySQL)");
                System.out.println("3. Quitter");
                System.out.print("Votre choix : ");

                String choix = scanner.nextLine();

                if (choix.equals("1")) {
                    askNoteQuestions(noteService, scanner);
                } else if (choix.equals("2")) {
                    askHistoriqueCppQuestions(historiqueService, scanner);
                } else if (choix.equals("3") || choix.equals("q") || choix.equals("quit")) {
                    System.out.println("Fin du menu.");
                    break;
                } else {
                    System.out.println("Choix invalide.");
                }
            }
        } catch (Exception e) {
            System.err.println("[Java] Erreur lors de la communication avec les services C++ : "
                    + e.getMessage());
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

            //! Instancier et enregistrer EtudiantService (Java)
            EtudiantServantImpl etuServant = new EtudiantServantImpl();
            org.omg.CORBA.Object refEtu = rootpoa.servant_to_reference(etuServant);
            EtudiantService hrefEtu = EtudiantServiceHelper.narrow(refEtu);
            NameComponent pathEtu[] = ncRef.to_name("EtudiantService");
            ncRef.rebind(pathEtu, hrefEtu);
            System.out.println("[Java] EtudiantService enregistre dans le NameService.");

            //! Instancier et enregistrer HistoriqueJavaService (Java)
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