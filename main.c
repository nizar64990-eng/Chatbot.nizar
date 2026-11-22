
/*
 * POKEMON GO
 * Jeu original pour NumWorks / NumPlay
 *
 * Un seul fichier : main.c
 * Moteur : API graphique officielle NumPlay
 * Ecran : 320 x 240
 *
 * Inspiration : Pokemon GO.
 * Pokemon et Pokemon GO appartiennent a leurs
 * proprietaires respectifs. Ce projet est independant
 * et n'est ni affilie ni approuve par Niantic,
 * Nintendo ou The Pokemon Company.
 */

#include <eadk.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../../launcher/src/gfx.h"
#include "../../../games/common/np_text.h"

#if NP_MODULE

#define np_app_begin()
#define np_app_end() 0

#else

#include "../../../games/common/epsilon_app.h"
#include "../../../games/common/epsilon_files.h"

#define np_file_read ef_read
#define np_file_write ef_write

#ifndef HOST
const char eadk_app_name[]
__attribute__((section(".rodata.eadk_app_name"))) = "Pokemon GO";

const uint32_t eadk_api_level
__attribute__((section(".rodata.eadk_api_level"))) = 0;
#endif

void np_push(int x, int y, int w, int h, const color_t *px) {
  eadk_display_push_rect(
    (eadk_rect_t){
      (uint16_t)x,
      (uint16_t)y,
      (uint16_t)w,
      (uint16_t)h
    },
    px
  );
}

#endif

/* =========================================================
   CONFIGURATION
   ========================================================= */

#define KEY(k) (1ull << (k))

#define TICK_MS 33

#define PARTY_MAX 6
#define SPECIES_COUNT 151

#define WORLD_SIZE 256

#define SAVE_NAME "creaturego.sav"

#define C_BG       HEX(0x101B2B)
#define C_PANEL    HEX(0x1D3045)
#define C_PANEL2   HEX(0x29445C)
#define C_WHITE    HEX(0xF4F7F9)
#define C_MUTED    HEX(0xA8BAC8)
#define C_GREEN    HEX(0x54D68B)
#define C_BLUE     HEX(0x51B9ED)
#define C_YELLOW   HEX(0xFFD45A)
#define C_RED      HEX(0xF06A70)
#define C_PURPLE   HEX(0xB18AFF)
#define C_DARK     HEX(0x0B1420)
#define C_ORANGE   HEX(0xF39C4A)

#define HUD_H 23

/* =========================================================
   STRUCTURES
   ========================================================= */

typedef struct __attribute__((packed)) {
  uint8_t species;
  uint8_t level;
  uint8_t xp;
  uint8_t status;
} Creature;

typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint8_t version;
  uint8_t party_count;
  uint8_t balls;
  uint8_t potions;

  int16_t world_x;
  int16_t world_y;

  uint32_t dex[5];
  uint32_t steps;
  uint32_t stardust;
  uint32_t coins;
  uint32_t captured;
  uint32_t distance;
  uint16_t quest_catches;
  uint16_t quest_steps;
  uint8_t quest_claimed;
  uint32_t seed;
  uint32_t player_xp;
  uint16_t player_level;
  uint32_t pokedollars;
  uint8_t ball_style;
  uint8_t skin_style;
  uint8_t berries;
  uint16_t badges;
  uint16_t bot_wins;
  char nickname[12];
  uint8_t team;
  uint8_t egg_type;
  uint16_t egg_distance;
  uint16_t egg_target;
  uint8_t buddy_species;
  uint16_t buddy_hearts;
  uint8_t gifts;
  uint8_t raid_tier;
  uint8_t raid_wins;
  uint8_t photos;
  uint8_t routes;
  uint8_t trades;
  uint8_t research_points;

  Creature party[PARTY_MAX];

  uint32_t checksum;
} SaveData;

typedef struct {
  const char *name;
  const char *description;

  color_t color;

  uint8_t evolution;
  uint8_t evolution_level;

  uint8_t base_hp;
  uint8_t base_attack;
} Species;

/* =========================================================
   CREATURES ORIGINALES
   ========================================================= */

