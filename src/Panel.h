#ifndef INTERFACE_PANEL_H
#define INTERFACE_PANEL_H

#include "CommonTypes.h"

enum Panel_t {panel_screen, panel_map, panel_character, panel_log};

class Panel {
public:
  Panel(const Pos& dims, const Pos& offset, Font_t standardFont,
        const Pos& bigFontDims) :
    dims_(dims), offset_(offset), standardFont_(standardFont),
    bigFontDims_(bigFontDims) {}
  ~Panel() {}

  inline Pos getPixelDims()   const {return Pos(dims_ * bigFontDims_);}
  inline Pos getPixelOffset() const {return Pos(offset_ * bigFontDims_);}

  const Pos dims_;
  const Pos offset_;
  const Font_t standardFont_;
  const Pos bigFontDims_;

private:
};

#endif
