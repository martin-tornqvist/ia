#include "FeatureVariousExaminable.h"

#include "Engine.h"
#include "Log.h"
#include "Render.h"

Tomb::Tomb(Feature_t id, coord pos, Engine* engine) :
	FeatureStatic(id, pos, engine) {

}

void Tomb::examine() {
   eng->log->addMessage("Pilla tomb");
   eng->renderer->flip();
}
