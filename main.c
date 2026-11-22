
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
#define SPECIES_COUNT 16

#define WORLD_SIZE 64

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
} Creature;

typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint8_t version;
  uint8_t party_count;
  uint8_t balls;
  uint8_t potions;

  int16_t world_x;
  int16_t world_y;

  uint16_t dex;
  uint32_t steps;
  uint32_t stardust;
  uint32_t coins;
  uint32_t captured;
  uint32_t distance;
  uint16_t quest_catches;
  uint16_t quest_steps;
  uint8_t quest_claimed;

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
  }
};

/* =========================================================
   VARIABLES GLOBALES
   ========================================================= */

static color_t strip[SCREEN_W * STRIP_H];

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
  save_data.version = 2;

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
      candidate.version != 2 ||
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

  player_x = clamp(candidate.world_x, 0, WORLD_SIZE - 1);
  player_y = clamp(candidate.world_y, 0, WORLD_SIZE - 1);

  return true;
}

static void new_game(void) {
  clear_bytes(&save_data, sizeof(save_data));

  player_x = 32;
  player_y = 32;

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

  save_data.party[0].species = 0;
  save_data.party[0].level = 5;
  save_data.party[0].xp = 0;

  save_data.dex = 1;

  save_game();
}

/* =========================================================
   POKEDEX
   ========================================================= */

static void register_creature(int id) {
  if (id < 0 || id >= SPECIES_COUNT) return;

  save_data.dex |= (uint16_t)(1u << id);
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

/* =========================================================
   CARTE ET TERRAIN
   ========================================================= */

static int terrain(int x, int y) {
  uint32_t h =
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

  /* Stocke la creature sauvage dans une variable temporaire. */
  wild_species = id;
  wild_level = 2 + rnd(5);

  register_creature(id);

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
  int damage = species[wild_species].base_attack / 2 + 1;

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

  wild_hp -= damage;

  if (wild_hp <= 0) {
    evolution_happened = add_experience(c, 5);
    save_data.stardust += 75 + wild_level * 10;
    save_data.coins += 10;

    save_game();

    show_message(
      evolution_happened ? MSG_EVOLUTION : MSG_VICTORY,
      SCREEN_WORLD
    );

    return;
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
    save_data.stardust += 100 + wild_level * 20;
    save_data.coins += 3;

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
  }
}

static void battle_input(uint64_t hit) {
  if (hit & KEY(eadk_key_left)) {
    battle_selection =
      (battle_selection + 3) % 4;
  }

  if (hit & KEY(eadk_key_right)) {
    battle_selection =
      (battle_selection + 1) % 4;
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

static void draw_creature(int id, int x, int y, int scale) {
  if (id < 0 || id >= SPECIES_COUNT) return;

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

  choice(190, "New / Continue", true);

  gfx_text_center(
    &np_font_small,
    160,
    235,
    "OK: play  Home: quit",
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

  /* Personnage au centre. */
  gfx_circle(160, 126, 11, HEX(0x183B54), 256);
  gfx_circle(160, 123, 7, C_YELLOW, 256);
  gfx_circle(160, 120, 3, C_WHITE, 256);
  gfx_fill(156, 128, 8, 11, C_BLUE);
  gfx_fill(158, 138, 3, 5, C_DARK);
  gfx_fill(164, 138, 3, 5, C_DARK);

  header("POKEMON GO");

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
    "Sauvage",
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

  const char *options[4] = {
    "Attaque",
    "Capsule",
    "Potion",
    "Fuite"
  };

  for (int i = 0; i < 4; i++) {
    int x = 47 + i * 75;

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
    "Back: retour",
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

  for (int i = 0; i < SPECIES_COUNT; i++) {
    int col = i % 5;
    int row = i / 5;

    int x = 32 + col * 64;
    int y = 62 + row * 55;

    panel(
      x - 28,
      y - 23,
      56,
      48
    );

    if (save_data.dex & (1u << i)) {
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

  panel(64, 30, 192, 202);

  gfx_text_center(
    &np_font_title,
    160,
    62,
    "PAUSE",
    C_YELLOW,
    256
  );

  choice(82, "Reprendre", selection == 0);
  choice(104, "Equipe", selection == 1);
  choice(126, "Pokedex", selection == 2);
  choice(148, "Sac", selection == 3);
  choice(170, "Missions", selection == 4);
  choice(192, "Boutique", selection == 5);
  choice(214, "Quitter", selection == 6);
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
    selection = (selection + 6) % 7;
  }

  if (hit & KEY(eadk_key_down)) {
    selection = (selection + 1) % 7;
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
      selection = 0;
      screen = SCREEN_CONFIRM;
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
      if (ok) {
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
      screen == SCREEN_SHOP
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
        save_game();
      }

      if (screen == SCREEN_SHOP) {
        if (hit & KEY(eadk_key_up)) {
          selection = (selection + 2) % 3;
        }
        if (hit & KEY(eadk_key_down)) {
          selection = (selection + 1) % 3;
        }
        if (ok) {
          if (selection == 0 && save_data.coins >= 100) {
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
