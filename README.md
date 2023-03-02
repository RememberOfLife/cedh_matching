# cedh matching

This project provides a random searching cEDH tournament matchmaker, and a general purpose web interface for all matchmakers adhering to the common protocol defined in this project.

## cedh tournament placement protocol
The matchmaker invoked by the web interface needs to support the folling protocol:
* Read from stdin the input format.
* Write to stdout the output format, and only that.

The input is a serialized json object:
```json
{
    "weights": {
        "rematch": 0.5,
        "non_equal_scores": 0.1,
        "3_player_table": 0.2,
    },
    "rounds": [
        [
            {
                "players": [1,2,3,4],
                "winner": 1
            },
            {
                "players": [5,6,7,8],
                "winner": 5
            },
            {
                "players": [9,10,11,12],
                "winner": 9
            },
            {
                "players": [13,14,15,16],
                "winner": 13
            }
        ]
    ],
    "players": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
}
```
`rounds` is a list of rounds, where every round is itself a list of tables. Every table has `players` and a `winner`.  
`players` is a list of player ids, where `0` is not allowed (it represents a table draw if used as a table winner). No order or continuity of players is required. This is the list of players for which the matchmaker will have to generate new table placements.

The output is a serialized json object:
```json
{
    "placements": [
        {
            "players":[1,4,8,2]
        },
        {
            "players":[9,15,7,3]
        }
    ]
}
```
`placements` is a list of tables, where each table has `players`. Only 3 or 4 player tables are allowed.

### note
The build for the matchmaker is broken right now because of an issue in the rosalia dep. (Will be fixed soon.)

