#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "rosalia/noise.h"
#include "rosalia/vector.h"

#include "objects.h"

uint32_t table_calc_badness(weights w, player* players, table table)
{
    uint32_t badness = 0;

    for (size_t player_idx = 0; player_idx < table.player_count; player_idx++) {
        uint8_t player_id = table.players[player_idx];
        for (size_t opp_idx = 0; opp_idx < table.player_count; opp_idx++) {
            uint8_t opp_id = table.players[opp_idx];
            badness += w.rematch * players[player_id].seen_opponents[opp_id];
            badness += w.noneqsc * abs_score_diff(players[player_id].score, players[opp_id].score);
        }
    }

    if (table.player_count == 3) {
        badness += w.table3p;
    }

    return badness;
}

uint32_t tables_calc_badness(weights w, player* players, table* tables)
{
    uint32_t badness = 0;
    for (size_t i = 0; i < VEC_LEN(&tables); i++) {
        badness += table_calc_badness(w, players, tables[i]);
    }
    return badness;
}

//TODO for performance reasons we should not recreate the table vec but rather write into an already existing one
table* assign_random_tables(uint8_t player_count, uint8_t t4p, uint8_t t3p, uint32_t seed)
{
    uint8_t* open_ids = NULL;
    VEC_CREATE(&open_ids, player_count);
    for (size_t player_id = 1; player_id < player_count + 1; player_id++) {
        VEC_PUSH(&open_ids, player_id);
    }

    table* tables = NULL;
    VEC_CREATE(&tables, t4p + t3p);
    uint8_t wt4p = t4p;
    uint8_t wt3p = t3p;
    table curr_table = (table){.player_count = 0};

    uint32_t ctr = 0;
    for (size_t i = VEC_LEN(&open_ids); i > 0; i--) {
        size_t id_idx = squirrelnoise5(ctr++, seed) % VEC_LEN(&open_ids); //TODO this had modulo bias!
        curr_table.players[curr_table.player_count++] = open_ids[id_idx];
        VEC_REMOVE_SWAP(&open_ids, id_idx);
        bool table_done = false;
        if (wt4p > 0) {
            table_done = curr_table.player_count == 4;
            if (table_done == true) {
                wt4p--;
            }
        } else {
            table_done = curr_table.player_count == 3;
            if (table_done == true) {
                wt3p--;
            }
        }
        if (table_done == true) {
            VEC_PUSH(&tables, curr_table);
            curr_table.player_count = 0;
        }
    }
    VEC_DESTROY(&open_ids);

    return tables;
}
