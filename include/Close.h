#ifndef CLOSE_H
#define CLOSE_H

#include "CmnData.h"


class Actor;
class Feature;

class Close {
public:
  Close() : eng() {
  }

  ~Close() {
  }

  void playerClose() const;

private:
  friend class Feature;
  void playerCloseFeature(Feature* const feature) const;


};

#endif