static const Species species[SPECIES_COUNT] = {
  {
    "Flamino",
    "Petit esprit de braise",
    C_RED,
    1, 8, 22, 7
  },

  {
    "Flamox",
    "Gardien des flammes",
    C_YELLOW,
    0, 0, 34, 12
  },

  {
    "Aquali",
    "Pokemon des rivieres",
    C_BLUE,
    3, 9, 25, 6
  },

  {
    "Aquaron",
    "Dragon des profondeurs",
    C_PURPLE,
    0, 0, 39, 11
  },

  {
    "Florin",
    "Petit gardien vegetal",
    C_GREEN,
    5, 8, 24, 7
  },

  {
    "Floragon",
    "Protecteur des forets",
    C_GREEN,
    0, 0, 38, 11
  },

  {
    "Rocub",
    "Pokemon de pierre",
    C_MUTED,
    7, 10, 30, 8
  },

  {
    "Rocor",
    "Titan des montagnes",
    C_YELLOW,
    0, 0, 45, 13
  },
  {
    "Voltini",
    "Petit messager de l'orage",
    C_YELLOW,
    9, 7, 21, 8
  },
  {
    "Voltige",
    "Seigneur des tempetes",
    C_BLUE,
    0, 0, 36, 13
  },
  {
    "Aqualis",
    "Sprite des lacs",
    C_BLUE,
    11, 9, 23, 7
  },
  {
    "Nymphar",
    "Gardienne des eaux profondes",
    C_PURPLE,
    0, 0, 40, 12
  },
  {
    "Feulic",
    "Renard des cendres",
    C_RED,
    13, 8, 24, 8
  },
  {
    "Pyrocer",
    "Garde-feu imperial",
    C_ORANGE,
    0, 0, 42, 14
  },
  {
    "Mousain",
    "Chene miniature",
    C_GREEN,
    15, 10, 22, 6
  },
  {
    "Sylvieux",
    "Esprit des forats anciens",
    C_GREEN,
    0, 0, 38, 12
  },
  {
    "Gelice",
    "Eclat des sommets geles",
    C_BLUE,
    17, 12, 27, 8
  },
  {
    "Polacore",
    "Garde des glaciers",
    C_WHITE,
    0, 0, 43, 13
  },
  {
    "Nebulox",
    "Gardien du ciel nocturne",
    C_PURPLE,
    19, 14, 31, 10
  },
  {
    "Astralion",
    "Messager des etoiles",
    C_YELLOW,
    0, 0, 48, 15
  },
  {
    "Bambrou",
    "Petit panda des bambous",
    C_GREEN,
    21, 11, 26, 7
  },
  {
    "Bamborix",
    "Protecteur des forets sacrées",
    C_YELLOW,
    0, 0, 41, 13
  },
  {
    "Mirmouche",
    "Larve miniature des forets",
    C_GREEN,
    23, 8, 20, 5
  },
  {
    "Phalynx",
    "Papillon aux ailes luminescentes",
    C_PURPLE,
    0, 0, 35, 11
  },
  {
    "Lumosel",
    "Graine brillante du matin",
    C_YELLOW,
    25, 10, 24, 7
  },
  {
    "Helios",
    "Lion solaire",
    C_RED,
    0, 0, 44, 14
  },
  {
    "Umbron",
    "Renard des ombres",
    C_PURPLE,
    27, 12, 25, 9
  },
  {
    "Nyctera",
    "Chasseur nocturne",
    C_BLUE,
    0, 0, 43, 14
  },
  {
    "Terrapin",
    "Tortue des dunes",
    C_MUTED,
    29, 13, 29, 8
  },
  {
    "Dunadar",
    "Seigneur du desert",
    C_ORANGE,
    0, 0, 47, 15
  },
  {
    "Cobaltin",
    "Alliage vivant",
    C_BLUE,
    31, 16, 33, 12
  },
  {
    "Cobaltron",
    "Golem mecanique",
    C_MUTED,
    0, 0, 51, 17
  },
  {
    "Mycélia",
    "Fée champignon",
    C_PURPLE,
    33, 14, 28, 9
  },
  {
    "Sporyon",
    "Ancien de la foret",
    C_GREEN,
    0, 0, 46, 14
  },
  {
    "Carapluie",
    "Crabe des marées",
    C_BLUE,
    35, 11, 30, 10
  },
  {
    "Tidalord",
    "Titan des abysses",
    C_PURPLE,
    0, 0, 49, 16
  },
  {
    "Arcélix",
    "Serpent d'or",
    C_YELLOW,
    37, 15, 32, 11
  },
  {
    "Dracelion",
    "Dragon royal",
    C_RED,
    0, 0, 52, 18
  },
  {
    "Brumelle",
    "Papillon des nuages",
    C_WHITE,
    39, 12, 26, 8
  },
  {
    "Tempestra",
    "Fée des orages",
    C_BLUE,
    0, 0, 45, 15
  },
  {
    "Vaporys",
    "Esprit de vapeur",
    C_BLUE,
    41, 13, 27, 9
  },
  {
    "Fumadora",
    "Machine a vapeur vivante",
    C_MUTED,
    0, 0, 44, 14
  },
  {
    "Luminiche",
    "Petit poisson lumineux",
    C_YELLOW,
    43, 10, 23, 7
  },
  {
    "Eclatide",
    "Poisson des profondeurs",
    C_BLUE,
    0, 0, 42, 13
  },
  {
    "Sablette",
    "Esprit du sable",
    C_ORANGE,
    45, 14, 30, 9
  },
  {
    "Dunese",
    "Garde des dunes",
    C_YELLOW,
    0, 0, 48, 16
  },
  {
    "Cendrel",
    "Aigle de cendre",
    C_RED,
    47, 15, 31, 12
  },
  {
    "Vulcarn",
    "Oiseau volcanique",
    C_ORANGE,
    0, 0, 50, 17
  },
  {
    "Glacipin",
    "Pingouin cristallin",
    C_BLUE,
    49, 12, 29, 9
  },
  {
    "Abyssorin",
    "Empereur des glaces",
    C_WHITE,
    0, 0, 49, 16
  },
  {
    "Sourisole",
    "Coureur des champs",
    C_MUTED,
    51, 10, 24, 8
  },
  {
    "Quickora",
    "L'animal le plus vif",
    C_YELLOW,
    0, 0, 39, 13
  },
  {
    "Cirelune",
    "Oiseau lunaire",
    C_WHITE,
    53, 15, 32, 10
  },
  {
    "Selenera",
    "Gardienne des cycles",
    C_PURPLE,
    0, 0, 53, 18
  },
  {
    "Rocaille",
    "Caillou anime",
    C_MUTED,
    55, 11, 31, 9
  },
  {
    "Titanite",
    "Colosse naturel",
    C_ORANGE,
    0, 0, 54, 18
  },
  {
    "Fleurose",
    "Fleur des prairies",
    C_GREEN,
    57, 12, 27, 8
  },
  {
    "Rosalithe",
    "Rose petrifiee",
    C_PURPLE,
    0, 0, 46, 14
  },
  {
    "Elekido",
    "Jeune gardien electrique",
    C_YELLOW,
    59, 13, 28, 11
  },
  {
    "Electar",
    "Bete de la foudre",
    C_BLUE,
    0, 0, 48, 16
  },
  {
    "Kokoon",
    "Cocon vegetal",
    C_GREEN,
    61, 14, 29, 8
  },
  {
    "Aureon",
    "Creature legendaire dorée",
    C_YELLOW,
    0, 0, 58, 20
  },
  {
    "Alakazam",
    "Esprit psy aux pouvoirs immenses",
    C_PURPLE,
    0, 0, 55, 21
  },
  {
    "Machoc",
    "Combattant musculeux",
    C_MUTED,
    65, 18, 37, 14
  },
  {
    "Machopeur",
    "Champion de puissance",
    C_MUTED,
    67, 28, 42, 17
  },
  {
    "Mackogneur",
    "Colosse des arts martiaux",
    C_MUTED,
    0, 0, 52, 20
  },
  {
    "Chetiflor",
    "Plante carnivore aggripante",
    C_GREEN,
    69, 12, 26, 8
  },
  {
    "Boustiflor",
    "Piege vivant des marais",
    C_GREEN,
    71, 21, 41, 13
  },
  {
    "Empiflor",
    "Predatrice des forets humides",
    C_GREEN,
    0, 0, 50, 16
  },
  {
    "Tentacool",
    "Meduse transparente",
    C_BLUE,
    73, 20, 30, 9
  },
  {
    "Tentacruel",
    "Creature des grands fonds",
    C_PURPLE,
    0, 0, 48, 15
  },
  {
    "Racaillou",
    "Pierre animee",
    C_MUTED,
    75, 25, 33, 10
  },
  {
    "Gravalanch",
    "Avalanche vivante",
    C_MUTED,
    77, 36, 43, 16
  },
  {
    "Grolem",
    "Armure rocheuse colossale",
    C_MUTED,
    0, 0, 52, 20
  },
  {
    "Ponyta",
    "Cheval des flames",
    C_RED,
    79, 30, 29, 12
  },
  {
    "Galopa",
    "Coursier volcanique",
    C_RED,
    0, 0, 44, 17
  },
  {
    "Ramoloss",
    "Paresse des rivages",
    C_PURPLE,
    81, 30, 32, 8
  },
  {
    "Flagadoss",
    "Gardien de la pause",
    C_PURPLE,
    0, 0, 48, 14
  },
  {
    "Magneti",
    "Aimant flottant",
    C_MUTED,
    83, 28, 30, 12
  },
  {
    "Magneton",
    "Trio magnetique",
    C_MUTED,
    0, 0, 45, 16
  },
  {
    "Canarticho",
    "Maître de la feuille",
    C_MUTED,
    0, 0, 34, 13
  },
  {
    "Doduo",
    "Couple a deux tetes",
    C_MUTED,
    86, 31, 31, 12
  },
  {
    "Dodrio",
    "Trio agile",
    C_MUTED,
    0, 0, 46, 18
  },
  {
    "Otaria",
    "Phoque des glaces",
    C_BLUE,
    88, 30, 32, 10
  },
  {
    "Lamantine",
    "Geant des mers froides",
    C_BLUE,
    0, 0, 49, 14
  },
  {
    "Tadmorv",
    "Monstre des decharges",
    C_PURPLE,
    90, 32, 35, 12
  },
  {
    "Grotadmorv",
    "Mare empoisonnée",
    C_PURPLE,
    0, 0, 52, 18
  },
  {
    "Kokiyas",
    "Coquillage profond",
    C_BLUE,
    92, 33, 28, 9
  },
  {
    "Crustabri",
    "Perle vivante",
    C_BLUE,
    0, 0, 44, 15
  },
  {
    "Fantominus",
    "Energie spectrale",
    C_PURPLE,
    94, 25, 27, 12
  },
  {
    "Spectrum",
    "Ombre errante",
    C_PURPLE,
    96, 36, 41, 16
  },
  {
    "Ectoplasma",
    "Esprit legend",
    C_PURPLE,
    0, 0, 49, 20
  },
  {
    "Onix",
    "Serpent de pierre",
    C_MUTED,
    0, 0, 39, 11
  },
  {
    "Soporifik",
    "Illusionneur endormi",
    C_PURPLE,
    99, 26, 30, 10
  },
  {
    "Hypnomade",
    "Maître des reves",
    C_PURPLE,
    0, 0, 44, 16
  },
  {
    "Krabby",
    "Crabe des plages",
    C_RED,
    101, 28, 27, 13
  },
  {
    "Krabboss",
    "Pinces titanesques",
    C_RED,
    0, 0, 43, 19
  },
  {
    "Voltorbe",
    "Sphère electrique",
    C_RED,
    103, 30, 24, 12
  },
  {
    "Electrode",
    "Orbe explosive",
    C_RED,
    0, 0, 39, 18
  },
  {
    "Noeunoeuf",
    "Essaim vegetal",
    C_PURPLE,
    105, 30, 31, 10
  },
  {
    "Noadkoko",
    "Foret consciente",
    C_GREEN,
    0, 0, 50, 17
  },
  {
    "Osselait",
    "Gardien des tombes",
    C_MUTED,
    107, 28, 27, 12
  },
  {
    "Ossatueur",
    "Os ancestral",
    C_MUTED,
    0, 0, 45, 18
  },
  {
    "Kicklee",
    "Combattant a longues jambes",
    C_MUTED,
    0, 0, 43, 19
  },
  {
    "Tygnon",
    "Boxeur invincible",
    C_MUTED,
    0, 0, 44, 18
  },
  {
    "Excelangue",
    "Langue géante",
    C_PURPLE,
    0, 0, 42, 11
  },
  {
    "Smogo",
    "Nuage toxique",
    C_PURPLE,
    113, 35, 30, 10
  },
  {
    "Smogogo",
    "Gaz empoisonne",
    C_PURPLE,
    0, 0, 46, 15
  },
  {
    "Rhinocorne",
    "Chargeur corné",
    C_MUTED,
    115, 42, 39, 17
  },
  {
    "Rhinoferos",
    "Tank vivant",
    C_MUTED,
    0, 0, 58, 22
  },
  {
    "Leveinard",
    "Soigneur du printemps",
    C_WHITE,
    0, 0, 49, 8
  },
  {
    "Saquedeneu",
    "Plante accrocheuse",
    C_GREEN,
    0, 0, 38, 16
  },
  {
    "Kangourex",
    "Protecteur maternel",
    C_MUTED,
    0, 0, 48, 17
  },
  {
    "Hypotrempe",
    "Cheval marin",
    C_BLUE,
    120, 32, 29, 11
  },
  {
    "Hypocean",
    "Dragon marin agile",
    C_BLUE,
    0, 0, 46, 17
  },
  {
    "Poissirene",
    "Poisson corne",
    C_BLUE,
    122, 33, 30, 12
  },
  {
    "Poissoroy",
    "Roi des courants",
    C_PURPLE,
    0, 0, 48, 18
  },
  {
    "Stari",
    "Etoile aquatique",
    C_PURPLE,
    124, 25, 27, 11
  },
  {
    "Staross",
    "Etoile des mers",
    C_PURPLE,
    0, 0, 43, 17
  },
  {
    "Mime Mister",
    "Pantomme mime",
    C_PURPLE,
    0, 0, 34, 14
  },
  {
    "Insecateur",
    "Lame des jardins",
    C_GREEN,
    0, 0, 42, 18
  },
  {
    "Lippoutou",
    "Danseuse des neiges",
    C_PURPLE,
    0, 0, 36, 13
  },
  {
    "Elektek",
    "Poing électrique",
    C_YELLOW,
    0, 0, 39, 18
  },
  {
    "Magmar",
    "Poing volcanique",
    C_RED,
    0, 0, 40, 18
  },
  {
    "Scarabrute",
    "Scarabée de combat",
    C_MUTED,
    0, 0, 44, 19
  },
  {
    "Tauros",
    "Taureau sauvage",
    C_MUTED,
    0, 0, 43, 18
  },
  {
    "Magicarpe",
    "Poisson fragile",
    C_RED,
    133, 20, 20, 5
  },
  {
    "Leviator",
    "Dragon des eaux",
    C_BLUE,
    0, 0, 60, 22
  },
  {
    "Lokhlass",
    "Voyageur des oceans",
    C_BLUE,
    0, 0, 65, 14
  },
  {
    "Metamorph",
    "Creature a mille formes",
    C_PURPLE,
    0, 0, 42, 12
  },
  {
    "Evoli",
    "Evolution instable",
    C_MUTED,
    136, 30, 31, 12
  },
  {
    "Aquali",
    "Evolution aquatique",
    C_BLUE,
    0, 0, 48, 15
  },
  {
    "Voltali",
    "Evolution electrique",
    C_YELLOW,
    0, 0, 48, 16
  },
  {
    "Pyroli",
    "Evolution flamboyante",
    C_RED,
    0, 0, 48, 17
  },
  {
    "Porygon",
    "Creation numerique",
    C_PURPLE,
    0, 0, 38, 13
  },
  {
    "Amonita",
    "Fossile ammonite",
    C_BLUE,
    142, 32, 32, 11
  },
  {
    "Amonistar",
    "Ancienne spirale",
    C_BLUE,
    0, 0, 49, 16
  },
  {
    "Kabuto",
    "Fossile corace",
    C_MUTED,
    144, 32, 31, 11
  },
  {
    "Kabutops",
    "Predateur fossile",
    C_MUTED,
    0, 0, 48, 18
  },
  {
    "Ptera",
    "Reptile volant antique",
    C_MUTED,
    0, 0, 52, 19
  },
  {
    "Ronflex",
    "Dormeur monumental",
    C_MUTED,
    0, 0, 68, 12
  },
  {
    "Artikodin",
    "Oiseau legendaire glace",
    C_BLUE,
    0, 0, 55, 18
  },
  {
    "Electhor",
    "Oiseau legendaire foudre",
    C_YELLOW,
    0, 0, 55, 19
  },
  {
    "Sulfura",
    "Oiseau legendaire feu",
    C_RED,
    0, 0, 55, 20
  },
  {
    "Minidraco",
    "Dragon reptilien",
    C_BLUE,
    149, 30, 34, 12
  },
  {
    "Draco",
    "Dragon novice",
    C_BLUE,
    151, 55, 44, 17
  },
  {
    "Dracolosse",
    "Dragon legendaire",
    C_BLUE,
    0, 0, 58, 23
  },
  {
    "Mewtwo",
    "Creation genetique supreme",
    C_PURPLE,
    0, 0, 62, 24
  },
  {
    "Mew",
    "Ancêtre mysterieux",
    C_PURPLE,
    0, 0, 58, 19
  }
};

/* =========================================================
   VARIABLES GLOBALES
   ========================================================= */

static color_t strip[SCREEN_W * STRIP_H];

extern const uint8_t game_data[];

static SaveData save_data;

static uint32_t seed = 1;
static uint32_t next_tick = 0;

static int screen = 0;
static int selection = 0;

static int player_x = 32;
static int player_y = 32;

static int wild_hp = 0;
static int player_hp = 0;
static int wild_species = 0;
static int wild_level = 2;

static int battle_selection = 0;

static int message_id = 0;
static int message_next = 1;

static int evolution_happened = 0;
static uint32_t world_clock = 0;
static int world_weather = 0;
static int combat_status = 0;
static int enemy_status = 0;
static int bot_selection = 0;
static int battle_boost = 0;
static int battle_boost_turns = 0;
static int dex_page = 0;

static void show_message(int id, int next);

/* =========================================================
   ECRANS
   ========================================================= */

enum {
  SCREEN_TITLE,
  SCREEN_WORLD,
  SCREEN_BATTLE,
  SCREEN_PAUSE,
  SCREEN_CONFIRM,
  SCREEN_PARTY,
  SCREEN_DEX,
  SCREEN_BAG,
  SCREEN_QUESTS,
  SCREEN_SHOP,
  SCREEN_PROFILE,
  SCREEN_BADGES,
  SCREEN_BOTS,
  SCREEN_CAPABILITIES,
  SCREEN_RAIDS,
  SCREEN_EGGS,
  SCREEN_BUDDY,
  SCREEN_GIFTS,
  SCREEN_ROUTES,
  SCREEN_RESEARCH,
  SCREEN_PHOTOS,
  SCREEN_TEAM,
  SCREEN_TRADING,
  SCREEN_MESSAGE
};

