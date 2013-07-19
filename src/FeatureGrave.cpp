#include "FeatureGrave.h"

Grave::Grave(Feature_t id, Pos pos, Engine* engine) :
  FeatureStatic(id, pos, engine) {

}

string Grave::getDescription(const bool DEFINITE_ARTICLE) const {
  return (DEFINITE_ARTICLE ? def_->name_the : def_->name_a) + "; " + inscription_;
}
