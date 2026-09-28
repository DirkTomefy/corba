package myjavacorba.server;

import EtudiantApp.*;
import java.io.*;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.stream.Collectors;

public class NoteServantImpl extends NoteServicePOA {

    private final List<Note> notes = new CopyOnWriteArrayList<>();

    public NoteServantImpl() throws IOException {
        loadNotesFromResource("/notes.txt");
    }

    private void loadNotesFromResource(String resourcePath) throws IOException {
        try (InputStream is = getClass().getResourceAsStream(resourcePath)) {
            if (is == null) {
                throw new IOException("Ressource introuvable dans le classpath : "
                                      + resourcePath);
            }

            try (BufferedReader reader = new BufferedReader(
                     new InputStreamReader(is, StandardCharsets.UTF_8))) {

                List<String> lignes = reader.lines().collect(Collectors.toList());
                for (String ligne : lignes) {
                    ligne = ligne.trim();
                    if (ligne.isEmpty()) continue;

                    String[] parts = ligne.split(";");
                    if (parts.length != 3) {
                        System.err.println("Ligne ignoree (format invalide) : " + ligne);
                        continue;
                    }

                    Note n = new Note();
                    n.numEtu  = parts[0].trim();
                    n.matiere = parts[1].trim();
                    n.valeur  = Double.parseDouble(parts[2].trim());
                    notes.add(n);
                }
            }
        }
        System.out.println("Notes chargees : " + notes.size());
    }

    @Override
    public void addNote(Note n) {
        HistoriqueJavaServantImpl.historize(
            "ADD_NOTE",
            "numEtu=" + n.numEtu
                + ", matiere=" + n.matiere
                + ", valeur=" + n.valeur
        );

        notes.add(n);
        System.out.println("[RECU] " + n.numEtu + " | " + n.matiere + " | " + n.valeur);
    }

    @Override
    public Note[] getAllNotes() {
        HistoriqueJavaServantImpl.historize(
            "GET_ALL_NOTES",
            "Nombre de notes retournees=" + notes.size()
        );

        return notes.toArray(new Note[0]);
    }

    @Override
    public Note[] getNotesByEtu(String numEtu) {
        List<Note> resultat = new ArrayList<>();
        for (Note n : notes) {
            if (n.numEtu.equals(numEtu)) {
                resultat.add(n);
            }
        }

        HistoriqueJavaServantImpl.historize(
            "GET_NOTES_BY_ETU",
            "numEtu=" + numEtu + ", resultats=" + resultat.size()
        );

        return resultat.toArray(new Note[0]);
    }

    @Override
    public double getMoyenne(String numEtu) {
        double somme = 0;
        int compte = 0;
        for (Note n : notes) {
            if (n.numEtu.equals(numEtu)) {
                somme += n.valeur;
                compte++;
            }
        }
        double moyenne = compte == 0 ? 0.0 : somme / compte;

        HistoriqueJavaServantImpl.historize(
            "GET_MOYENNE",
            "numEtu=" + numEtu
                + ", matiere_count=" + compte
                + ", moyenne=" + moyenne
        );

        return moyenne;
    }
}