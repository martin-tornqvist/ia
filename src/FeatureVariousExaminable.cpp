#include "FeatureVariousExaminable.h"

#include "Engine.h"
#include "Log.h"
#include "Render.h"


//------------------------------------------------------------------ BASE CLASS
FeatureExaminable::FeatureExaminable(Feature_t id, coord pos, Engine* engine) :
  FeatureStatic(id, pos, engine), isExaminableFurther_(true) {

}

sf::Color FeatureExaminable::getColorBg() const {
  return isExaminableFurther_ ? clrBlue : clrBlack;
}

void FeatureExaminable::examine() {
  tracer << "FeatureExaminable::examine()..." << endl;

  if(isExaminableFurther_) {
    tracer << "FeatureExaminable: Feature is further examinable, calling featureSpecific_examine()" << endl;
    featureSpecific_examine();
    eng->gameTime->letNextAct();
  } else {
    tracer << "FeatureExaminable: Feature can not be further examined, exiting with no action" << endl;
    eng->log->addMessage("[EXAMINE DONE]");
  }

  tracer << "FeatureExaminable::examine() [DONE]" << endl;
}

//------------------------------------------------------------------ SPECIFIC FEATURES
Tomb::Tomb(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {

}

void Tomb::featureSpecific_examine() {
  isExaminableFurther_ = false;
}

Cabinet::Cabinet(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {

}

void Cabinet::featureSpecific_examine() {
  isExaminableFurther_ = false;
}

Chest::Chest(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {

}

void Chest::featureSpecific_examine() {
  isExaminableFurther_ = false;
}

Cocoon::Cocoon(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {

}

void Cocoon::featureSpecific_examine() {
  isExaminableFurther_ = false;
}

Altar::Altar(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {

}

void Altar::featureSpecific_examine() {
  isExaminableFurther_ = false;
}

CarvedPillar::CarvedPillar(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {

}

void CarvedPillar::featureSpecific_examine() {
  isExaminableFurther_ = false;
}

Barrel::Barrel(Feature_t id, coord pos, Engine* engine) :
  FeatureExaminable(id, pos, engine) {

}

void Barrel::featureSpecific_examine() {
  isExaminableFurther_ = false;
}