/* =========================================================
   MESSAGES
   ========================================================= */

enum {
  MSG_NONE,
  MSG_VICTORY,
  MSG_CAPTURE,
  MSG_FAILED,
  MSG_EVOLUTION,
  MSG_FULL,
  MSG_NO_BALL,
  MSG_NO_POTION,
  MSG_HEAL,
  MSG_ESCAPE,
  MSG_FAINT
};

static const char *messages[] = {
  "",
  "Victoire ! Experience gagnee.",
  "Pokemon capture !",
  "La capture a echoue !",
  "Une evolution a eu lieu !",
  "Ton equipe est complete !",
  "Tu n'as plus de capsules !",
  "Tu n'as plus de potions !",
  "Ton equipe est soignee !",
  "Tu as fui le combat.",
  "Ton equipe est fatiguee."
};

enum {
  STATUS_OK,
  STATUS_BURN,
  STATUS_SLEEP,
  STATUS_POISON,
  STATUS_PARALYSIS
};

/* =========================================================
   OUTILS
   ========================================================= */

static uint32_t rnd(uint32_t max) {
  seed ^= seed << 13;
  seed ^= seed >> 17;
  seed ^= seed << 5;

  return max ? seed % max : 0;
}

static int clamp(int v, int min, int max) {
  if (v < min) return min;
  if (v > max) return max;
  return v;
}

static int species_type(int id) {
  if (id < 0 || id >= SPECIES_COUNT) return 0;
  if (id % 8 == 0 || id % 8 == 1) return 1; /* feu */
  if (id % 8 == 2 || id % 8 == 3) return 2; /* eau */
  if (id % 8 == 4 || id % 8 == 5) return 3; /* plante */
  if (id % 8 == 6 || id % 8 == 7) return 4; /* roche */
  return 0;
}

static int type_multiplier(int attack_type, int defender_type) {
  if (attack_type == 1 && defender_type == 3) return 160;
  if (attack_type == 1 && defender_type == 2) return 50;
  if (attack_type == 2 && defender_type == 1) return 160;
  if (attack_type == 2 && defender_type == 4) return 160;
  if (attack_type == 3 && defender_type == 2) return 160;
  if (attack_type == 3 && defender_type == 1) return 50;
  if (attack_type == 4 && defender_type == 1) return 50;
  return 100;
}

static uint32_t hash_bytes(const uint8_t *data, uint32_t len) {
  uint32_t h = 2166136261u;

  for (uint32_t i = 0; i < len; i++) {
    h ^= data[i];
    h *= 16777619u;
  }

  return h;
}

static void clear_bytes(void *ptr, uint32_t len) {
  uint8_t *p = (uint8_t *)ptr;

  for (uint32_t i = 0; i < len; i++) {
    p[i] = 0;
  }
}

static int creature_max_hp(Creature c) {
  if (c.species >= SPECIES_COUNT) return 1;

  return species[c.species].base_hp + c.level * 3;
}

static int creature_attack(Creature c) {
  if (c.species >= SPECIES_COUNT) return 1;

  return species[c.species].base_attack + c.level;
}

static int active_creature(void) {
  if (!save_data.party_count) return -1;

  return 0;
}

/* =========================================================
   SAUVEGARDE
   ========================================================= */

static void save_game(void) {
  save_data.magic = 0x4352474Fu;
  save_data.version = 5;

  save_data.world_x = (int16_t)player_x;
  save_data.world_y = (int16_t)player_y;

  save_data.checksum = 0;

  save_data.checksum = hash_bytes(
    (const uint8_t *)&save_data,
    sizeof(save_data) - sizeof(uint32_t)
  );

  np_file_write(
    SAVE_NAME,
    &save_data,
    sizeof(save_data)
  );
}

static bool load_game(void) {
  uint32_t len = 0;

  const uint8_t *data = np_file_read(
    SAVE_NAME,
    &len
  );

  if (!data || len != sizeof(SaveData)) {
    return false;
  }

  SaveData candidate;

  uint8_t *dst = (uint8_t *)&candidate;

  for (uint32_t i = 0; i < len; i++) {
    dst[i] = data[i];
  }

  uint32_t stored_checksum = candidate.checksum;

  candidate.checksum = 0;

  uint32_t calculated = hash_bytes(
    (const uint8_t *)&candidate,
    sizeof(candidate) - sizeof(uint32_t)
  );

  if (candidate.magic != 0x4352474Fu ||
      candidate.version != 5 ||
      candidate.party_count > PARTY_MAX ||
      stored_checksum != calculated) {
    return false;
  }

  for (int i = 0; i < candidate.party_count; i++) {
    if (candidate.party[i].species >= SPECIES_COUNT ||
        candidate.party[i].level == 0) {
      return false;
    }
  }

  save_data = candidate;
  save_data.checksum = stored_checksum;
  seed = save_data.seed ? save_data.seed : 1u;

  player_x = clamp(candidate.world_x, 0, WORLD_SIZE - 1);
  player_y = clamp(candidate.world_y, 0, WORLD_SIZE - 1);

  return true;
}

static void new_game(void) {
  clear_bytes(&save_data, sizeof(save_data));

  save_data.seed =
    (uint32_t)eadk_timing_millis() ^ (uint32_t)rnd(0x7FFFFFFFu);
  seed = save_data.seed ? save_data.seed : 1u;
  seed ^= (uint32_t)game_data[seed & 819199u];
  player_x = 16 + (int)(seed % 33);
  player_y = 16 + (int)((seed >> 8) % 33);

  save_data.party_count = 1;
  save_data.balls = 10;
  save_data.potions = 5;
  save_data.stardust = 300;
  save_data.coins = 100;
  save_data.captured = 0;
  save_data.distance = 0;
  save_data.quest_catches = 0;
  save_data.quest_steps = 0;
  save_data.quest_claimed = 0;
  save_data.berries = 5;
  save_data.team = 0;
  save_data.egg_type = 2;
  save_data.egg_distance = 0;
  save_data.egg_target = 2;
  save_data.buddy_species = 0;
  save_data.buddy_hearts = 0;
  save_data.gifts = 3;
  save_data.raid_tier = 1;
  save_data.raid_wins = 0;
  save_data.photos = 0;
  save_data.routes = 0;
  save_data.trades = 0;
  save_data.research_points = 0;
  save_data.player_xp = 0;
  save_data.player_level = 1;
  save_data.pokedollars = 500;
  save_data.ball_style = 0;
  save_data.skin_style = 0;
  save_data.badges = 0;
  save_data.bot_wins = 0;
  save_data.nickname[0] = 'J';
  save_data.nickname[1] = 'o';
  save_data.nickname[2] = 'u';
  save_data.nickname[3] = 'e';
  save_data.nickname[4] = 'u';
  save_data.nickname[5] = 'r';
  save_data.nickname[6] = 0;

  save_data.party[0].species = 0;
  save_data.party[0].level = 5;
  save_data.party[0].xp = 0;

  for (int i = 0; i < 5; i++) save_data.dex[i] = 0;
  save_data.dex[0] = 1;

  save_game();
}

/* =========================================================
   POKEDEX
   ========================================================= */

static void register_creature(int id) {
  if (id < 0 || id >= SPECIES_COUNT) return;

  save_data.dex[id >> 5] |= 1u << (id & 31);
}

/* =========================================================
   EXPERIENCE ET EVOLUTION
   ========================================================= */

static bool add_experience(Creature *c, int amount) {
  if (!c || c->species >= SPECIES_COUNT) {
    return false;
  }

  c->xp += (uint8_t)amount;

  if (c->xp >= 10) {
    c->xp -= 10;
    c->level++;

    int next = species[c->species].evolution;

    if (next &&
        c->level >= species[c->species].evolution_level) {
      c->species = (uint8_t)next;

      register_creature(next);

      return true;
    }
  }

  return false;
}

static void add_player_experience(uint32_t amount) {
  save_data.player_xp += amount;
  while (save_data.player_xp >= save_data.player_level * 100u) {
    save_data.player_xp -= save_data.player_level * 100u;
    save_data.player_level++;
    save_data.pokedollars += 250u;
  }
}

/* =========================================================
   CARTE ET TERRAIN
   ========================================================= */

static int terrain(int x, int y) {
  uint32_t h =
    save_data.seed ^
    (uint32_t)(x * 73856093u) ^
    (uint32_t)(y * 19349663u);

  h ^= h >> 13;
  h *= 1274126177u;
  h ^= h >> 16;

  int v = h % 40;

  /* Routes larges, quartiers urbains, eau, parc et montagnes. */
  int road = ((x + 3) % 13 == 0) || ((y + 5) % 17 == 0);
  int district = ((x / 8 + y / 8) & 1);

  if (!road && v < 3) return 3;
  if (!road && v < 7) return 2;
  if (road) return 4;
  if (district && v < 16) return 5;
  if (v < 12) return 1;

  return 0;
}

static bool blocked(int x, int y) {
  if (x < 0 || y < 0 ||
      x >= WORLD_SIZE || y >= WORLD_SIZE) {
    return true;
  }

  int t = terrain(x, y);

  return t == 2 || t == 3 || t == 5;
}

static int region_id(int x, int y) {
  int rx = x / 64;
  int ry = y / 64;
  return rx + ry * 4;
}

static const char *region_name(int r) {
  static const char *names[16] = {
    "Luminosa", "Flavora", "Aqualis", "Rochelle",
    "Sylvana", "Voltoria", "Glacialis", "Nebulosa",
    "Cendral", "Dunélia", "Myrtille", "Terralys",
    "Carapagos", "Solaris", "Nyxoria", "Aurelia"
  };
  if (r < 0 || r >= 16) return "Region inconnue";
  return names[r];
}

/* =========================================================
   RENCONTRES
   ========================================================= */

static void start_battle(void) {
  int zone = terrain(player_x, player_y);
  int id;

  if (zone == 1) id = rnd(2) ? 4 : 5;
  else if (zone == 2) id = rnd(2) ? 2 : 10;
  else if (zone == 3) id = rnd(2) ? 6 : 7;
  else if (zone == 4) id = 8 + (int)rnd(8);
  else id = (int)rnd(SPECIES_COUNT);

  wild_hp = species[id].base_hp + 3 * (2 + rnd(5));

  int active = active_creature();

  if (active < 0) {
    screen = SCREEN_WORLD;
    return;
  }

  Creature c = save_data.party[active];

  player_hp = creature_max_hp(c);

  battle_selection = 0;
  combat_status = STATUS_OK;
  enemy_status = STATUS_OK;
  battle_boost = 0;
  battle_boost_turns = 0;

  /* Stocke la creature sauvage dans une variable temporaire. */
  wild_species = id;
  wild_level = 2 + rnd(5);

  screen = SCREEN_BATTLE;
}

/* =========================================================
   DEPLACEMENT
   ========================================================= */

static void move_player(int dx, int dy) {
  int nx = player_x + dx;
  int ny = player_y + dy;

  if (blocked(nx, ny)) return;

  player_x = nx;
  player_y = ny;

  save_data.steps++;
  save_data.quest_steps =
    save_data.quest_steps < 100 ? (uint16_t)(save_data.quest_steps + 1) : 100;
  save_data.distance++;
  if (save_data.egg_type && save_data.egg_distance < save_data.egg_target) {
    save_data.egg_distance++;
  }
  if (save_data.egg_distance >= save_data.egg_target && save_data.egg_type) {
    save_data.egg_type = 0;
    save_data.captured++;
    save_data.pokedollars += 150;
    save_data.stardust += 500;
  }

  if (rnd(100) < 13) {
    start_battle();
  }
}

