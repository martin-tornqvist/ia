#ifndef MAPTEMPLATES_H
#define MAPTEMPLATES_H

#include <vector>
#include <string>
#include <iostream>

#include "FeatureData.h"

using namespace std;

class Engine;

enum MapTemplateId_t {
  mapTemplate_church,
  mapTemplate_pharaohsChamber
};

struct MapTemplate {
  MapTemplate(MapTemplateId_t templateId_) :
    templateId(templateId_) {}

  MapTemplate() {
  }

  vector<vector<Feature_t> > featureVector;

  MapTemplateId_t templateId;
  int width, height;
};

class MapTemplateHandler {
public:
  MapTemplateHandler(Engine* engine) :
    eng(engine) {
    initTemplates();
  }

  MapTemplate* getTemplate(MapTemplateId_t templateId);

private:
  void initTemplates();

  struct CharToIdTranslation {
    CharToIdTranslation(char cha_, Feature_t featureId) :
      cha(cha_), featureId_(featureId) {
    }

    CharToIdTranslation() {}

    char cha;
    Feature_t featureId_;
  };

  void addTranslationToVector(vector<CharToIdTranslation>& vec, const char cha, const Feature_t featureId) {
    CharToIdTranslation t(cha, featureId);
    vec.push_back(t);
  }

  Feature_t translate(const char cha, const vector<CharToIdTranslation>& translations) {
    for(unsigned int i = 0; i < translations.size(); i++) {
      if(cha == translations.at(i).cha) {
        return translations.at(i).featureId_;
      }
    }

    return feature_empty;
  }

  MapTemplate stringToTemplate(const string str, const vector<CharToIdTranslation>& translations,
                               const MapTemplateId_t templateId);

  vector<MapTemplate> templates_;

  Engine* eng;
};

#endif
