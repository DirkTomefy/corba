CREATE DATABASE IF NOT EXISTS corbatest
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE corbatest;

CREATE TABLE IF NOT EXISTS etudiant (
    id        INT AUTO_INCREMENT PRIMARY KEY,
    num_etu   VARCHAR(20)  NOT NULL UNIQUE,
    nom       VARCHAR(50)  NOT NULL,
    prenom    VARCHAR(50)  NOT NULL,
    email     VARCHAR(100),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Quelques données de test
INSERT INTO etudiant (num_etu, nom, prenom, email) VALUES
('etu003948', 'Tomefy', 'Rakoto', 'tomefy@example.com'),
('etu003949', 'Dupont', 'Marie',   'marie@example.com'),
('etu003950', 'Martin', 'Paul',    'paul@example.com');

CREATE TABLE IF NOT EXISTS historique (
    id        INT AUTO_INCREMENT PRIMARY KEY,
    action    VARCHAR(100) NOT NULL,
    details   TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);