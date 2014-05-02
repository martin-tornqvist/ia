#ifndef MARKER_H
#define MARKER_H

class Item;

struct MarkerRetData {
  MarkerRetData() : didThrowMissile(false) {}
  bool didThrowMissile;
};

namespace Marker {

MarkerRetData run(const MarkerTask markerTask, Item* itemThrown);

const Pos& getPos();

void draw(const MarkerTask markerTask);

} //Marker

#endif
