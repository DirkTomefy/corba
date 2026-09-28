package myjavacorba.server;

import EtudiantApp.HistoriqueJava;
import EtudiantApp.HistoriqueJavaServicePOA;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class HistoriqueJavaServantImpl extends HistoriqueJavaServicePOA {

    private static final String FICHIER = "historique.txt";

     public static HistoriqueJava historize(String action, String details) {
            long nextId = 1;
            File f = new File(FICHIER);
            if (f.exists()) {
                try (BufferedReader br = new BufferedReader(new FileReader(f))) {
                    String ligne;
                    long maxId = 0;
                    while ((ligne = br.readLine()) != null) {
                        ligne = ligne.trim();
                        if (ligne.isEmpty() || ligne.startsWith("#")) continue;
                        String[] parts = ligne.split(";");
                        if (parts.length < 1) continue;
                        try {
                            long id = Long.parseLong(parts[0].trim());
                            if (id > maxId) maxId = id;
                        } catch (NumberFormatException ignored) { }
                    }
                    nextId = maxId + 1;
                } catch (IOException e) {
                    System.err.println("Erreur lecture " + FICHIER + " : " + e.getMessage());
                }
            }

            HistoriqueJava h = new HistoriqueJava();
            h.id         = (int) nextId;
            h.action     = action;
            h.details    = details;
            h.created_at = System.currentTimeMillis(); 

            
            try (BufferedWriter bw = new BufferedWriter(new FileWriter(FICHIER, true))) {
                bw.write(h.id + ";" + h.action + ";" + h.details + ";" + h.created_at);
                bw.newLine();
            } catch (IOException e) {
                System.err.println("Erreur écriture " + FICHIER + " : " + e.getMessage());
            }
            
            return h;   
    }

    @Override
    public HistoriqueJava[] getAllHistorique() {
        List<HistoriqueJava> liste = new ArrayList<>();

        try (BufferedReader br = new BufferedReader(new FileReader(FICHIER))) {
            String ligne;
            while ((ligne = br.readLine()) != null) {
                ligne = ligne.trim();
                if (ligne.isEmpty() || ligne.startsWith("#")) continue;

                String[] parts = ligne.split(";");
                if (parts.length < 4) continue; 

                HistoriqueJava h = new HistoriqueJava();
                h.id         =  Integer.parseInt(parts[0].trim());
                h.action     = parts[1].trim();
                h.details    = parts[2].trim();
                h.created_at = Long.parseLong(parts[3].trim());

                liste.add(h);
            }
        } catch (IOException e) {
            System.err.println("Erreur lecture " + FICHIER + " : " + e.getMessage());
        } catch (NumberFormatException e) {
            System.err.println("Format numérique invalide : " + e.getMessage());
        }

        return liste.toArray(new HistoriqueJava[0]);
    }
}