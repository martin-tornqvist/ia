#ifndef MARKER_H
#define MARKER_H

#include <functional>
#include <climits>

#include "CmnTypes.h"

struct KeyData;

enum class MarkerDone             {no, yes};
enum class MarkerDrawTail         {no, yes};
enum class MarkerUsePlayerTarget  {no, yes};

namespace Marker
{

void run(const MarkerDrawTail drawTail, const MarkerUsePlayerTarget useTarget,
         std::function<void(const Pos&)> onMarkerAtPos,
         std::function<MarkerDone(const Pos&, const KeyData&)> onKeyPress,
         const int EFFECTIVE_RANGE_LMT = INT_MAX);

} //Marker

#endif
