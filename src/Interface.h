#ifndef INTERFACE_H
#define INTERFACE_H

#include "InventoryHandler.h"

class Engine;

class Interface
{
public:
	Interface(Engine* engine) {eng = engine;}

private:
   friend class Renderer;
   void drawInfoLines();
   void drawLocationInfo();
	Engine* eng;
};


#endif

