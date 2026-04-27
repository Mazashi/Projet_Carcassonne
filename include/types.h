#ifndef TYPES_H
#define TYPES_H


// Constantes pour le plateau et les limites du jeu
#define PLATEAU_TAILLE 145  // Permet de s'étendre de 72 tuiles dans chaque direction
#define CENTRE_PLATEAU 72   // Coordonnée de la tuile de départ (72, 72)
#define MAX_JOUEURS    5
#define NB_PARTISANS_MAX 7
#define TOTAL_TUILES   72

/* --- ENUMERATIONS --- */

// Types de zones présentes sur les faces ou au centre d'une tuile
typedef enum {
    PLAINE,
    ROUTE,
    VILLE,
    ABBAYE,
    VIDE       // Utile pour initialiser ou marquer une absence
} TypeZone;

// Orientations et positions sur une tuile
typedef enum {
    NORD = 0,
    EST  = 1,
    SUD  = 2,
    OUEST = 3,
    CENTRE = 4
} Position;

/* --- STRUCTURES --- */

// Représentation d'une tuile de jeu
typedef struct {
    int id;                 // Identifiant unique (1 à 72)
    TypeZone bords[4];      // [NORD, EST, SUD, OUEST]
    TypeZone centre;        // Type de zone au milieu de la tuile
    int nbBlasons;          // Nombre de blasons (souvent 0 ou 1)
    int rotation;           // Angle actuel (0, 90, 180, 270)
} Tuile;

// Représentation d'un partisan (Meeple) posé sur le plateau
typedef struct {
    int idJoueur;           // ID du propriétaire (-1 si aucun)
    Position emplacement;   // Où est-il sur la tuile (NORD, SUD, CENTRE, etc.)
} Partisan;

// Représentation d'un joueur
typedef struct {
    int id;                 // ID du joueur (1 à 5)
    int score;              // Points accumulés
    int partisansRestants;  // Stock de partisans (démarre à 7)
    char nom[32];           // Nom du joueur
} Joueur;

// Structure du Plateau (Grille de jeu)
typedef struct {
    Tuile* grille[PLATEAU_TAILLE][PLATEAU_TAILLE];    // Stocke les tuiles posées
    Partisan* occupation[PLATEAU_TAILLE][PLATEAU_TAILLE]; // Stocke les pions posés
} Plateau;

// Structure globale de contrôle (Game State)
typedef struct {
    Plateau plateau;
    Joueur joueurs[MAX_JOUEURS];
    int nbJoueurs;
    int tourActuel;         // Index du joueur dont c'est le tour
    Tuile pioche[TOTAL_TUILES];
    int indexProchaineTuile;
} GameState;

#endif // TYPES_H