static void world_tick(uint64_t keys) {
  world_clock = (world_clock + 1) & 719;
  world_weather = (world_clock / 180) & 3;

  if (keys & KEY(eadk_key_left)) {
    move_player(-1, 0);
  } else if (keys & KEY(eadk_key_right)) {
    move_player(1, 0);
  } else if (keys & KEY(eadk_key_up)) {
    move_player(0, -1);
  } else if (keys & KEY(eadk_key_down)) {
    move_player(0, 1);
  }
}

static bool near_pokestop(void) {
  int sx = 24 + ((world_weather * 17 + player_y) % 28);
  int sy = 24 + ((world_weather * 13 + player_x) % 28);
  return player_x >= sx - 2 && player_x <= sx + 2 &&
         player_y >= sy - 2 && player_y <= sy + 2;
}

static void spin_pokestop(void) {
  if (!near_pokestop()) return;
  save_data.coins += 5;
  save_data.balls += 3;
  save_data.potions += 1;
  save_data.stardust += 150;
  save_data.gifts = save_data.gifts < 20 ? (uint8_t)(save_data.gifts + 1) : 20;
  save_game();
  show_message(MSG_HEAL, SCREEN_WORLD);
}

/* =========================================================
   COMBAT
   ========================================================= */

static void show_message(int id, int next) {
  message_id = id;
  message_next = next;
  screen = SCREEN_MESSAGE;
}

static void enemy_turn(void) {
  if (enemy_status == STATUS_SLEEP && (rnd(100) < 55)) {
    return;
  }
  if (enemy_status == STATUS_POISON) {
    wild_hp -= 3;
  }

  int damage = species[wild_species].base_attack / 2 + 1;
  if (enemy_status == STATUS_BURN) damage -= damage / 3;
  if (battle_boost_turns > 0) {
    damage += battle_boost;
    battle_boost_turns--;
  }

  player_hp -= damage;

  if (player_hp <= 0) {
    show_message(MSG_FAINT, SCREEN_WORLD);
    player_hp = 1;
  }
}

static void battle_fight(void) {
  int active = active_creature();

  if (active < 0) {
    screen = SCREEN_WORLD;
    return;
  }

  Creature *c = &save_data.party[active];

  int damage = creature_attack(*c);
  int multiplier = type_multiplier(
    species_type(save_data.party[active].species),
    species_type(wild_species)
  );
  damage = damage * multiplier / 100;
  if (combat_status == STATUS_PARALYSIS && rnd(100) < 25) {
    enemy_turn();
    return;
  }
  if (combat_status == STATUS_BURN) {
    damage -= damage / 4;
  }
  if (combat_status == STATUS_POISON) {
    player_hp -= 2;
  }

  wild_hp -= damage;

  if (wild_hp <= 0) {
    evolution_happened = add_experience(c, 5);
    save_data.stardust += 75 + wild_level * 10;
    save_data.coins += 10;
    save_data.pokedollars += 20u;
    add_player_experience(40u);
    save_data.research_points =
      save_data.research_points < 255 ?
      (uint8_t)(save_data.research_points + 1) : 255;

    save_game();

    show_message(
      evolution_happened ? MSG_EVOLUTION : MSG_VICTORY,
      SCREEN_WORLD
    );

    return;
  }

  if (enemy_status == STATUS_OK && rnd(100) < 12) {
    enemy_status = 1 + (int)rnd(4);
  }

  enemy_turn();

  if (screen == SCREEN_BATTLE) {
    save_game();
  }
}

static void battle_capture(void) {
  if (!save_data.balls) {
    show_message(MSG_NO_BALL, SCREEN_BATTLE);
    return;
  }

  save_data.balls--;

  int max_hp = species[wild_species].base_hp + wild_level * 3;

  int missing = max_hp - wild_hp;

  int chance = 30 + (missing * 60) / (max_hp ? max_hp : 1);

  if ((int)rnd(100) < chance) {
    if (save_data.party_count >= PARTY_MAX) {
      save_game();
      show_message(MSG_FULL, SCREEN_BATTLE);
      return;
    }

    Creature captured;

    captured.species = (uint8_t)wild_species;
    captured.level = (uint8_t)wild_level;
    captured.xp = 0;

    save_data.party[save_data.party_count++] = captured;

    register_creature(wild_species);
    save_data.captured++;
    save_data.quest_catches =
      save_data.quest_catches < 100 ?
      (uint16_t)(save_data.quest_catches + 1) : 100;
    if (save_data.captured >= 1) save_data.badges |= 1u << 0;
    if (save_data.captured >= 10) save_data.badges |= 1u << 4;
    if (save_data.captured >= 25) save_data.badges |= 1u << 6;
    if (save_data.captured >= 50) save_data.badges |= 1u << 7;
    save_data.stardust += 100 + wild_level * 20;
    save_data.coins += 3;
    save_data.pokedollars += 35u;
    add_player_experience(60u);

    save_game();

    show_message(MSG_CAPTURE, SCREEN_WORLD);

    return;
  }

  save_game();

  show_message(MSG_FAILED, SCREEN_BATTLE);
}

static void battle_potion(void) {
  if (!save_data.potions) {
    show_message(MSG_NO_POTION, SCREEN_BATTLE);
    return;
  }

  save_data.potions--;

  int active = active_creature();

  if (active >= 0) {
    int max_hp = creature_max_hp(save_data.party[active]);

    player_hp += 15;

    if (player_hp > max_hp) {
      player_hp = max_hp;
    }
  }

  save_game();

  enemy_turn();

  if (screen == SCREEN_BATTLE) {
    show_message(MSG_HEAL, SCREEN_BATTLE);
  }
}

static void battle_action(void) {
  switch (battle_selection) {
    case 0:
      battle_fight();
      break;

    case 1:
      battle_capture();
      break;

    case 2:
      battle_potion();
      break;

    case 3:
      show_message(MSG_ESCAPE, SCREEN_WORLD);
      break;
    case 4:
      if (save_data.pokedollars >= 50) {
        save_data.pokedollars -= 50;
        battle_boost = 8;
        battle_boost_turns = 3;
      }
      enemy_turn();
      break;
  }
}

static void battle_input(uint64_t hit) {
  if (hit & KEY(eadk_key_left)) {
    battle_selection =
      (battle_selection + 4) % 5;
  }

  if (hit & KEY(eadk_key_right)) {
    battle_selection =
      (battle_selection + 1) % 5;
  }

  if (hit & KEY(eadk_key_back)) {
    show_message(MSG_ESCAPE, SCREEN_WORLD);
    return;
  }

  if (hit & (KEY(eadk_key_ok) | KEY(eadk_key_exe))) {
    battle_action();
  }
}

/* =========================================================
   DESSIN DES CREATURES
   ========================================================= */

extern const uint16_t pokemon_atlas[];
extern const uint16_t world_atlas[];
extern const uint8_t game_data[];

static void draw_atlas_creature(int id, int cx, int cy, int scale) {
  if (!pokemon_atlas || id < 0 || id >= SPECIES_COUNT) return;

  const int atlas_w = 128;
  const int atlas_h = 128;
  const int dw = 64 * scale;
  const int dh = 64 * scale;
  const int x0 = cx - dw / 2;
  const int y0 = cy - dh / 2;
  const uint16_t *sprite = pokemon_atlas + id * atlas_w * atlas_h;

  for (int py = y0; py < y0 + dh; py++) {
    if (py < gfx_y0 || py >= gfx_y1) continue;
    int sy = (py - y0) * atlas_h / dh;
    for (int px = x0; px < x0 + dw; px++) {
      if (px < 0 || px >= SCREEN_W) continue;
      int sx = (px - x0) * atlas_w / dw;
      uint16_t pixel = sprite[sy * atlas_w + sx];
      if (pixel != 0x0001u) {
        gfx_buf[(py - gfx_y0) * SCREEN_W + px] = (color_t)pixel;
      }
    }
  }
}

static void draw_world_thumbnail(int region, int x, int y) {
  if (region < 0 || region >= 16) return;
  const int sw = 128;
  const int dw = 64;
  const uint16_t *map = world_atlas + region * sw * sw;

  for (int py = y; py < y + dw; py++) {
    if (py < gfx_y0 || py >= gfx_y1) continue;
    int sy = (py - y) * sw / dw;
    for (int px = x; px < x + dw; px++) {
      if (px < 0 || px >= SCREEN_W) continue;
      int sx = (px - x) * sw / dw;
      gfx_buf[(py - gfx_y0) * SCREEN_W + px] =
        (color_t)map[sy * sw + sx];
    }
  }
}

static void draw_creature(int id, int x, int y, int scale) {
  if (id < 0 || id >= SPECIES_COUNT) return;
  draw_atlas_creature(id, x, y, scale);
  return;

  color_t c = species[id].color;

  int r = 12 * scale;

  gfx_circle(x + 2 * scale, y + r + 5 * scale, r + 4 * scale, HEX(0x0A2A32), 160);
  gfx_circle(x, y, r + 2, C_DARK, 256);
  gfx_circle(x, y, r, c, 256);

  /* Reflet organique sur le corps. */
  gfx_circle(
    x - 5 * scale,
    y - 7 * scale,
    5 * scale,
    gfx_mix(c, C_WHITE, 10),
    180
  );

  gfx_circle(
    x - 5 * scale,
    y - 3 * scale,
    2 * scale,
    C_WHITE,
    256
  );

  gfx_circle(
    x + 5 * scale,
    y - 3 * scale,
    2 * scale,
    C_WHITE,
    256
  );

  gfx_circle(
    x - 5 * scale,
    y - 3 * scale,
    scale,
    C_DARK,
    256
  );

  gfx_circle(
    x + 5 * scale,
    y - 3 * scale,
    scale,
    C_DARK,
    256
  );

  gfx_fill(
    x - 4 * scale,
    y + 5 * scale,
    8 * scale,
    scale,
    C_DARK
  );

  if (id == 0 || id == 1) {
    gfx_circle(
      x,
      y - 16 * scale,
      5 * scale,
      C_YELLOW,
      256
    );
  }

  if (id == 2 || id == 3) {
    gfx_circle(
      x,
      y - 16 * scale,
      4 * scale,
      C_BLUE,
      256
    );
  }

  if (id == 4 || id == 5) {
    gfx_circle(
      x - 12 * scale,
      y - 12 * scale,
      5 * scale,
      C_GREEN,
      256
    );
  }

  if (id == 6 || id == 7) {
    gfx_fill(
      x - 4 * scale,
      y - 18 * scale,
      8 * scale,
      5 * scale,
      C_MUTED
    );
  }

  if (id >= 8 && id <= 9) {
    gfx_circle(x - 10 * scale, y - 13 * scale, 3 * scale, C_YELLOW, 256);
    gfx_circle(x + 10 * scale, y - 13 * scale, 3 * scale, C_YELLOW, 256);
    gfx_fill(x - 2 * scale, y - 19 * scale, 4 * scale, 10 * scale, C_YELLOW);
  }

  if (id >= 10 && id <= 11) {
    gfx_circle(x - 10 * scale, y - 10 * scale, 5 * scale, C_BLUE, 256);
    gfx_circle(x + 10 * scale, y - 10 * scale, 5 * scale, C_BLUE, 256);
  }

  if (id >= 12 && id <= 13) {
    gfx_rrect(x - 9 * scale, y - 20 * scale, 18 * scale, 7 * scale, 3 * scale, C_ORANGE, 256);
    gfx_circle(x - 9 * scale, y - 17 * scale, 4 * scale, C_RED, 256);
  }

  if (id >= 14 && id <= 15) {
    gfx_circle(x - 11 * scale, y - 14 * scale, 6 * scale, C_GREEN, 256);
    gfx_circle(x + 11 * scale, y - 14 * scale, 6 * scale, C_GREEN, 256);
  }
}

