

let state;
let request_pending = false;

let sync_state = false;
let sync_badge = document.getElementById("sync_badge");

function unsync() {
    sync_state = false;
    sync_badge.classList.remove("text-bg-warning");
    sync_badge.classList.remove("text-bg-success");
    sync_badge.classList.remove("text-bg-danger");
    sync_badge.classList.add("text-bg-warning");
};

function sync() {
    sync_state = true;
    sync_badge.classList.remove("text-bg-warning");
    sync_badge.classList.remove("text-bg-success");
    sync_badge.classList.remove("text-bg-danger");
    sync_badge.classList.add("text-bg-success");
};

function desync() {
    sync_state = false;
    sync_badge.classList.remove("text-bg-warning");
    sync_badge.classList.remove("text-bg-success");
    sync_badge.classList.remove("text-bg-danger");
    sync_badge.classList.add("text-bg-danger");
};

/////
// global errors

function show_error(text) {
    document.getElementById("global_error").innerText = text;
};

function clear_error() {
    document.getElementById("global_error").innerText = "";
};

/////
// container refresh and state update

function refresh_players_container() {
    let opp_win_percentage_aarr = {};
    let table_win_4p_aarr = {};
    let total_seating_number_aarr = {};
    for (let player of Object.values(state.players)) {

        let opp_win_percentage = 0;
        if (player.tiebreaker.opp_ids.length > 0) {
            for (let opp_id of player.tiebreaker.opp_ids) {
                let owp = state.players[opp_id].wins / state.players[opp_id].game_count;
                if (owp < 0.33) {
                    owp = 0.33;
                }
                opp_win_percentage += owp;
            }
            opp_win_percentage = opp_win_percentage / player.tiebreaker.opp_ids.length;
        }
        opp_win_percentage_aarr[player.id] = (opp_win_percentage).toFixed(2);

        let table_win_4p = 0;
        if (player.tiebreaker.tw4p.length > 0) {
            for (let tw4p of player.tiebreaker.tw4p) {
                table_win_4p += tw4p;
            }
            table_win_4p = (table_win_4p / player.tiebreaker.tw4p.length) * 100;
        }
        table_win_4p_aarr[player.id] = (table_win_4p).toFixed(2);

        let total_seating_number = 0;
        if (player.tiebreaker.tsn.length > 0) {
            for (let tsn of player.tiebreaker.tsn) {
                total_seating_number += tsn;
            }
        }
        total_seating_number_aarr[player.id] = total_seating_number;
    }

    let sort_players = Object.values(state.players);

    function compare_players_name(l,r) {
        if (l.name < r.name)
            return -1;
        if (l.name > r.name)
            return 1;
        return 0;
    }
    sort_players.sort(compare_players_name);

    function compare_players_4tw(l,r) {
        if (table_win_4p_aarr[l.id] < table_win_4p_aarr[r.id])
            return 1;
        if (table_win_4p_aarr[l.id] > table_win_4p_aarr[r.id])
            return -1;
        return 0;
    }
    sort_players.sort(compare_players_4tw);

    function compare_players_tsn(l,r) {
        if (total_seating_number_aarr[l.id] < total_seating_number_aarr[r.id])
            return 1;
        if (total_seating_number_aarr[l.id] > total_seating_number_aarr[r.id])
            return -1;
        return 0;
    }
    sort_players.sort(compare_players_tsn);

    function compare_players_opa(l,r) {
        if (opp_win_percentage_aarr[l.id] < opp_win_percentage_aarr[r.id])
            return 1;
        if (opp_win_percentage_aarr[l.id] > opp_win_percentage_aarr[r.id])
            return -1;
        return 0;
    }
    sort_players.sort(compare_players_opa);

    function compare_players_score(l,r) {
        if (l.score < r.score)
            return 1;
        if (l.score > r.score)
            return -1;
        return 0;
    }
    sort_players.sort(compare_players_score);

    let players_container = document.getElementById("players_container");
    while (players_container.children.length > 1) {
        players_container.removeChild(players_container.lastChild);
    }
    let i = 1;
    for (let player of sort_players) {
        let new_player = document.getElementById("template_player").cloneNode(true);
        new_player.attributes.removeNamedItem("id");
        new_player.classList.remove("d-none");

        //TODO somehow gray out inactive players
        new_player.querySelector(".jsq_rank").innerText = i++;
        new_player.querySelector(".jsq_name").innerText = player.name;
        new_player.querySelector(".jsq_id").innerText = player.id;
        new_player.querySelector(".jsq_id").parentElement.classList.add("d-none"); //TODO switch for debug mode
        new_player.querySelector(".jsq_score").innerText = player.score;
        new_player.querySelector(".jsq_tb_owp").innerText = opp_win_percentage_aarr[player.id];
        new_player.querySelector(".jsq_tb_tsn").innerText = total_seating_number_aarr[player.id];
        new_player.querySelector(".jsq_tb_4tw").innerText = table_win_4p_aarr[player.id];
        new_player.querySelector(".jsq_btn").dataset.id = player.id;
        new_player.querySelector(".jsq_btn").onclick = player.active ? remove_player : activate_player;
        new_player.querySelector(".jsq_btn").innerText = player.active ? "X" : "+";
        new_player.querySelector(".jsq_btn").classList.add(player.active ? "btn-outline-danger" : "btn-outline-success");

        players_container.appendChild(new_player);
    }
};

