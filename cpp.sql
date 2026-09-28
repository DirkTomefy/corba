-- =====================================================
--  Notes : base de données côté C++ (ex-étudiants)
-- =====================================================

DROP DATABASE IF EXISTS notes_cpp_db;
CREATE DATABASE notes_cpp_db CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE notes_cpp_db;

-- Table des notes (ex-table etudiant)
CREATE TABLE note (
    id        INT AUTO_INCREMENT PRIMARY KEY,
    num_etu   VARCHAR(50)  NOT NULL,
    matiere   VARCHAR(100) NOT NULL,
    valeur    DOUBLE       NOT NULL,
    INDEX idx_num_etu (num_etu)
) ENGINE=InnoDB;

-- Table historique : schéma INCHANGÉ
CREATE TABLE historique (
    id         BIGINT AUTO_INCREMENT PRIMARY KEY,
    action     VARCHAR(100) NOT NULL,
    details    TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

-- Jeu de données de test
INSERT INTO note (num_etu, matiere, valeur) VALUES
('etu003948', 'Mathématiques', 15.5),
('etu003948', 'Physique',      12.0),
('etu003948', 'Informatique',  17.5),
('etu001234', 'Mathématiques', 10.0),
('etu001234', 'Informatique',  14.0);