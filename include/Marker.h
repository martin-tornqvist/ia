#ifndef MARKER_H
#define MARKER_H

#include <functional>

#include "CmnTypes.h"

struct KeyData;

enum class MarkerDone             {no, yes};
enum class MarkerDrawTail         {no, yes};
enum class MarkerUsePlayerTarget  {no, yes};

namespace Marker {

void run(const MarkerDrawTail drawTail, const MarkerUsePlayerTarget useTarget,
         std::function<void(const Pos&)> onMarkerAtPos,
         std::function<MarkerDone(const Pos&, const KeyData&)> onKeyPress);

} //Marker

#endif
