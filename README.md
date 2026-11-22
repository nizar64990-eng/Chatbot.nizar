# Pokemon GO pour NumWorks

Cette application est un jeu d'exploration et de capture écrit en C pour les
calculatrices NumWorks compatibles avec le lanceur NumPlay/Epsilon.

## Fichiers

- `main.c` : code complet de l'application
- `PokemonGO.nwa` : application NumWorks compilee

## Controles

- Fleches : se deplacer ou naviguer dans les menus
- OK / EXE : valider
- Back : revenir en arriere ou ouvrir la pause
- Home / On/Off : sauvegarder puis quitter

## Installation

Le code utilise les API officielles suivantes :

- `eadk.h`
- `gfx.h`
- `np_text.h`
- `epsilon_app.h`
- `epsilon_files.h`

Integrez `main.c` dans l'arborescence source d'un SDK NumWorks/NumPlay qui
fournit ces fichiers. Conservez ou adaptez les chemins d'inclusion relatifs
presentes en debut de fichier, puis ajoutez le dossier du jeu aux scripts de
compilation du launcher.

La sauvegarde est stockee sous le nom `pokemongo.sav`.

## Fonctionnalites

- Carte procedurale et deplacement sur grille
- Rencontres sauvages aleatoires
- Combats, captures, potions et fuite
- Experience et evolutions
- Equipe de six Pokemon maximum
- Pokedex et inventaire
- Sauvegarde locale avec verification par checksum