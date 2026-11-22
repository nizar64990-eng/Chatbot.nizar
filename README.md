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

- Monde procedural avec routes, eau, montagnes, quartiers urbains et parcs
- 16 Pokemon originaux et Pokedex detaille
- Pokemon visibles dans les zones explorees
- Rencontres sauvages adaptees au biome
- Combats, captures, potions et fuite
- Experience et evolutions
- Equipe de six Pokemon maximum
- Pokedex complet, sac, niveau, PV, XP et inventaire
- Poudre et pieces de progression
- Missions quotidiennes et recompenses
- PokeStops proceduraux avec rotation d'inventaire
- Boutique interne avec achats en pieces
- Cycle jour/nuit et effets de meteo proceduraux
- Graphismes vectoriels proceduraux avec ombres, reflexions et decor detaille
- 256x256 cases procedurales reparties en 16 regions
- Nouveau depart avec seed aleatoire par partie
- Pokedex regional de 64 Pokemon, informations verrouillees avant capture
- Niveau joueur, XP, Pokedollars et recompenses de quetes
- Profil, pseudos, skins et personnalisation des Pokeballs
- Duellistes diriges par des bots
- Brulure, sommeil, poison, paralysie, resistances et faiblesses
- Capacites X, Y, Z, V, VMAX, EX, Mega EX, Teracristal, Gigamax et Dynamax
- Centres Pokemon, magasins, villes, PokeStops, baies et potions
- Badges de captures et d'arènes
- Sauvegarde locale avec verification par checksum