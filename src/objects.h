#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct weights_s {
    uint32_t rematch;
    uint32_t noneqsc;
    uint32_t table3p;
} weights;

static uint8_t PLAYER_NONE = 0;

typedef struct player_s {
    uint8_t id;
    uint8_t score;
    uint8_t* seen_opponents;
} player;

typedef struct table_s {
    uint8_t player_count;
    uint8_t players[4];
} table;

static uint8_t abs_score_diff(uint8_t s1, uint8_t s2)
{
    return s1 >= s2 ? s1 - s2 : s2 - s1;
}

// players: rosa_vec<player>
uint32_t table_calc_badness(weights w, player* players, table table);

// players: rosa_vec<player>
// tables: rosa_vec<table>
uint32_t tables_calc_badness(weights w, player* players, table* tables);

// returns: rosa_vec<table>
table* assign_random_tables(uint8_t player_count, uint8_t t4p, uint8_t t3p, uint32_t seed);