/* =========================================================
   INTERFACE
   ========================================================= */

static void panel(int x, int y, int w, int h) {
  gfx_rrect(x, y, w, h, 9, C_PANEL, 256);

  gfx_rrect_outline(
    x, y, w, h, 9, 1, C_PANEL2, 256
  );
}

static void choice(int y, const char *text, bool selected) {
  if (selected) {
    gfx_rrect(
      76, y - 17, 168, 25,
      8, C_YELLOW, 256
    );
  }

  gfx_text_center(
    &np_font_body,
    160,
    y,
    text,
    selected ? C_DARK : C_WHITE,
    256
  );
}

static void header(const char *text) {
  gfx_fill(0, 0, SCREEN_W, HUD_H, C_DARK);

  gfx_text(
    &np_font_body,
    8,
    16,
    text,
    C_WHITE,
    256
  );
}

/* =========================================================
   ECRAN TITRE
   ========================================================= */

static void draw_title(void) {
  gfx_vgrad(
    0, 0, SCREEN_W, SCREEN_H,
    0x102B40, 0x101B2B
  );

  /* Logo, silhouette d'arene et atmosphere nocturne. */
  for (int i = 0; i < 18; i++) {
    int x = (i * 47 + 13) % SCREEN_W;
    int y = 52 + ((i * 29) % 92);
    gfx_circle(x, y, 1 + (i & 1), C_WHITE, 80);
  }

  draw_creature(0, 160, 87, 2);
  draw_creature(2, 105, 119, 1);
  draw_creature(4, 215, 119, 1);

  gfx_text_center(
    &np_font_title,
    160,
    37,
    "POKEMON GO",
    C_YELLOW,
    256
  );

  gfx_text_center(
    &np_font_small,
    160,
    151,
    "Explore. Discover. Capture.",
    C_WHITE,
    256
  );

  panel(65, 169, 190, 52);

  choice(183, "Continuer", selection == 0);
  choice(207, "Nouvelle partie", selection == 1);

  gfx_text_center(
    &np_font_small,
    160,
    235,
    "OK: nouvelle partie  Back: reprendre",
    C_MUTED,
    256
  );
}

/* =========================================================
   MONDE
   ========================================================= */

static void draw_world(void) {
  gfx_vgrad(
    0, 0, SCREEN_W, SCREEN_H,
    0x315B56, 0x183B3B
  );

  const int tile_w = 26;
  const int tile_h = 25;

  for (int ty = 0; ty < 8; ty++) {
    for (int tx = 0; tx < 12; tx++) {
      int wx = player_x + tx - 6;
      int wy = player_y + ty - 4;

      int px = tx * tile_w + 4;
      int py = HUD_H + ty * tile_h;

      int t = terrain(wx, wy);

      color_t ground;

      if (t == 0) {
        ground = HEX(0x39785B);
      } else if (t == 1) {
        ground = HEX(0x477D52);
      } else if (t == 2) {
        ground = HEX(0x397CA2);
      } else if (t == 3) {
        ground = HEX(0x596B72);
      } else if (t == 4) {
        ground = HEX(0xA2AAA7);
      } else {
        ground = HEX(0x8A8F82);
      }

      gfx_fill(px, py, tile_w, tile_h, ground);

      if (t == 4) {
        if (((tx + ty) & 1) == 0) {
          gfx_fill(px + 2, py + 11, 22, 2, HEX(0xCBD1CE));
        }
        if (tx % 3 == 0) {
          gfx_fill(px + 12, py, 2, 25, HEX(0xC3C9C6));
        }
      }

      if (t == 5) {
        gfx_rrect(px + 3, py + 3, 20, 19, 2, HEX(0x7A8580), 256);
        gfx_rrect(px + 7, py + 7, 12, 10, 1, HEX(0xBFD5D5), 256);
        gfx_fill(px + 11, py + 17, 4, 5, HEX(0x4A514F));
      }

      if (t == 1) {
        gfx_circle(
          px + 7,
          py + 9,
          3,
          C_GREEN,
          256
        );
        gfx_fill(px + 6, py + 11, 2, 7, HEX(0x234B3A));
      }

      if (t == 2) {
        gfx_fill(
          px + ((ty * 7) % 9),
          py + 17,
          14,
          2,
          HEX(0x8ED0DF)
        );
      }

      if (t == 3) {
        gfx_circle(
          px + 13,
          py + 12,
          9,
          HEX(0x777F7C),
          256
        );
        gfx_fill(px + 8, py + 8, 10, 3, HEX(0xAEB6B3));
      }
    }
  }

  /* Pokeball markers and nearby wild Pokemon in the visible district. */
  for (int i = 0; i < 3; i++) {
    int mx = 70 + i * 96 + (int)(player_x % 3) * 5;
    int my = 78 + ((i * 43 + player_y) % 74);
    int species_id = (player_x + player_y * 3 + i * 5) % SPECIES_COUNT;

    gfx_circle(mx, my + 15, 13, HEX(0xAFC7C0), 100);
    draw_creature(species_id, mx, my, 1);
  }

  if (((player_x + player_y) & 3) == 0) {
    gfx_rrect(258, 63, 24, 24, 12, C_BLUE, 256);
    gfx_rrect(263, 68, 14, 14, 7, C_WHITE, 256);
    gfx_circle(270, 75, 3, C_BLUE, 256);
  }

  /* PokéStop procedural: couronne bleue, anneau lumineux et balise. */
  if (near_pokestop()) {
    gfx_circle(160, 87, 32, HEX(0x55C4EA), 170);
    gfx_circle(160, 87, 22, C_WHITE, 256);
    gfx_rrect_outline(148, 75, 24, 24, 12, 3, C_BLUE, 256);
    gfx_circle(160, 87, 5, C_BLUE, 256);
  }

  /* Météo et cycle jour/nuit. */
  if (world_weather == 2) {
    gfx_fill_alpha(0, HUD_H, SCREEN_W, SCREEN_H - HUD_H, HEX(0x6B91A8), 48);
  } else if (world_weather == 3) {
    gfx_fill_alpha(0, HUD_H, SCREEN_W, SCREEN_H - HUD_H, HEX(0x1B3048), 70);
  }

  draw_world_thumbnail(region_id(player_x, player_y), 248, 36);

  /* Personnage au centre. */
  gfx_circle(160, 126, 11, HEX(0x183B54), 256);
  gfx_circle(160, 123, 7, C_YELLOW, 256);
  gfx_circle(160, 120, 3, C_WHITE, 256);
  gfx_fill(156, 128, 8, 11, C_BLUE);
  gfx_fill(158, 138, 3, 5, C_DARK);
  gfx_fill(164, 138, 3, 5, C_DARK);

  char world_header[40];
  int r = region_id(player_x, player_y);
  const char *rn = region_name(r);
  int n = 0;
  while (rn[n] && n < 38) {
    world_header[n] = rn[n];
    n++;
  }
  world_header[n] = 0;
  header(world_header);

  gfx_text(
    &np_font_small,
    8,
    237,
    "Deplacer",
    C_WHITE,
    256
  );

  gfx_text_right(
    &np_font_small,
    312,
    237,
    "Menu",
    C_WHITE,
    256
  );
}

/* =========================================================
   COMBAT
   ========================================================= */

static void draw_battle(void) {
  gfx_vgrad(
    0, 0, SCREEN_W, SCREEN_H,
    0x294F65, 0x152A3E
  );

  /* Ciel, horizon et terrain de combat stylise 3D. */
  gfx_vgrad(0, 23, SCREEN_W, 175, 0x77BDB0, 0x2E6D67);
  gfx_rrect(0, 160, 320, 17, 8, HEX(0x315844), 256);
  for (int i = 0; i < 12; i++) {
    int x = i * 31 - ((player_x * 3) % 31);
    gfx_fill(x, 151 - (i & 3) * 3, 24, 3, HEX(0x72A28D));
  }

  gfx_circle(240, 94, 49, HEX(0x355F58), 220);
  gfx_circle(83, 155, 43, HEX(0x355F58), 220);
  gfx_circle(
    240, 94, 43,
    HEX(0x47765D),
    256
  );

  gfx_circle(
    83, 155, 37,
    HEX(0x47765D),
    256
  );

  draw_creature(
    wild_species,
    240,
    76,
    2
  );

  int active = active_creature();

  if (active >= 0) {
    draw_creature(
      save_data.party[active].species,
      80,
      142,
      2
    );
  }

  panel(8, 31, 145, 49);

  gfx_text(
    &np_font_body,
    16,
    48,
    species[wild_species].name,
    C_WHITE,
    256
  );

  gfx_text(
    &np_font_small,
    16,
    65,
    enemy_status == STATUS_BURN ? "Brule" :
    enemy_status == STATUS_SLEEP ? "Endormi" :
    enemy_status == STATUS_POISON ? "Empoisonne" :
    enemy_status == STATUS_PARALYSIS ? "Paralyse" : "Sauvage",
    C_MUTED,
    256
  );

  int max_wild_hp =
    species[wild_species].base_hp + wild_level * 3;

  gfx_fill(16, 72, 115, 4, C_DARK);

  gfx_fill(
    16,
    72,
    clamp(115 * wild_hp / (max_wild_hp ? max_wild_hp : 1), 0, 115),
    4,
    C_GREEN
  );

  panel(166, 113, 145, 43);

  if (active >= 0) {
    gfx_text(
      &np_font_body,
      174,
      131,
      species[save_data.party[active].species].name,
      C_WHITE,
      256
    );
  }

  gfx_text(
    &np_font_small,
    174,
    148,
    "Ton equipe",
    C_MUTED,
    256
  );

  panel(8, 176, 304, 56);

  const char *options[5] = {
    "Attaque",
    "Capsule",
    "Potion",
    "Fuite",
    "Capacite"
  };

  for (int i = 0; i < 5; i++) {
    int x = 31 + i * 64;

    if (battle_selection == i) {
      gfx_rrect(
        x - 34,
        188,
        68,
        27,
        7,
        C_YELLOW,
        256
      );
    }

    gfx_text_center(
      &np_font_small,
      x,
      206,
      options[i],
      battle_selection == i ? C_DARK : C_WHITE,
      256
    );
  }

  gfx_text_center(
    &np_font_small,
    160,
    237,
    "Left/Right: choisir   OK: valider",
    C_WHITE,
    256
  );
}

/* =========================================================
   EQUIPE
   ========================================================= */

