#ifndef MARKER_HPP
#define MARKER_HPP

#include <functional>
#include <climits>

#include "rl_utils.hpp"
#include "global.hpp"

struct KeyData;
struct CellOverlay;

enum class MarkerDone              {no, yes};
enum class MarkerUsePlayerTgt    {no, yes};

//Used for specifying if marker should be drawn red when aiming past
//seen cells blocking projectiles
enum class MarkerShowBlocked      {no, yes};

namespace marker
{

P run(const MarkerUsePlayerTgt use_tgt,
      std::function<void(const P&,
                         CellOverlay overlay[map_w][map_h])> on_marker_at_pos,
      std::function<MarkerDone(const P&,
                                const KeyData&)> on_key_press,
      MarkerShowBlocked show_blocked,
      const int effective_range_lmt = INT_MAX);

} //marker

#endif
