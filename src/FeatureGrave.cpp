#include "FeatureGrave.h"

Grave::Grave(Feature_t id, Pos pos, Engine& engine) :
  FeatureStatic(id, pos, engine) {}

string Grave::getDescription(const bool DEFINITE_ARTICLE) const {
  return (DEFINITE_ARTICLE ?
          data_->name_the :
          data_->name_a) + "; " + inscription_;
}
