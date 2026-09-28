package myjavacorba.server;

import EtudiantApp.HistoriqueJava;
import EtudiantApp.HistoriqueJavaServicePOA;

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public class HistoriqueJavaServantImpl extends HistoriqueJavaServicePOA {

    private static final String FICHIER = "historique.txt";
    private static final Object LOCK    = new Object();

    private static void ensureFileExists() {
        File f = new File(FICHIER);
        if (f.exists()) return;

        try (InputStream is = HistoriqueJavaServantImpl.class
                                 .getResourceAsStream("/" + FICHIER)) {
            if (is == null) {
                System.err.println("Ressource introuvable : /" + FICHIER
                        + " (le fichier sera cree a la premiere ecriture)");
                return;
            }
            try (OutputStream os = new FileOutputStream(f)) {
                is.transferTo(os);
            }
            System.out.println("Fichier initialise depuis /" + FICHIER
                    + " -> " + FICHIER);
        } catch (IOException e) {
            System.err.println("Erreur copie /" + FICHIER + " : " + e.getMessage());
        }
    }

    public static HistoriqueJava historize(String action, String details) {
        synchronized (LOCK) {
            ensureFileExists();

            long nextId = 1;
            File f = new File(FICHIER);
            if (f.exists()) {
                try (BufferedReader br = new BufferedReader(
                        new InputStreamReader(new FileInputStream(f),
                                              StandardCharsets.UTF_8))) {

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

            try (BufferedWriter bw = new BufferedWriter(
                    new OutputStreamWriter(new FileOutputStream(FICHIER, true),
                                           StandardCharsets.UTF_8))) {
                bw.write(h.id + ";" + h.action + ";" + h.details + ";" + h.created_at);
                bw.newLine();
            } catch (IOException e) {
                System.err.println("Erreur ecriture " + FICHIER + " : " + e.getMessage());
            }

            return h;
        }
    }

    @Override
    public HistoriqueJava[] getAllHistorique() {
        synchronized (LOCK) {
            ensureFileExists();

            List<HistoriqueJava> liste = new ArrayList<>();

            try (BufferedReader br = new BufferedReader(
                    new InputStreamReader(new FileInputStream(FICHIER),
                                          StandardCharsets.UTF_8))) {

                String ligne;
                while ((ligne = br.readLine()) != null) {
                    ligne = ligne.trim();
                    if (ligne.isEmpty() || ligne.startsWith("#")) continue;

                    String[] parts = ligne.split(";");
                    if (parts.length < 4) continue;

                    HistoriqueJava h = new HistoriqueJava();
                    h.id         = Integer.parseInt(parts[0].trim());
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
}