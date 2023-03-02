#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "rosalia/noise.h"
#include "rosalia/vector.h"

#include "objects.h"
#include "search.h"

#include "tests.h"

typedef struct meta_player_s {
    uint8_t id; // internal id
    uint32_t skill; // very rudimentary integer math only skill evaluation
    uint32_t acc_score;
    uint32_t wins;
} meta_player;

uint8_t get_table_winner(meta_player* meta_players, table t, uint32_t seed)
{
    //TODO include chance for draw
    uint32_t max_skill = 0;
    for (uint8_t pi = 0; pi < t.player_count; pi++) {
        max_skill += meta_players[t.players[pi]].skill;
    }
    uint32_t racc = 0;
    uint32_t r = squirrelnoise5(17, seed) % max_skill; //BUG modulo bias
    for (uint8_t pi = 0; pi < t.player_count; pi++) {
        racc += meta_players[t.players[pi]].skill;
        if (r < racc) {
            return t.players[pi];
        }
    }
    return t.player_count - 1;
}

void test_all(uint8_t player_count, uint64_t timeout_ms, uint32_t seed)
{
    printf("testing %hhu players, %lums, seed:%u\n", player_count, timeout_ms, seed);

    const weights w = (weights){
        .rematch = 500,
        .noneqsc = 100,
        .table3p = 200,
    };

    // generate meta players and players
    const uint32_t base_skill = 1000;
    const uint32_t skill_inc = 2000;
    meta_player* meta_players = NULL;
    VEC_CREATE(&meta_players, player_count + 1);
    for (uint8_t player_id = 0; player_id < player_count + 1; player_id++) {
        meta_player new_meta_player = (meta_player){
            .id = player_id,
            .skill = base_skill + skill_inc * player_id,
            .acc_score = 0,
            .wins = 0,
        };
        VEC_PUSH(&meta_players, new_meta_player);
    }

    // play multiple "tournaments" with multiple rounds each and record avg stats and wins
    const uint32_t max_iter = 300;
    const uint32_t max_rounds = 6;
    for (uint32_t iter = 0; iter < max_iter; iter++) {
        printf("\r%u", iter);
        fflush(stdout);
        // create players for this tournament
        player* players = NULL;
        VEC_CREATE(&players, player_count + 1);
        for (uint8_t player_id = 0; player_id < player_count + 1; player_id++) {
            player new_player = (player){
                .id = player_id,
                .score = 0,
                .seen_opponents = NULL,
            };
            VEC_PUSH_N(&new_player.seen_opponents, player_count + 1);
            for (size_t opp_id = 0; opp_id < VEC_LEN(&new_player.seen_opponents); opp_id++) {
                new_player.seen_opponents[opp_id] = 0;
            }
            VEC_PUSH(&players, new_player);
        }
        // play rounds
        for (uint32_t round = 0; round < max_rounds; round++) {
            uint32_t round_seed = squirrelnoise5(iter * max_rounds + round, seed);
            table* placements = optimize_table_placements(w, players, timeout_ms, round_seed);
            for (size_t table_idx = 0; table_idx < VEC_LEN(&placements); table_idx++) {
                uint8_t table_winner = get_table_winner(meta_players, placements[table_idx], squirrelnoise5(table_idx, round_seed));
                if (table_winner != PLAYER_NONE) {
                    players[table_winner].score += 3;
                }
                // record all seen enemies
                for (uint8_t pi = 0; pi < placements[table_idx].player_count; pi++) {
                    uint8_t player_id = placements[table_idx].players[pi];
                    if (table_winner == PLAYER_NONE) {
                        players[player_id].score += 1;
                    }
                    for (uint8_t oi = 0; oi < placements[table_idx].player_count; oi++) {
                        uint8_t opp_id = placements[table_idx].players[oi];
                        players[player_id].seen_opponents[opp_id] += 1;
                    }
                }
            }
            VEC_DESTROY(&placements);
        }
        // record winner and acc score
        uint8_t winner_id = 0;
        uint8_t winner_score = 0;
        for (size_t player_id = 1; player_id < VEC_LEN(&players); player_id++) {
            uint8_t player_score = players[player_id].score;
            meta_players[player_id].acc_score += player_score;
            if (player_score > winner_score) {
                winner_id = player_id;
                winner_score = player_score;
            }
        }
        meta_players[winner_id].wins += 1;
        // destroy players
        for (size_t player_id = 0; player_id < VEC_LEN(&players); player_id++) {
            VEC_DESTROY(&players[player_id].seen_opponents);
        }
        VEC_DESTROY(&players);
    }
    printf("\n");
    fflush(stdout);

    // print player infos
    for (uint8_t player_id = 1; player_id < player_count; player_id++) {
        printf("P%hhu\n", player_id);
        printf("    TRU SKIL: %u\n", meta_players[player_id].skill);
        printf("    AVG SCOR: %u\n", meta_players[player_id].acc_score / max_iter);
        printf("    TOT WINS: %u\n", meta_players[player_id].wins);
    }
}