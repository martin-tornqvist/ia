#ifndef CLOSE_H
#define CLOSE_H

/*-----------------------------------------
Class for closing doors, chests(?), etc(?).
This class works only as a "bridge" between
closer and closee.
------------------------------------------*/

#include "ConstTypes.h"

class Engine;
class Actor;

class Close {
public:
    Close(Engine* engine) : eng(engine) {
    }

    ~Close() {
    }

    void playerClose() const;

private:
    Engine* eng;
};

#endif

