#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rosalia/argparse.h"
#include "rosalia/json.h"
#include "rosalia/noise.h"
#include "rosalia/vector.h"

#include "objects.h"
#include "search.h"
#include "tests.h"

uint8_t external_to_internal_id(player* players, uint8_t eid)
{
    for (size_t player_id = 1; player_id < VEC_LEN(&players); player_id++) {
        if (players[player_id].id == eid) {
            return player_id;
        }
    }
    return PLAYER_NONE;
}

int main(int argc, char** argv)
{
    bool json_packed_output = true;
    uint64_t timeout_ms = 200;
    uint32_t seed = 42;
    uint8_t test_player_count = 0;

    // parse args
    argp_basic ap = argp_basic_init(argc, argv);
    while (argp_basic_process(&ap) == true) {
        const char* vstr = NULL;
        if (argp_basic_arg_a0(&ap, "--json-nopack")) {
            json_packed_output = false;
        } else if (argp_basic_arg_a1(&ap, "--timeout", &vstr)) {
            if (vstr == NULL) {
                printf("error: missing timeout\n");
                return EXIT_FAILURE;
            }
            int ec = sscanf(vstr, "%lu", &timeout_ms);
            if (ec != 1) {
                printf("error: could not parse timeout\n");
                return EXIT_FAILURE;
            }
        } else if (argp_basic_arg_a1(&ap, "--seed", &vstr)) {
            if (vstr == NULL) {
                printf("error: missing timeout\n");
                return EXIT_FAILURE;
            }
            int ec = sscanf(vstr, "%u", &seed);
            if (ec != 1) {
                printf("error: could not parse seed\n");
                return EXIT_FAILURE;
            }
        } else if (argp_basic_arg_a1(&ap, "--test", &vstr)) {
            if (vstr == NULL) {
                printf("error: missing timeout\n");
                return EXIT_FAILURE;
            }
            int ec = sscanf(vstr, "%hhu", &test_player_count);
            if (ec != 1) {
                printf("error: could not parse test player count\n");
                return EXIT_FAILURE;
            }
            if (test_player_count < 6) {
                printf("error: need at least 6 players to place\n");
                return EXIT_FAILURE;
            }
        } else {
            printf("unknown arg: \"%s\"\n", ap.w_arg);
        }
    }

    // break out into test if requested
    if (test_player_count > 0) {
        test_all(test_player_count, timeout_ms, seed);
        return EXIT_SUCCESS;
    }

    // read in everything from stdin
    size_t acc_buf_len = 0;
    size_t acc_buf_size = 4096;
    char* acc_buf = (char*)malloc(acc_buf_size);
    acc_buf[0] = '\0';
    const size_t READ_BUF_SIZE = 256;
    char read_buf[READ_BUF_SIZE];
    while (fgets(read_buf, READ_BUF_SIZE, stdin) != NULL) {
        size_t read_len = strlen(read_buf);
        if (acc_buf_len + read_len >= acc_buf_size) {
            acc_buf_size += read_len + 1;
            acc_buf = (char*)realloc(acc_buf, acc_buf_size);
        }
        strcpy(acc_buf + acc_buf_len, read_buf);
        acc_buf_len += read_len;
    }

    // parse as json
    cj_ovac* data_root = cj_deserialize(acc_buf, false);
    if (data_root->type == CJ_TYPE_ERROR) {
        fprintf(stderr, "json input read error: %s\n", data_root->v.s.str);
        return EXIT_FAILURE;
    }
    free(acc_buf);

    // parse weights
    weights placement_weights;
    float wval;
    if (!cj_get_f32(data_root, "weights.rematch", &wval, NULL)) {
        fprintf(stderr, "json input read error: weights.rematch not found\n");
        return EXIT_FAILURE;
    }
    placement_weights.rematch = (uint32_t)((float)1000 * wval);
    if (!cj_get_f32(data_root, "weights.non_equal_scores", &wval, NULL)) {
        fprintf(stderr, "json input read error: weights.non_equal_scores not found\n");
        return EXIT_FAILURE;
    }
    placement_weights.noneqsc = (uint32_t)((float)1000 * wval);
    if (!cj_get_f32(data_root, "weights.3_player_table", &wval, NULL)) {
        fprintf(stderr, "json input read error: weights.3_player_table not found\n");
        return EXIT_FAILURE;
    }
    placement_weights.table3p = (uint32_t)((float)1000 * wval);

    // parse all previous rounds tables into player infos
    player* global_player_infos = NULL;
    if (false) {
        // assuming previous rounds exist and always with the same players and in order 1-N and we want to generate for exactly these:
        uint8_t* player_ids = NULL;
        VEC_PUSH(&player_ids, PLAYER_NONE);
        cj_ovac* rounds_arr = cj_find(data_root, "rounds");
        for (uint32_t round_idx = 0; round_idx < rounds_arr->child_count; round_idx++) {
            cj_ovac* tables_arr = rounds_arr->children[round_idx];
            for (uint32_t table_idx = 0; table_idx < tables_arr->child_count; table_idx++) {
                cj_ovac* table = tables_arr->children[table_idx];
                cj_ovac* table_players = cj_find(table, "players");
                for (uint32_t player_idx = 0; player_idx < table_players->child_count; player_idx++) {
                    uint8_t new_id = table_players->children[player_idx]->v.u64;
                    bool exists = false;
                    for (size_t ex_player_id = 1; ex_player_id < VEC_LEN(&player_ids); ex_player_id++) {
                        if (player_ids[ex_player_id] == new_id) {
                            exists = true;
                            break;
                        }
                    }
                    if (exists == false) {
                        VEC_PUSH(&player_ids, new_id);
                    }
                }
            }
        }
        uint8_t player_count = VEC_LEN(&player_ids); // includes the 0 player //TODO error on overflow
        VEC_DESTROY(&player_ids);
        VEC_CREATE(&global_player_infos, player_count);
        for (size_t player_idx = 0; player_idx < player_count; player_idx++) {
            player new_player = (player){
                .id = player_idx,
                .score = 0,
                .seen_opponents = NULL,
            };
            VEC_PUSH_N(&new_player.seen_opponents, player_count);
            for (size_t opp_idx = 0; opp_idx < VEC_LEN(&new_player.seen_opponents); opp_idx++) {
                new_player.seen_opponents[opp_idx] = 0;
            }
            VEC_PUSH(&global_player_infos, new_player);
        }
        for (uint32_t round_idx = 0; round_idx < rounds_arr->child_count; round_idx++) {
            cj_ovac* tables_arr = rounds_arr->children[round_idx];
            for (uint32_t table_idx = 0; table_idx < tables_arr->child_count; table_idx++) {
                cj_ovac* table = tables_arr->children[table_idx];
                cj_ovac* table_players = cj_find(table, "players");
                uint8_t table_winner_id = cj_find(table, "winner")->v.u64;
                if (table_winner_id != PLAYER_NONE) {
                    global_player_infos[table_winner_id].score += 3;
                }
                for (uint32_t player_idx = 0; player_idx < table_players->child_count; player_idx++) {
                    uint8_t player_id = table_players->children[player_idx]->v.u64;
                    if (table_winner_id == PLAYER_NONE) {
                        global_player_infos[player_id].score += 1;
                    }
                    for (uint32_t opp_idx = 0; opp_idx < table_players->child_count; opp_idx++) {
                        uint8_t opp_id = table_players->children[opp_idx]->v.u64;
                        if (opp_id != player_id) { // do this here so the eval function doesnt have to do the comparison
                            global_player_infos[player_id].seen_opponents[opp_id] += 1;
                        }
                    }
                }
            }
        }
    } else {
        // readin the player ids we will be placing
        cj_ovac* players_arr = cj_find(data_root, "players");
        if (players_arr->child_count > 254 || players_arr->child_count < 6) {
            //TODO error on overflow when to many users or not enough to fill tables
        }
        VEC_CREATE(&global_player_infos, players_arr->child_count);
        player none_player = (player){
            .id = PLAYER_NONE,
            .score = 0,
            .seen_opponents = NULL,
        };
        VEC_PUSH(&global_player_infos, none_player);
        for (uint32_t player_idx = 0; player_idx < players_arr->child_count; player_idx++) {
            //TODO there should be no duplicates here, but check it anyway
            uint8_t new_id = players_arr->children[player_idx]->v.u64;
            if (new_id == PLAYER_NONE) {
                //TODO this is not allowed, error!
            }
            bool exists = false;
            for (size_t ex_player_id = 1; ex_player_id < VEC_LEN(&global_player_infos); ex_player_id++) {
                if (global_player_infos[ex_player_id].id == new_id) {
                    exists = true;
                    break;
                }
            }
            if (exists == false) {
                player new_player = (player){
                    .id = new_id,
                    .score = 0,
                    .seen_opponents = NULL,
                };
                VEC_PUSH(&global_player_infos, new_player);
            }
        }
        for (size_t player_idx = 0; player_idx < VEC_LEN(&global_player_infos); player_idx++) {
            VEC_PUSH_N(&global_player_infos[player_idx].seen_opponents, VEC_LEN(&global_player_infos));
            for (size_t opp_idx = 0; opp_idx < VEC_LEN(&global_player_infos[player_idx].seen_opponents); opp_idx++) {
                global_player_infos[player_idx].seen_opponents[opp_idx] = 0;
            }
        }
        // readin the round history, for the players we need to place, increment their stats appropriately
        cj_ovac* rounds_arr = cj_find(data_root, "rounds");
        for (uint32_t round_idx = 0; round_idx < rounds_arr->child_count; round_idx++) {
            cj_ovac* tables_arr = rounds_arr->children[round_idx];
            for (uint32_t table_idx = 0; table_idx < tables_arr->child_count; table_idx++) {
                cj_ovac* table = tables_arr->children[table_idx];
                cj_ovac* table_players = cj_find(table, "players");
                uint8_t table_winner_id = cj_find(table, "winner")->v.u64;
                if (table_winner_id != PLAYER_NONE) {
                    uint8_t table_winner_id_int = external_to_internal_id(global_player_infos, table_winner_id);
                    if (table_winner_id_int != PLAYER_NONE) {
                        global_player_infos[table_winner_id_int].score += 3;
                    }
                }
                for (uint32_t player_idx = 0; player_idx < table_players->child_count; player_idx++) {
                    uint8_t player_id = table_players->children[player_idx]->v.u64;
                    uint8_t player_id_int = external_to_internal_id(global_player_infos, player_id);
                    if (table_winner_id == PLAYER_NONE) {
                        if (player_id_int != PLAYER_NONE) {
                            global_player_infos[player_id_int].score += 1;
                        }
                    }
                    if (player_id_int != PLAYER_NONE) {
                        for (uint32_t opp_idx = 0; opp_idx < table_players->child_count; opp_idx++) {
                            uint8_t opp_id = table_players->children[opp_idx]->v.u64;
                            if (opp_id != player_id) { // do this here so the eval function doesnt have to do the comparison
                                uint8_t opp_id_int = external_to_internal_id(global_player_infos, opp_id);
                                if (opp_id_int != PLAYER_NONE) {
                                    global_player_infos[player_id_int].seen_opponents[opp_id_int] += 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    cj_ovac_destroy(data_root);

    //REMOVE debug print players to place
    // printf("to place for %zu players\n", VEC_LEN(&global_player_infos) - 1);
    // for (size_t player_idx = 1; player_idx < VEC_LEN(&global_player_infos); player_idx++) {
    //     printf("INT P%zu:\n", player_idx);
    //     printf("    EXT %hhu\n", global_player_infos[player_idx].id);
    //     printf("    SCO %hhu\n", global_player_infos[player_idx].score);
    //     printf("    SEEN INT: ");
    //     for (size_t opp_idx = 0; opp_idx < VEC_LEN(&global_player_infos[player_idx].seen_opponents); opp_idx++) {
    //         printf(" %hhu", global_player_infos[player_idx].seen_opponents[opp_idx]);
    //     }
    //     printf("\n");
    // }

    // generate optimized table placements
    table* tables_out = optimize_table_placements(placement_weights, global_player_infos, timeout_ms, seed);

    // generate placement
    cj_ovac* out_root = cj_create_object(1);
    {
        cj_ovac* out_table_arr = cj_create_array(VEC_LEN(&tables_out));
        cj_object_append(out_root, "placements", out_table_arr);
        for (size_t table_idx = 0; table_idx < VEC_LEN(&tables_out); table_idx++) {
            cj_ovac* out_table = cj_create_object(1);
            cj_ovac* out_table_players = cj_create_array(4);
            cj_object_append(out_table, "players", out_table_players);
            for (uint8_t player_idx = 0; player_idx < tables_out[table_idx].player_count; player_idx++) {
                uint8_t player_id = global_player_infos[tables_out[table_idx].players[player_idx]].id;
                cj_array_append(out_table_players, cj_create_u64(player_id));
            }
            cj_array_append(out_table_arr, out_table);
        }
        VEC_DESTROY(&tables_out);
    }
    for (size_t player_idx = 0; player_idx < VEC_LEN(&global_player_infos); player_idx++) {
        VEC_DESTROY(&global_player_infos[player_idx].seen_opponents);
    }
    VEC_DESTROY(&global_player_infos);

    // print placement
    size_t out_size = cj_measure(out_root, json_packed_output, false);
    char* out_buf = (char*)malloc(out_size);
    cj_serialize(out_buf, out_root, json_packed_output, false);
    printf("%s", out_buf);
    cj_ovac_destroy(out_root);
    free(out_buf);

    return EXIT_SUCCESS;
}