static void draw_party(void) {
  gfx_vgrad(
    0, 0, SCREEN_W, SCREEN_H,
    0x162B42, 0x101B2B
  );

  header("MON EQUIPE");

  for (int i = 0; i < save_data.party_count; i++) {
    int y = 34 + i * 31;

    panel(8, y, 304, 28);

    draw_creature(
      save_data.party[i].species,
      27,
      y + 14,
      1
    );

    gfx_text(
      &np_font_body,
      49,
      y + 18,
      species[save_data.party[i].species].name,
      C_WHITE,
      256
    );

    char level[12] = "Niv. 00";

    level[5] = '0' + save_data.party[i].level / 10;
    level[6] = '0' + save_data.party[i].level % 10;
    level[7] = 0;

    gfx_text_right(
      &np_font_small,
      299,
      y + 18,
      level,
      C_YELLOW,
      256
    );
  }

  if (!save_data.party_count) {
    gfx_text_center(
      &np_font_body,
      160,
      120,
      "Equipe vide",
      C_WHITE,
      256
    );
  }

  gfx_text_center(
    &np_font_small,
    160,
    235,
    dex_page < 10 ? "Left/Right: pages   Back: retour" : "Derniere page   Back: retour",
    C_MUTED,
    256
  );
}

/* =========================================================
   POKEDEX
   ========================================================= */

static void draw_dex(void) {
  gfx_vgrad(
    0, 0, SCREEN_W, SCREEN_H,
    0x162B42, 0x101B2B
  );

  header("POKEDEX");

  int start = dex_page * 15;
  if (start >= SPECIES_COUNT) start = 0;
  dex_page = start / 15;

  for (int slot = 0; slot < 15; slot++) {
    int i = start + slot;
    if (i >= SPECIES_COUNT) break;
    int col = slot % 5;
    int row = slot / 5;

    int x = 32 + col * 64;
    int y = 76 + row * 52;

    panel(
      x - 28,
      y - 26,
      56,
      50
    );

    if (save_data.dex[i >> 5] & (1u << (i & 31))) {
      draw_creature(i, x, y - 5, 1);

      gfx_text_center(
        &np_font_small,
        x,
        y + 29,
        species[i].name,
        C_WHITE,
        256
      );
    } else {
      gfx_circle(
        x,
        y - 5,
        13,
        C_MUTED,
        256
      );

      gfx_text_center(
        &np_font_small,
        x,
        y + 29,
        "???",
        C_MUTED,
        256
      );
    }
  }

  gfx_text_center(
    &np_font_small,
    160,
    235,
    "Back: retour",
    C_MUTED,
    256
  );
}

/* =========================================================
   SAC
   ========================================================= */

static void draw_bag(void) {
  gfx_vgrad(
    0, 0, SCREEN_W, SCREEN_H,
    0x162B42, 0x101B2B
  );

  header("SAC");

  panel(28, 52, 264, 48);
  panel(28, 110, 264, 48);
  panel(28, 168, 264, 48);

  gfx_text(
    &np_font_body,
    48,
    78,
    "Capsules",
    C_WHITE,
    256
  );

  gfx_text_right(
    &np_font_title,
    270,
    82,
    " ",
    C_YELLOW,
    256
  );

  gfx_text_right(
    &np_font_body,
    272,
    78,
    save_data.balls == 0 ? "0" :
      save_data.balls < 10 ? "0" : "10+",
    C_YELLOW,
    256
  );

  gfx_text(
    &np_font_body,
    48,
    136,
    "Potions",
    C_WHITE,
    256
  );

  gfx_text_right(
    &np_font_body,
    272,
    136,
    save_data.potions == 0 ? "0" :
      save_data.potions < 10 ? "0" : "10+",
    C_GREEN,
    256
  );

  gfx_text(&np_font_body, 48, 194, "Poudre / Pieces",
           C_WHITE, 256);
  char resources[24];
  resources[0] = '0';
  if (save_data.stardust >= 1000) {
    resources[0] = '1';
    resources[1] = 'K';
    resources[2] = '+';
    resources[3] = 0;
  } else {
    resources[1] = '/';
    resources[2] = ' ';
    resources[3] = '0' + save_data.coins / 100;
    resources[4] = '0' + (save_data.coins / 10) % 10;
    resources[5] = '0' + save_data.coins % 10;
    resources[6] = 0;
  }
  gfx_text_right(&np_font_body, 272, 194, resources, C_YELLOW, 256);

  gfx_text(&np_font_small, 48, 222, "Baies nourriture:",
           C_MUTED, 256);
  gfx_text_right(&np_font_small, 272, 222,
                 save_data.berries ? "Disponible" : "Vide",
                 C_GREEN, 256);

  gfx_text_center(
    &np_font_small,
    160,
    235,
    "Objets utilises pendant les combats",
    C_MUTED,
    256
  );
}

/* =========================================================
   PROFIL
   ========================================================= */

static void draw_profile(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, 0x162B42, 0x101B2B);
  header("PROFIL");

  panel(18, 38, 284, 84);
  gfx_circle(50, 80, 18, save_data.skin_style ? C_PURPLE : C_BLUE, 256);
  gfx_circle(50, 76, 7, C_WHITE, 256);
  gfx_fill(43, 86, 14, 18, C_GREEN);
  gfx_text(&np_font_title, 82, 66, save_data.nickname, C_WHITE, 256);
  gfx_text(&np_font_small, 82, 94,
           save_data.player_level < 10 ? "Niveau 01+" : "Niveau",
           C_YELLOW, 256);

  panel(18, 132, 284, 48);
  gfx_text(&np_font_body, 30, 158,
           save_data.ball_style == 0 ? "Boule classique" :
           save_data.ball_style == 1 ? "Boule etoilee" : "Boule legendaire",
           C_WHITE, 256);
  gfx_rrect(232, 143, 48, 24, 12,
            save_data.ball_style == 2 ? C_YELLOW : C_RED, 256);
  gfx_fill(232, 153, 48, 4, C_WHITE);

  panel(18, 190, 284, 36);
  char dollars[16];
  dollars[0] = ' ';
  dollars[1] = '0' + save_data.pokedollars / 1000;
  dollars[2] = '0' + (save_data.pokedollars / 100) % 10;
  dollars[3] = '0' + (save_data.pokedollars / 10) % 10;
  dollars[4] = '0' + save_data.pokedollars % 10;
  dollars[5] = ' ';
  dollars[6] = 'P';
  dollars[7] = '$';
  dollars[8] = 0;
  gfx_text_right(&np_font_body, 290, 213, dollars, C_YELLOW, 256);
}

/* =========================================================
   BADGES
   ========================================================= */

static void draw_badges(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, 0x162B42, 0x101B2B);
  header("BADGES");

  const char *names[8] = {
    "Badge Feu", "Badge Eau", "Badge Plante", "Badge Roche",
    "Badge Foudre", "Badge Glace", "Badge Dragon", "Badge Aero"
  };

  for (int i = 0; i < 8; i++) {
    int col = i % 4;
    int row = i / 4;
    int x = 40 + col * 80;
    int y = 82 + row * 78;
    bool owned = save_data.badges & (1u << i);
    gfx_circle(x, y, 24, owned ? C_YELLOW : C_MUTED,
               owned ? 256 : 120);
    gfx_circle(x, y, 15, owned ? C_BLUE : C_DARK, 256);
    gfx_text_center(&np_font_small, x, y + 41,
                    owned ? names[i] : "???", owned ? C_WHITE : C_MUTED, 256);
  }

  gfx_text_center(&np_font_small, 160, 235,
                  "Badges selon les captures ou les arènes",
                  C_MUTED, 256);
}

/* =========================================================
   DUELLISTES BOT
   ========================================================= */

static void draw_bots(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, 0x162B42, 0x101B2B);
  header("DUELLISTES");

  const char *bots[4] = {"Nova", "Rex", "Akira", "Maelle"};
  for (int i = 0; i < 4; i++) {
    int y = 43 + i * 43;
    panel(18, y, 284, 36);
    gfx_circle(40, y + 18, 12, i == bot_selection ? C_YELLOW : C_BLUE, 256);
    gfx_text(&np_font_body, 62, y + 22, bots[i], C_WHITE, 256);
    gfx_text_right(&np_font_small, 290, y + 22,
                   i < save_data.bot_wins ? "Vaincu" : "Pret",
                   i < save_data.bot_wins ? C_GREEN : C_MUTED, 256);
  }

  gfx_text_center(&np_font_small, 160, 235,
                  "OK: defier   Back: retour", C_MUTED, 256);
}

/* =========================================================
   CAPACITES DE POKEMON
   ========================================================= */

static void draw_capabilities(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, 0x162B42, 0x101B2B);
  header("CAPACITES");

  const char *forms[10] = {
    "X", "Y", "Z", "V", "VMAX", "EX", "Mega EX",
    "Teracristal", "Gigamax", "Dynamax"
  };
  int active = active_creature();
  int owned = 0;
  if (active >= 0) {
    int id = save_data.party[active].species;
    owned = save_data.dex[id >> 5] & (1u << (id & 31));
  }
  for (int i = 0; i < 10; i++) {
    int col = i % 2;
    int row = i / 2;
    int x = 18 + col * 146;
    int y = 38 + row * 38;
    panel(x, y, 140, 32);
    gfx_text(&np_font_small, x + 12, y + 21, forms[i],
             owned && i <= save_data.player_level / 5 ? C_YELLOW :
             owned ? C_WHITE : C_MUTED, 256);
  }

  gfx_text_center(&np_font_small, 160, 235,
                  "Achetables avec des Pokedollars",
                  C_MUTED, 256);
}

/* =========================================================
   RAIDS
   ========================================================= */

static void draw_raids(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, 0x162B42, 0x101B2B);
  header("RAIDS");

  panel(18, 42, 284, 70);
  gfx_text(&np_font_title, 30, 70, "Raid 5*", C_YELLOW, 256);
  gfx_text(&np_font_small, 30, 96,
           save_data.raid_wins ? "Arène vaincue" : "Puissant boss disponible",
           C_WHITE, 256);

  panel(18, 122, 284, 60);
  gfx_text(&np_font_body, 30, 145, "Equipe recommandée",
           C_WHITE, 256);
  gfx_text_right(&np_font_small, 290, 165,
                 save_data.team ? "Equipe 2" : "Equipe 1",
                 C_BLUE, 256);

  gfx_text_center(&np_font_small, 160, 235,
                  "OK: commencer   Back: retour", C_MUTED, 256);
}

/* =========================================================
   OEUFS
   ========================================================= */

static void draw_eggs(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, 0x162B42, 0x101B2B);
  header("INCUBATEUR");

  panel(18, 45, 284, 88);
  gfx_circle(58, 89, 30, C_WHITE, 256);
  gfx_circle(58, 89, 21, C_YELLOW, 256);
  gfx_circle(58, 89, 7, C_WHITE, 256);

  if (save_data.egg_type) {
    gfx_text(&np_font_body, 105, 72, "Oeuf rare", C_WHITE, 256);
    gfx_fill(105, 93, 180, 8, C_DARK);
    gfx_fill(105, 93,
             clamp(save_data.egg_distance * 180 / save_data.egg_target, 0, 180),
             8, C_GREEN);
    gfx_text(&np_font_small, 105, 116, "Distance",
             C_MUTED, 256);
  } else {
    gfx_text(&np_font_body, 105, 90, "Oeuf eclos", C_YELLOW, 256);
  }

  gfx_text_center(&np_font_small, 160, 235,
                  "Marche avec l'oeuf actif", C_MUTED, 256);
}

/* =========================================================
   COPAIN
   ========================================================= */

static void draw_buddy(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, 0x162B42, 0x101B2B);
  header("COPAIN");

  panel(18, 42, 284, 92);
  draw_creature(save_data.buddy_species, 58, 88, 2);
  gfx_text(&np_font_body, 108, 72,
           species[save_data.buddy_species].name, C_WHITE, 256);
  gfx_text(&np_font_small, 108, 94, "Coeurs: ",
           C_MUTED, 256);
  for (int i = 0; i < 10; i++) {
    gfx_circle(108 + i * 16, 108, i < save_data.buddy_hearts ? 5 : 3,
               i < save_data.buddy_hearts ? C_RED : C_MUTED, 256);
  }

  gfx_text_center(&np_font_small, 160, 235,
                  "OK: donner baie   Back: retour", C_MUTED, 256);
}