function refresh_tables_container() {
    let tables_container = document.getElementById("tables_container");
    while (tables_container.children.length > 1) {
        tables_container.removeChild(tables_container.lastChild);
    }
    let i = 0;
    for (let table of state.placements) {
        let new_table = document.getElementById("template_table").cloneNode(true);
        new_table.attributes.removeNamedItem("id");
        new_table.classList.remove("d-none");

        if (table.winner == 0) {
            new_table.querySelector(".jsq_table").classList.add("table-secondary");
        }
        new_table.querySelector(".jsq_table_num").innerText = i+1;
        new_table.querySelector(".jsq_btn_draw").dataset.id = i;
        new_table.querySelector(".jsq_btn_draw").onclick = table_draw;
        new_table.querySelector(".jsq_btn_draw").classList.add(table.winner == 0 ? "btn-dark" : "btn-outline-dark");
        if (table.winner == 0) {
            new_table.querySelector(".jsq_btn_draw").disabled = true;
        }
        
        let new_table_body = new_table.querySelector(".jsq_table_body");
        for (let player of table.players) {
            let new_player_line = new_table_body.querySelector(".jsq_template_name_entry").cloneNode(true);
            new_player_line.classList.remove("d-none");

            if (table.winner == player) {
                new_player_line.classList.add("table-success");
            }
            new_player_line.querySelector(".jsq_name").innerText = state.players[player].name;
            new_player_line.querySelector(".jsq_id").innerText = state.players[player].id;
            new_player_line.querySelector(".jsq_id").parentElement.classList.add("d-none"); //TODO switch for debug mode
            new_player_line.querySelector(".jsq_btn_v").dataset.tid = i;
            new_player_line.querySelector(".jsq_btn_v").dataset.pid = player;
            new_player_line.querySelector(".jsq_btn_v").onclick = table_win;
            new_player_line.querySelector(".jsq_btn_v").classList.add(table.winner == player ? "btn-success" : "btn-outline-success");
            if (table.winner == player) {
                new_player_line.querySelector(".jsq_btn_v").disabled = true;
            }
            new_table_body.appendChild(new_player_line);
        }

        tables_container.appendChild(new_table);

        i++;
    }
};

function set_primary_button(val) {
    let e_generate = document.getElementById("generate_placements");
    let e_loading = document.getElementById("generating_spinner");
    let e_confirm = document.getElementById("confirm_results");
    e_generate.classList.add("d-none")
    e_loading.classList.add("d-none")
    e_confirm.classList.add("d-none")
    switch (val) {
        case "generate": {
            e_generate.classList.remove("d-none");
        } break;
        case "loading": {
            e_loading.classList.remove("d-none");
        } break;
        case "confirm": {
            e_confirm.classList.remove("d-none");
        } break;
        default: {
            show_error("impossible primary button value");
        } break;
    }
};

/////
// button triggers

function add_player() {
    clear_error();
    let in_player_name = document.getElementById("in_player_name");
    let new_name = in_player_name.value.trim();
    in_player_name.value = "";
    if (new_name == "") {
        return;
    }
    let is_dupe = false;
    for (let player of Object.values(state.players)) {
        if (new_name == player.name) {
            is_dupe = true;
            break;
        }
    }
    if (is_dupe) {
        show_error("duplicate player name, ignored");
        return;
    }
    let new_player = {};
    new_player.id = state.next_id++;
    new_player.name = new_name;
    new_player.score = Number(0);
    new_player.wins = Number(0);
    new_player.game_count = Number(0);
    new_player.tiebreaker = {};
    new_player.tiebreaker.opp_ids = []; // opponent ids over all tables
    new_player.tiebreaker.tw4p = []; // for all wins: 1 for 4 tables, 0 for 3 tables
    new_player.tiebreaker.tsn = []; // for every table, the experienced total seating number
    new_player.active = true;
    state.players[new_player.id] = new_player;
    update_state();
};
document.getElementById("add_player").onclick = add_player;

function form_add_player_submit(e) {
    // add_player is triggered by default somehow?
    e.preventDefault();
    return false;
};
document.getElementById("form_add_player").addEventListener("submit", form_add_player_submit);

function activate_player(e) {
    clear_error();
    let pid = e.target.dataset.id;
    state.players[pid].active = true;
    update_state();
};

