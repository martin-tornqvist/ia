#ifndef MARKER_H
#define MARKER_H

#include <functional>
#include <climits>

#include "CmnTypes.h"

struct KeyData;

enum class MarkerDone         {no, yes};
enum class MarkerDrawTail     {no, yes};
enum class MarkerUsePlayerTgt {no, yes};

namespace Marker
{

Pos run(const MarkerDrawTail drawTail, const MarkerUsePlayerTgt useTarget,
        std::function<void(const Pos&)> onMarkerAtPos,
        std::function<MarkerDone(const Pos&, const KeyData&)> onKeyPress,
        const int EFFECTIVE_RANGE_LMT = INT_MAX);

} //Marker

#endif
