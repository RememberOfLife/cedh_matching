<?php
error_reporting(E_ALL);
ini_set('display_errors', '1');
?>
<!DOCTYPE html>
<html lang="en">

    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>cEDH Matchmaker</title>
        <link rel="stylesheet" href="index.css">
        <!-- TODO make bootstrap static -->
        <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0-alpha1/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-GLhlTQ8iRABdZLl6O3oVMWSktQOp6b7In1Zl3/Jr59b6EGGoI1aFkw7cmDA6j6gD" crossorigin="anonymous">
    </head>
    <body id="page-container">
        <div id="content-wrap" class="container-fluid mt-3">

            <div class="row">

                <div class="col-xl-3">

                    <span class="text-danger" id="global_error"></span>

                </div>

                <div class="col-xl-6">


                    <h1 class="d-inline">cEDH Matchmaker</h1>
                    <span class="badge text-bg-danger float-end mt-3" id="sync_badge">SYNC</span>
                    <br>
                    <hr class="mt-1">
                    <table class="table table-sm table-striped align-middle">
                        <thead>
                            <tr>
                                <th class="fit">#</th>
                                <th>Name</th>
                                <th>Score</th>
                                <!-- tiebreaker stats e.g. opponent win percent -->
                                <th class="fit"></th>
                            </tr>
                        </thead>
                        <tbody class="table-group-divider" id="players_container">
                            <tr class="d-none" id="template_player">
                                <th class="jsq_rank"></th>
                                <td><span class="jsq_name"></span> <small class="text-muted">(<span class="jsq_id"></span>)</small></td>
                                <td class="jsq_score"></td>
                                <td><button type="button" class="jsq_btn btn btn-sm"></button></td>
                            </tr>
                        </tbody>
                    </table>

                    <!-- <div class="row g-3 align-items-center">
                        <div class="col-auto">
                        <label class="col-form-label" for="in_player_name">Add Player:</label>
                        </div>
                        <div class="col-auto">
                            <input type="text" class="form-control" id="in_player_name" placeholder="Name"></input>
                        </div>
                        <div class="col-auto">
                            <button type="button" class="btn btn btn-outline-success" id="add_player">+</button>
                        </div>
                    </div> -->

                    <form id="form_add_player">
                        <div class="row g-3 align-items-center">
                            <div class="col-auto">
                            <label class="col-form-label" for="in_player_name">Add Player:</label>
                            </div>
                            <div class="col-auto">
                                <input type="text" class="form-control" id="in_player_name" placeholder="Name"></input>
                            </div>
                            <div class="col-auto">
                                <button type="submit" class="btn btn btn-outline-success" id="add_player">+</button>
                            </div>
                        </div>
                    </form>

                    <hr>

                    <div class="d-grid">
                        <button type="button" class="btn btn-outline-primary" id="generate_placements">generate placements</button>
                        <div class="d-none" id="generating_spinner">
                            <div class="d-flex justify-content-center">
                                <div class="spinner-border"></div>
                            </div>
                        </div>
                        <button type="button" class="btn btn-outline-primary d-none" id="confirm_results">confirm results</button>
                    </div>

                    <hr>

                    <div class="row" id="tables_container">

                        <div class="col-sm-6 mb-4 d-none" id="template_table">
                            <div class="card">
                                <div class="card-body">
                                    <table class="table table-sm align-middle mb-0 jsq_table">
                                        <thead>
                                            <tr>
                                                <th>Table <span class="jsq_table_num"></span>:</th>
                                                <th class="fit"><button type="button" class="btn btn-sm jsq_btn_draw">D</button></th>
                                            </tr>
                                        </thead>
                                        <tbody class="table-group-divider jsq_table_body">
                                            <tr class="jsq_template_name_entry d-none">
                                                <td class="jsq_name"></td>
                                                <td><button type="button" class="btn btn-sm jsq_btn_v">V</button></td>
                                            </tr>
                                        </tbody>
                                    </table>
                                </div>
                            </div>
                        </div>

                    </div>


                </div>

                <div class="col-xl-3">

                    <div class="d-grid">
                        <button type="button" class="btn btn-sm btn-outline-danger mb-3 ms-3" id="delete_reset">delete all data and reset</button>
                    </div>

                    <div class="d-grid">
                        <button type="button" class="btn btn-sm btn-outline-warning mb-3 ms-3" id="placements_reset">reset placements</button>
                    </div>

                    <!-- TODO collapsable command history for debugging purposes? -->

                </div>

                <!-- TODO navbar bottom with imprint and open source link -->

            </div>

        </div>

        <script src="index.js"></script>
        <!-- TODO make bootstrap static -->
        <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0-alpha1/dist/js/bootstrap.bundle.min.js" integrity="sha384-w76AqPfDkMBDXo30jS1Sgez6pr3x5MlQ1ZAGC+nuZB+EYdgRZgiwxhTBTkF7CXvN" crossorigin="anonymous"></script>

    </body>
</html>