/* =========================================================
   CADEAUX
   ========================================================= */

static void draw_gifts(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, 0x162B42, 0x101B2B);
  header("CADEAUX");

  panel(18, 42, 284, 48);
  gfx_text(&np_font_body, 30, 72, "Cadeaux amis",
           C_WHITE, 256);
  gfx_text_right(&np_font_title, 290, 76, save_data.gifts ? "3" : "0",
                 C_YELLOW, 256);

  panel(18, 100, 284, 48);
  gfx_text(&np_font_body, 30, 130, "Cartes postales",
           C_WHITE, 256);
  gfx_text_right(&np_font_body, 290, 130, "12", C_BLUE, 256);

  gfx_text_center(&np_font_small, 160, 235,
                  "OK: ouvrir cadeau", C_MUTED, 256);
}

/* =========================================================
   ROUTES
   ========================================================= */

static void draw_routes(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, 0x162B42, 0x101B2B);
  header("ROUTES");

  for (int i = 0; i < 4; i++) {
    int y = 42 + i * 46;
    panel(18, y, 284, 38);
    gfx_circle(42, y + 19, 10, i == selection ? C_YELLOW : C_GREEN, 256);
    gfx_text(&np_font_body, 64, y + 23,
             i == 0 ? "Boucle de la ville" :
             i == 1 ? "Sentier du lac" :
             i == 2 ? "Route des dunes" : "Chemin legendaire",
             C_WHITE, 256);
  }

  gfx_text_center(&np_font_small, 160, 235,
                  "OK: enregistrer la route", C_MUTED, 256);
}

/* =========================================================
   RECHERCHES
   ========================================================= */

static void draw_research(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, 0x162B42, 0x101B2B);
  header("RECHERCHE");

  panel(18, 42, 284, 64);
  gfx_text(&np_font_body, 30, 66, "Recherche terrain",
           C_WHITE, 256);
  gfx_fill(30, 78, 260, 7, C_DARK);
  gfx_fill(30, 78,
           clamp(save_data.research_points * 260 / 7, 0, 260),
           7, C_PURPLE);

  panel(18, 116, 284, 64);
  gfx_text(&np_font_body, 30, 140, "Rencontre speciale",
           C_WHITE, 256);
  gfx_text_right(&np_font_small, 290, 140,
                 save_data.research_points >= 7 ? "Disponible" : "En cours",
                 C_YELLOW, 256);

  gfx_text_center(&np_font_small, 160, 235,
                  "Recompense apres 7 recherches", C_MUTED, 256);
}

/* =========================================================
   PHOTOS
   ========================================================= */

static void draw_photos(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, 0x162B42, 0x101B2B);
  header("APPAREIL");

  gfx_rrect(22, 42, 276, 150, 14, HEX(0x244F63), 256);
  if (save_data.party_count) {
    draw_creature(save_data.party[0].species, 160, 105, 3);
  }
  gfx_circle(284, 62, 9, C_RED, 256);
  gfx_text(&np_font_small, 34, 211,
           save_data.photos ? "Derniere photo sauvegardee" : "Mode photo",
           C_WHITE, 256);
  gfx_text_center(&np_font_small, 160, 235,
                  "OK: prendre une photo", C_MUTED, 256);
}

/* =========================================================
   EQUIPES
   ========================================================= */

static void draw_team(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, 0x162B42, 0x101B2B);
  header("EQUIPE");

  const char *teams[3] = {"Equipe Saphir", "Equipe Rubis", "Equipe Emeraude"};
  const color_t colors[3] = {C_BLUE, C_RED, C_GREEN};
  for (int i = 0; i < 3; i++) {
    int y = 58 + i * 55;
    panel(18, y, 284, 44);
    gfx_circle(46, y + 22, 14,
               i == save_data.team ? C_YELLOW : colors[i], 256);
    gfx_text(&np_font_body, 72, y + 27, teams[i], C_WHITE, 256);
  }

  gfx_text_center(&np_font_small, 160, 235,
                  "OK: choisir une equipe", C_MUTED, 256);
}

/* =========================================================
   ECHANGE
   ========================================================= */

static void draw_trading(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, 0x162B42, 0x101B2B);
  header("ECHANGE");

  panel(18, 42, 284, 70);
  gfx_text(&np_font_title, 30, 70, "Partenaire bot", C_YELLOW, 256);
  gfx_text(&np_font_small, 30, 96,
           "Pokemon hors liste disponible", C_WHITE, 256);

  panel(18, 122, 284, 58);
  gfx_text(&np_font_body, 30, 150, "Amity / distance",
           C_WHITE, 256);

  gfx_text_center(&np_font_small, 160, 235,
                  "OK: proposer un echange", C_MUTED, 256);
}

/* =========================================================
   QUETES
   ========================================================= */

static void draw_quests(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, 0x162B42, 0x101B2B);
  header("MISSIONS");

  panel(18, 38, 284, 67);
  gfx_text(&np_font_body, 30, 58, "Capturer 5 Pokemon", C_WHITE, 256);
  gfx_fill(30, 69, 260, 7, C_DARK);
  gfx_fill(30, 69, clamp(save_data.quest_catches * 260 / 5, 0, 260), 7,
           C_GREEN);
  gfx_text_right(&np_font_small, 290, 91,
                 save_data.quest_catches >= 5 ? "Terminee" : "En cours",
                 C_YELLOW, 256);

  panel(18, 115, 284, 67);
  gfx_text(&np_font_body, 30, 135, "Marcher 100 pas", C_WHITE, 256);
  gfx_fill(30, 146, 260, 7, C_DARK);
  gfx_fill(30, 146, clamp(save_data.quest_steps * 260 / 100, 0, 260), 7,
           C_BLUE);
  gfx_text_right(&np_font_small, 290, 168,
                 save_data.quest_steps >= 100 ? "Terminee" : "En cours",
                 C_YELLOW, 256);

  if (!save_data.quest_claimed &&
      save_data.quest_catches >= 5 &&
      save_data.quest_steps >= 100) {
    gfx_rrect(42, 195, 236, 30, 8, C_YELLOW, 256);
    gfx_text_center(&np_font_body, 160, 215, "Recevoir 500 poudre",
                     C_DARK, 256);
  } else {
    gfx_text_center(&np_font_small, 160, 210,
                    save_data.quest_claimed ? "Recompense recue" : "Objectifs a completer",
                    C_MUTED, 256);
  }

  gfx_text_center(&np_font_small, 160, 235, "Back: retour", C_MUTED, 256);
}

/* =========================================================
   BOUTIQUE
   ========================================================= */

static void draw_shop(void) {
  gfx_vgrad(0, 0, SCREEN_W, SCREEN_H, 0x162B42, 0x101B2B);
  header("BOUTIQUE");

  panel(18, 40, 284, 52);
  choice(66, "10 capsules - 100 pieces", selection == 0);

  panel(18, 102, 284, 52);
  choice(128, "5 potions - 80 pieces", selection == 1);

  panel(18, 164, 284, 52);
  choice(190, "Pack explorateur - 200 pieces", selection == 2);

  panel(18, 196, 284, 32);
  choice(212, "Speciales X-Y-Z-V-EX", selection == 3);

  panel(18, 228, 284, 24);
  choice(240, "Baies - 40 pieces", selection == 4);

  gfx_text_center(&np_font_small, 160, 235,
                  save_data.coins >= 200 ? "Pieces disponibles: 200+" :
                  "Acheter avec les pieces", C_MUTED, 256);
}

/* =========================================================
   PAUSE
   ========================================================= */

static void draw_pause(void) {
  gfx_vgrad(
    0, 0, SCREEN_W, SCREEN_H,
    0x102B40, 0x101B2B
  );

  panel(8, 24, 304, 196);

  gfx_text_center(
    &np_font_title,
    160,
    62,
    "PAUSE",
    C_YELLOW,
    256
  );

  const char *pause_items[18] = {
    "Reprendre", "Equipe", "Pokedex", "Sac", "Missions",
    "Boutique", "Profil", "Badges", "Capacites",
    "Raids", "Oeufs", "Copain", "Cadeaux", "Routes",
    "Recherche", "Photos", "Equipes", "Echange"
  };

  for (int i = 0; i < 18; i++) {
    int col = i % 3;
    int row = i / 3;
    int x = 54 + col * 98;
    int y = 82 + row * 38;
    if (i == selection) {
      gfx_rrect(x - 42, y - 16, 84, 27, 7, C_YELLOW, 256);
    }
    gfx_text_center(&np_font_small, x, y,
                    pause_items[i],
                    i == selection ? C_DARK : C_WHITE, 256);
  }
}

/* =========================================================
   CONFIRMATION
   ========================================================= */

static void draw_confirm(void) {
  gfx_vgrad(
    0, 0, SCREEN_W, SCREEN_H,
    0x102B40, 0x101B2B
  );

  panel(48, 65, 224, 116);

  gfx_text_center(
    &np_font_title,
    160,
    96,
    "QUITTER ?",
    C_YELLOW,
    256
  );

  gfx_text_center(
    &np_font_body,
    160,
    124,
    "Sauvegarder et quitter ?",
    C_WHITE,
    256
  );

  choice(151, "Non", selection == 0);
  choice(179, "Oui", selection == 1);
}

/* =========================================================
   MESSAGE
   ========================================================= */

static void draw_message(void) {
  gfx_vgrad(
    0, 0, SCREEN_W, SCREEN_H,
    0x102B40, 0x101B2B
  );

  panel(28, 63, 264, 115);

  gfx_text_center(
    &np_font_title,
    160,
    103,
    "INFORMATION",
    C_YELLOW,
    256
  );

  if (message_id >= 0 &&
      message_id <= MSG_FAINT) {
    gfx_paragraph(
      &np_font_body,
      160,
      132,
      220,
      18,
      messages[message_id],
      C_WHITE,
      256
    );
  }

  gfx_text_center(
    &np_font_small,
    160,
    163,
    "OK pour continuer",
    C_MUTED,
    256
  );
}

/* =========================================================
   SCENE PRINCIPALE
   ========================================================= */

static void scene(void *ctx) {
  (void)ctx;

  switch (screen) {
    case SCREEN_TITLE:
      draw_title();
      break;

    case SCREEN_WORLD:
      draw_world();
      break;

    case SCREEN_BATTLE:
      draw_battle();
      break;

    case SCREEN_PAUSE:
      draw_pause();
      break;

    case SCREEN_CONFIRM:
      draw_confirm();
      break;

    case SCREEN_PARTY:
      draw_party();
      break;

    case SCREEN_DEX:
      draw_dex();
      break;

    case SCREEN_BAG:
      draw_bag();
      break;

    case SCREEN_QUESTS:
      draw_quests();
      break;

    case SCREEN_SHOP:
      draw_shop();
      break;

    case SCREEN_PROFILE:
      draw_profile();
      break;

    case SCREEN_BADGES:
      draw_badges();
      break;

    case SCREEN_BOTS:
      draw_bots();
      break;

    case SCREEN_CAPABILITIES:
      draw_capabilities();
      break;

    case SCREEN_RAIDS:
      draw_raids();
      break;

    case SCREEN_EGGS:
      draw_eggs();
      break;

    case SCREEN_BUDDY:
      draw_buddy();
      break;

    case SCREEN_GIFTS:
      draw_gifts();
      break;

    case SCREEN_ROUTES:
      draw_routes();
      break;

    case SCREEN_RESEARCH:
      draw_research();
      break;

    case SCREEN_PHOTOS:
      draw_photos();
      break;

    case SCREEN_TEAM:
      draw_team();
      break;

    case SCREEN_TRADING:
      draw_trading();
      break;

    case SCREEN_MESSAGE:
      draw_message();
      break;
  }
}