function remove_player(e) {
    clear_error();
    let pid = e.target.dataset.id;
    if (state.rounds.length > 0 || request_pending || state.placements.length > 0) {
        state.players[pid].active = false;
    } else {
        delete state.players[pid];
    }
    update_state();
};

function generate_placements() {
    clear_error();
    let player_ids = [];
    for (let player of Object.values(state.players)) {
        if (player.active) {
            player_ids.push(player.id); 
        }
    }
    set_primary_button("loading");
    request_pending = true;
    fetch("request.php", {
        method: "POST",
        headers: {
            'Content-Type': 'application/json; charset=UTF-8',
            'Accept': 'application/json; charset=UTF-8',
        },
        body: JSON.stringify({
            weights: {
                rematch: 0.5,
                non_equal_scores: 0.1,
                "3_player_table": 0.2,
            },
            rounds: state.rounds,
            players: player_ids,
        }),
    })
    .then((response) => response.json())
    .then((json) => receive_placements(json))
    .catch(error => {
        request_pending = false;
        show_error("ERROR: placement request failed");
        update_state();
    });
};
document.getElementById("generate_placements").onclick = generate_placements;

function confirm_results() {
    clear_error();
    for (let table of state.placements) {
        if (table.winner != 0) {
            state.players[table.winner].score += 3;
            state.players[table.winner].wins += 1;
            state.players[table.winner].tiebreaker.tw4p.push(table.players.length == 4 ? 1 : 0);
        }
        let seating_number = 1;
        for (let player of table.players) {
            state.players[player].game_count += 1;
            if (table.winner == 0) {
                state.players[player].score += 1;
            }
            state.players[player].tiebreaker.tsn.push(seating_number);
            for (let opp of table.players) {
                if (player != opp) {
                    state.players[player].tiebreaker.opp_ids.push(opp);
                }
            }
            seating_number++;
        }

    }
    state.rounds.push(state.placements);
    state.placements = [];
    update_state();
};
document.getElementById("confirm_results").onclick = confirm_results;

function table_draw(e) {
    clear_error();
    let tid = e.target.dataset.id;
    state.placements[tid].winner = Number(0);
    update_state();
};

function table_win(e) {
    clear_error();
    let tid = e.target.dataset.tid;
    let pid = e.target.dataset.pid;
    state.placements[tid].winner = Number(pid);
    update_state();
};

function remove_reset() {
    clear_error();
    localStorage.removeItem("state");
    load_state();
};
document.getElementById("delete_reset").onclick = remove_reset;

function reset_placements() {
    clear_error();
    state.placements = [];
    update_state();
};
document.getElementById("placements_reset").onclick = reset_placements;

/////
// async request handler

function receive_placements(response) {
    request_pending = false;
    if (response.hasOwnProperty("error")) {
        show_error("ERROR: " + response.error);
        update_state();
        return;
    }
    if (response.placements.length == 0 ){
        show_error("ERROR: empty placements");
        update_state();
        return;
    }
    state.placements = [];
    for (let table of response.placements) {
        for (let player of table.players) {
            if (!state.players.hasOwnProperty(player)) {
                show_error("ERROR: unknown player in placements");
                desync();
                state.placements = [];
                update_state();
                return;
            }
        }
        table.winner = Number(-1);
        state.placements.push(table);
    }
    update_state();
};

/////
// state management

function load_state() {
    let stdata = localStorage.getItem("state");
    if (stdata == null) {
        stdata = {};
        stdata.next_id = 1;
        stdata.players = {};
        stdata.rounds = [];
        stdata.placements = [];
    } else {
        stdata = JSON.parse(stdata);
    }
    state = stdata;
    update_state();
};
window.onload = load_state;

function save_state() {
    localStorage.setItem("state", JSON.stringify(state));
};

function update_state() {
    //TODO possiblity to redo table placements e.g. on update
    save_state();
    // enable / disable placement reset
    document.getElementById("placements_reset").disabled = state.placements.length == 0;
    // choose primary //TODO slightly broken
    if (request_pending) {
        set_primary_button("loading");
    } else {
        if (state.placements.length > 0) {
            set_primary_button("confirm");
        } else {
            set_primary_button("generate");
        }
    }
    // enable / disable generate button
    let active_players = 0;
    for (let player of Object.values(state.players)) {
        if (player.active) {
            active_players++;
        }
    }
    document.getElementById("generate_placements").disabled = active_players < 6;
    // enable / disable confirm button
    document.getElementById("confirm_results").disabled = false;
    for (let table of state.placements) {
        if (table.winner == -1) {
            document.getElementById("confirm_results").disabled = true;
            break;
        }
    }
    // refresh containers
    refresh_players_container();
    refresh_tables_container();
    // show the current round number
    document.getElementById("round_number").innerText = state.rounds.length + 1;
    // set sync
    sync();
};
