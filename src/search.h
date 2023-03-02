#pragma once

#include "objects.h"

// players: read-only rosa_vec<player>
// returns: rosa_vec<table>
table* optimize_table_placements(weights w, player* players, uint64_t timeout_ms, uint32_t seed);
