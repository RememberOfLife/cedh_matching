#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h> // for debug reporting

#include "rosalia/noise.h"
#include "rosalia/vector.h"
#include "rosalia/timestamp.h"

#include "objects.h"

#include "search.h"

table* optimize_table_placements(weights w, player* players, uint64_t timeout_ms, uint32_t seed)
{
    const uint8_t player_count = VEC_LEN(&players) - 1; // count that does not include 0 player

    // calc min table counts
    uint8_t table_count_4p = player_count / 4;
    uint8_t table_count_3p;
    switch (player_count % 4) {
        case 0: {
            table_count_3p = 0;
        } break;
        case 1: {
            table_count_4p -= 2;
            table_count_3p = 3;
        } break;
        case 2: {
            table_count_4p -= 1;
            table_count_3p = 2;
        } break;
        case 3: {
            table_count_3p = 1;
        } break;
    }
    uint8_t table_count = table_count_4p + table_count_3p;

    table* best_placement = NULL;
    uint32_t least_badness = UINT32_MAX;
    uint32_t iter = 0;
    table* tables = assign_random_tables(player_count, table_count_4p, table_count_3p, seed ^ iter++);
    uint32_t tables_badness = tables_calc_badness(w, players, tables);
    if (tables_badness == 0) {
        return tables;
    }
    uint64_t start_ms = timestamp_get_ms64();
    while (true) {
        if (iter++ % 32 == 0 && timestamp_get_ms64() - start_ms >= timeout_ms) {
            break;
        }

        uint32_t r1 = squirrelnoise5(iter, seed);
        if (r1 < UINT32_MAX * 1 /*0.001*/) { // small chance to do an entirely random skip //TODO pure random still performs better :(
            VEC_DESTROY(&tables);
            tables = assign_random_tables(player_count, table_count_4p, table_count_3p, seed ^ iter);
            tables_badness = tables_calc_badness(w, players, tables);
        }

        table* neighbor = NULL;
        VEC_CLONE(&neighbor, &tables);
        // generate random neighbour of position
        uint32_t c = 0;
        uint8_t table1 = squirrelnoise5(c++, r1) % table_count;
        uint8_t table2 = squirrelnoise5(c++, r1) % table_count;
        while (table2 == table1) {
            table2 = squirrelnoise5(c++, r1) % table_count;
        }
        uint8_t player1 = squirrelnoise5(c++, r1);
        uint8_t player2 = squirrelnoise5(c++, r1);
        uint8_t swap = neighbor[table1].players[player1 % neighbor[table1].player_count];
        neighbor[table1].players[player1 % neighbor[table1].player_count] = neighbor[table2].players[player2 % neighbor[table2].player_count];
        neighbor[table2].players[player2 % neighbor[table2].player_count] = swap;
        uint32_t neighbor_badness = tables_calc_badness(w, players, neighbor);

        // accept if good or random
        if ((neighbor_badness < tables_badness && r1 > UINT32_MAX * 0.5) || r1 > UINT32_MAX * 0.9) {
            VEC_DESTROY(&tables);
            tables = neighbor;
        } else {
            VEC_DESTROY(&neighbor);
        }

        if (tables_badness < least_badness) {
            VEC_DESTROY(&best_placement);
            VEC_CLONE(&best_placement, &tables);
            least_badness = tables_badness;
            if (least_badness == 0) {
                break;
            }
        }
    }

    // printf("evaluated %u placements, least badness: %u\n", iter, least_badness); //REMOVE

    return best_placement;
}
