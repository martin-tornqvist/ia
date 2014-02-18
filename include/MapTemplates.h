#ifndef MAPTEMPLATES_H
#define MAPTEMPLATES_H

#include <vector>
#include <string>
#include <iostream>

#include "FeatureData.h"

using namespace std;

class Engine;

enum MapTemplateId {
  mapTemplate_church,
  mapTemplate_pharaohsChamber
};

struct MapTemplate {
  MapTemplate(MapTemplateId templateId_) :
    templateId(templateId_) {}

  MapTemplate() {
  }

  vector<vector<FeatureId> > featureVector;

  MapTemplateId templateId;
  int w, h;
};

class MapTemplateHandler {
public:
  MapTemplateHandler() {
    initTemplates();
  }

  MapTemplate* getTemplate(MapTemplateId templateId);

private:
  void initTemplates();

  struct CharToIdTranslation {
    CharToIdTranslation(char cha_, FeatureId featureId) :
      cha(cha_), featureId_(featureId) {
    }

    CharToIdTranslation() {}

    char cha;
    FeatureId featureId_;
  };

  void addTranslationToVector(vector<CharToIdTranslation>& vec, const char cha, const FeatureId featureId) {
    CharToIdTranslation t(cha, featureId);
    vec.push_back(t);
  }

  FeatureId translate(const char cha, const vector<CharToIdTranslation>& translations) {
    for(unsigned int i = 0; i < translations.size(); i++) {
      if(cha == translations.at(i).cha) {
        return translations.at(i).featureId_;
      }
    }

    return feature_empty;
  }

  MapTemplate stringToTemplate(const string str, const vector<CharToIdTranslation>& translations,
                               const MapTemplateId templateId);

  vector<MapTemplate> templates_;
};

#endif