/* =========================================================
   MENU PAUSE
   ========================================================= */

static void pause_input(uint64_t hit) {
  if (hit & KEY(eadk_key_up)) {
    selection = (selection + 17) % 18;
  }

  if (hit & KEY(eadk_key_down)) {
    selection = (selection + 1) % 18;
  }

  if (hit & KEY(eadk_key_back)) {
    screen = SCREEN_WORLD;
    return;
  }

  if (!(hit & (KEY(eadk_key_ok) | KEY(eadk_key_exe))) &&
      !(hit & KEY(eadk_key_right))) {
    return;
  }

  switch (selection) {
    case 0:
      screen = SCREEN_WORLD;
      break;

    case 1:
      screen = SCREEN_PARTY;
      break;

    case 2:
      screen = SCREEN_DEX;
      break;

    case 3:
      screen = SCREEN_BAG;
      break;

    case 4:
      screen = SCREEN_QUESTS;
      break;

    case 5:
      screen = SCREEN_SHOP;
      break;

    case 6:
      screen = SCREEN_PROFILE;
      break;

    case 7:
      screen = SCREEN_BADGES;
      break;

    case 8:
      screen = SCREEN_CAPABILITIES;
      break;

    case 9:
      screen = SCREEN_RAIDS;
      break;

    case 10:
      screen = SCREEN_EGGS;
      break;

    case 11:
      screen = SCREEN_BUDDY;
      break;

    case 12:
      screen = SCREEN_GIFTS;
      break;

    case 13:
      screen = SCREEN_ROUTES;
      break;

    case 14:
      screen = SCREEN_RESEARCH;
      break;

    case 15:
      screen = SCREEN_PHOTOS;
      break;

    case 16:
      screen = SCREEN_TEAM;
      break;

    case 17:
      screen = SCREEN_TRADING;
      break;
  }
}

/* =========================================================
   CONFIRMATION
   ========================================================= */

static void confirm_input(uint64_t hit) {
  if (hit & (
    KEY(eadk_key_left) |
    KEY(eadk_key_right) |
    KEY(eadk_key_up) |
    KEY(eadk_key_down)
  )) {
    selection = !selection;
  }

  if (hit & KEY(eadk_key_back)) {
    screen = SCREEN_PAUSE;
    selection = 0;
    return;
  }

  if (hit & (KEY(eadk_key_ok) | KEY(eadk_key_exe))) {
    if (selection == 1) {
      save_game();
      screen = SCREEN_TITLE;
    } else {
      screen = SCREEN_PAUSE;
    }

    selection = 0;
  }
}

/* =========================================================
   BOUCLE PRINCIPALE
   ========================================================= */

int main(void) {
  np_app_begin();

  color_t *old_strip = gfx_buf;

  gfx_buf = strip;

  seed = (uint32_t)eadk_timing_millis() | 1u;

  if (!load_game()) {
    new_game();
  }

  screen = SCREEN_TITLE;

  uint64_t held = eadk_keyboard_scan();

  uint32_t next = (uint32_t)eadk_timing_millis();

  bool quit = false;

  while (!quit) {
    uint64_t keys = eadk_keyboard_scan();

    uint64_t hit = keys & ~held;

    held = keys;

    /* Home quitte proprement depuis tous les ecrans. */
    if (keys & (
      KEY(eadk_key_home) |
      KEY(eadk_key_on_off)
    )) {
      save_game();
      break;
    }

    bool ok = hit & (
      KEY(eadk_key_ok) |
      KEY(eadk_key_exe)
    );

    bool back = hit & KEY(eadk_key_back);

    /* TITRE */
    if (screen == SCREEN_TITLE) {
      if (hit & (KEY(eadk_key_up) | KEY(eadk_key_left))) {
        selection = 0;
      }
      if (hit & (KEY(eadk_key_down) | KEY(eadk_key_right))) {
        selection = 1;
      }
      if (ok) {
        if (selection == 1) {
          new_game();
        }
        selection = 0;
        screen = SCREEN_WORLD;
      }
    }

    /* EXPLORATION */
    else if (screen == SCREEN_WORLD) {
      if (ok && near_pokestop()) {
        spin_pokestop();
      }
      if (back) {
        screen = SCREEN_PAUSE;
        selection = 0;
      }

      if (screen == SCREEN_DEX) {
        if (hit & KEY(eadk_key_left)) {
          dex_page = (dex_page + 9) % 11;
        }
        if (hit & KEY(eadk_key_right)) {
          dex_page = (dex_page + 1) % 11;
        }
      }
    }

    /* COMBAT */
    else if (screen == SCREEN_BATTLE) {
      battle_input(hit);
    }

    /* PAUSE */
    else if (screen == SCREEN_PAUSE) {
      pause_input(hit);
    }

    /* CONFIRMATION */
    else if (screen == SCREEN_CONFIRM) {
      confirm_input(hit);
    }

    /* EQUIPE, POKEDEX, SAC */
    else if (
      screen == SCREEN_PARTY ||
      screen == SCREEN_DEX ||
      screen == SCREEN_BAG ||
      screen == SCREEN_QUESTS ||
      screen == SCREEN_SHOP ||
      screen == SCREEN_PROFILE ||
      screen == SCREEN_BADGES ||
      screen == SCREEN_BOTS ||
      screen == SCREEN_CAPABILITIES ||
      screen == SCREEN_RAIDS ||
      screen == SCREEN_EGGS ||
      screen == SCREEN_BUDDY ||
      screen == SCREEN_GIFTS ||
      screen == SCREEN_ROUTES ||
      screen == SCREEN_RESEARCH ||
      screen == SCREEN_PHOTOS ||
      screen == SCREEN_TEAM ||
      screen == SCREEN_TRADING
    ) {
      if (back) {
        screen = SCREEN_PAUSE;
        selection = 0;
      }

      if (screen == SCREEN_QUESTS && ok &&
          !save_data.quest_claimed &&
          save_data.quest_catches >= 5 &&
          save_data.quest_steps >= 100) {
        save_data.quest_claimed = 1;
        save_data.stardust += 500;
        save_data.coins += 50;
        save_data.pokedollars += 500;
        add_player_experience(150);
        save_game();
      }

      if (screen == SCREEN_SHOP) {
        if (hit & KEY(eadk_key_up)) {
            selection = (selection + 4) % 5;
        }
        if (hit & KEY(eadk_key_down)) {
            selection = (selection + 1) % 5;
        }
        if (ok) {
          if (selection == 4 && save_data.coins >= 40) {
            save_data.coins -= 40;
            save_data.berries += 5;
          } else if (selection == 3) {
            screen = SCREEN_CAPABILITIES;
          } else if (selection == 0 && save_data.coins >= 100) {
            save_data.coins -= 100;
            save_data.balls += 10;
          } else if (selection == 1 && save_data.coins >= 80) {
            save_data.coins -= 80;
            save_data.potions += 5;
          } else if (selection == 2 && save_data.coins >= 200) {
            save_data.coins -= 200;
            save_data.balls += 20;
            save_data.potions += 10;
            save_data.stardust += 1000;
          }
          save_game();
        }
      }

      if (screen == SCREEN_BOTS) {
        if (hit & KEY(eadk_key_up)) {
          bot_selection = (bot_selection + 3) % 4;
        }
        if (hit & KEY(eadk_key_down)) {
          bot_selection = (bot_selection + 1) % 4;
        }
        if (ok) {
          wild_species = 8 + bot_selection * 8;
          if (wild_species >= SPECIES_COUNT) wild_species = 60;
          wild_level = 8 + bot_selection * 3;
          wild_hp = species[wild_species].base_hp + wild_level * 3;
          int active = active_creature();
          if (active >= 0) {
            player_hp = creature_max_hp(save_data.party[active]);
          }
          combat_status = STATUS_OK;
          enemy_status = STATUS_OK;
          battle_selection = 0;
          screen = SCREEN_BATTLE;
        }
      }

      if (screen == SCREEN_PROFILE) {
        if (hit & (KEY(eadk_key_left) | KEY(eadk_key_right))) {
          if (save_data.pokedollars >= 100) {
            save_data.pokedollars -= 100;
            save_data.ball_style = (save_data.ball_style + 1) % 3;
            save_data.skin_style = (save_data.skin_style + 1) % 3;
            save_game();
          }
        }
      }

      if (screen == SCREEN_RAIDS && ok) {
        if (save_data.raid_wins < 255) save_data.raid_wins++;
        save_data.pokedollars += 1000;
        save_data.stardust += 2000;
        add_player_experience(300);
        save_data.badges |= 1u << 5;
        save_game();
      }

      if (screen == SCREEN_EGGS && ok) {
        if (!save_data.egg_type) {
          save_data.egg_type = 1;
          save_data.egg_distance = 0;
          save_data.egg_target = 5;
          save_game();
        }
      }

      if (screen == SCREEN_BUDDY && ok && save_data.berries) {
        save_data.berries--;
        if (save_data.buddy_hearts < 10) save_data.buddy_hearts++;
        add_player_experience(20);
        save_game();
      }

      if (screen == SCREEN_GIFTS && ok && save_data.gifts) {
        save_data.gifts--;
        save_data.pokedollars += 100;
        save_data.stardust += 100;
        save_game();
      }

      if (screen == SCREEN_ROUTES) {
        if (hit & KEY(eadk_key_up)) selection = (selection + 3) % 4;
        if (hit & KEY(eadk_key_down)) selection = (selection + 1) % 4;
        if (ok) {
          save_data.routes++;
          save_data.pokedollars += 80;
          add_player_experience(50);
          save_game();
        }
      }

      if (screen == SCREEN_RESEARCH && ok &&
          save_data.research_points >= 7) {
        save_data.research_points = 0;
        save_data.pokedollars += 500;
        save_data.stardust += 1000;
        save_game();
      }

      if (screen == SCREEN_PHOTOS && ok) {
        save_data.photos++;
        save_data.pokedollars += 50;
        save_game();
      }

      if (screen == SCREEN_TEAM) {
        if (hit & KEY(eadk_key_up)) selection = (selection + 2) % 3;
        if (hit & KEY(eadk_key_down)) selection = (selection + 1) % 3;
        if (ok) {
          save_data.team = (uint8_t)selection;
          save_game();
        }
      }

      if (screen == SCREEN_TRADING && ok) {
        if (save_data.party_count > 1) {
          save_data.trades++;
          save_data.pokedollars += 250;
          add_player_experience(100);
          save_game();
        }
      }
    }

    /* MESSAGE */
    else if (screen == SCREEN_MESSAGE) {
      if (ok || back) {
        screen = message_next;
      }
    }

    /* TEMPS FIXE */
    uint32_t now = (uint32_t)eadk_timing_millis();

    for (
      int i = 0;
      i < 5 && (int32_t)(now - next) >= 0;
      i++, next += TICK_MS
    ) {
      if (screen == SCREEN_WORLD) {
        world_tick(keys);
      }
    }

    if ((int32_t)(now - next) > 5 * TICK_MS) {
      next = now;
    }

    gfx_render(
      scene,
      0,
      0,
      SCREEN_H
    );

    eadk_timing_msleep(1);
  }

  save_game();

  gfx_buf = old_strip;

  return np_app_end();
}
