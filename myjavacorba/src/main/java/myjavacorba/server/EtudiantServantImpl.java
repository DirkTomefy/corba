package myjavacorba.server;

import EtudiantApp.*;

import java.io.*;
import java.util.*;
import java.util.concurrent.CopyOnWriteArrayList;

public class EtudiantServantImpl extends EtudiantServicePOA {

    private static final String FICHIER = "etudiants.txt";
    private static final Object LOCK = new Object();

    private final List<Etudiant> etudiants = new CopyOnWriteArrayList<>();

    public EtudiantServantImpl() throws IOException {
        loadEtudiants();
    }

    private void loadEtudiants() throws IOException {
        File f = new File(FICHIER);
        if (!f.exists()) {
            System.out.println("Fichier " + FICHIER + " introuvable, création...");
            f.createNewFile();
            return;
        }

        try (BufferedReader br = new BufferedReader(new FileReader(f))) {
            String ligne;
            while ((ligne = br.readLine()) != null) {
                ligne = ligne.trim();
                if (ligne.isEmpty() || ligne.startsWith("#")) continue;

                String[] parts = ligne.split(";");
                if (parts.length < 5) {
                    System.err.println("Ligne ignoree (format invalide) : " + ligne);
                    continue;
                }

                Etudiant e = new Etudiant();
                e.id     = Integer.parseInt(parts[0].trim());
                e.numEtu = parts[1].trim();
                e.nom    = parts[2].trim();
                e.prenom = parts[3].trim();
                e.email  = parts[4].trim();
                etudiants.add(e);
            }
        }
        System.out.println("Etudiants charges : " + etudiants.size());
    }

    private void saveAll() {
        try (BufferedWriter bw = new BufferedWriter(new FileWriter(FICHIER, false))) {
            for (Etudiant e : etudiants) {
                bw.write(e.id + ";" + e.numEtu + ";" + e.nom
                        + ";" + e.prenom + ";" + e.email);
                bw.newLine();
            }
        } catch (IOException ex) {
            System.err.println("Erreur ecriture " + FICHIER + " : " + ex.getMessage());
        }
    }

    @Override
    public Etudiant[] getAll() {
        HistoriqueJavaServantImpl.historize(
            "GET_ALL_ETUDIANTS",
            "Nombre=" + etudiants.size()
        );
        return etudiants.toArray(new Etudiant[0]);
    }

    @Override
    public Etudiant getByNumEtu(String numEtu) {
        for (Etudiant e : etudiants) {
            if (e.numEtu.equals(numEtu)) {
                HistoriqueJavaServantImpl.historize(
                    "GET_ETUDIANT_BY_NUM",
                    "numEtu=" + numEtu
                );
                return e;
            }
        }

        HistoriqueJavaServantImpl.historize(
            "GET_ETUDIANT_BY_NUM",
            "numEtu=" + numEtu + " -> introuvable"
        );

        Etudiant vide = new Etudiant();
        vide.id     = 0;
        vide.numEtu = "";
        vide.nom    = "";
        vide.prenom = "";
        vide.email  = "";
        return vide;
    }

    @Override
    public int addEtudiant(Etudiant etu) {
        synchronized (LOCK) {
            int maxId = 0;
            for (Etudiant e : etudiants) if (e.id > maxId) maxId = e.id;
            etu.id = maxId + 1;
            etudiants.add(etu);
            saveAll();
        }

        HistoriqueJavaServantImpl.historize(
            "ADD_ETUDIANT",
            "id=" + etu.id + ", numEtu=" + etu.numEtu
                + ", nom=" + etu.nom + ", prenom=" + etu.prenom
        );
        return etu.id;
    }

    @Override
    public Etudiant[] filtrer(String colonne, boolean isasc) {
        List<Etudiant> copie = new ArrayList<>(etudiants);

        Comparator<Etudiant> cmp;
        switch (colonne) {
            case "id":     cmp = Comparator.comparingInt(e -> e.id); break;
            case "numEtu": cmp = Comparator.comparing(e -> e.numEtu); break;
            case "nom":    cmp = Comparator.comparing(e -> e.nom); break;
            case "prenom": cmp = Comparator.comparing(e -> e.prenom); break;
            case "email":  cmp = Comparator.comparing(e -> e.email); break;
            default:       cmp = Comparator.comparingInt(e -> e.id);
        }
        if (!isasc) cmp = cmp.reversed();

        copie.sort(cmp);

        HistoriqueJavaServantImpl.historize(
            "FILTRER_ETUDIANTS",
            "colonne=" + colonne + ", asc=" + isasc
                + ", resultats=" + copie.size()
        );

        return copie.toArray(new Etudiant[0]);
    }

    @Override
    public int count() {
        HistoriqueJavaServantImpl.historize(
            "COUNT_ETUDIANTS",
            "total=" + etudiants.size()
        );
        return etudiants.size();
    }
}