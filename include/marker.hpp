#ifndef MARKER_H
#define MARKER_H

#include <functional>
#include <climits>

#include "Cmn_types.h"

struct Key_data;

enum class Marker_done         {no, yes};
enum class Marker_draw_tail     {no, yes};
enum class Marker_use_player_tgt {no, yes};

namespace Marker
{

Pos run(const Marker_draw_tail draw_tail, const Marker_use_player_tgt use_target,
        std::function<void(const Pos&)> on_marker_at_pos,
        std::function<Marker_done(const Pos&, const Key_data&)> on_key_press,
        const int EFFECTIVE_RANGE_LMT = INT_MAX);

} //Marker

#endif
