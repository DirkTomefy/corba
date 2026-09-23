package myjavacorba.server;

import EtudiantApp.*;
import org.omg.CORBA.*;
import org.omg.CosNaming.*;
import org.omg.PortableServer.*;
import java.util.Properties;

public class NoteServer {

    public static void main(String[] args) {
        try {
            Properties props = new Properties();
            props.put("ORBInitRef.NameService",
                      "corbaloc::localhost:2809/NameService");

            ORB orb = ORB.init(args, props);

            POA rootPOA = POAHelper.narrow(
                orb.resolve_initial_references("RootPOA"));

            // Constructeur sans argument (charge la ressource du classpath)
            NoteServantImpl servant = new NoteServantImpl();

            byte[] id = rootPOA.activate_object(servant);
            org.omg.CORBA.Object ref = rootPOA.id_to_reference(id);

            org.omg.CORBA.Object nsObj =
                orb.resolve_initial_references("NameService");
            NamingContextExt nc = NamingContextExtHelper.narrow(nsObj);

            nc.rebind(nc.to_name("NoteService"), ref);

            System.out.println("NoteService enregistre aupres du Naming Service.");
            System.out.println("Serveur Java pret. Ctrl+C pour quitter.");

            rootPOA.the_POAManager().activate();
            orb.run();

        } catch (Exception e) {
            System.err.println("Erreur au demarrage : " + e);
            e.printStackTrace();
        }
    }
}