#ifndef MAPTEMPLATES_H
#define MAPTEMPLATES_H

#include <vector>
#include <string>
#include <iostream>

#include "FeatureData.h"

enum class MapTemplateId {church, pharaohsChamber};

struct MapTemplate {
  MapTemplate(MapTemplateId templateId_) : templateId(templateId_) {}

  MapTemplate() {}

  std::vector< std::vector<FeatureId> > featureVector;

  MapTemplateId templateId;
  int w, h;
};

class MapTemplateHandler {
public:
  MapTemplateHandler() {initTemplates();}

  MapTemplate* getTemplate(MapTemplateId templateId);

private:
  void initTemplates();

  struct CharToIdTranslation {
    CharToIdTranslation(char cha_, FeatureId featureId) :
      cha(cha_), featureId_(featureId) {}

    CharToIdTranslation() {}

    char cha;
    FeatureId featureId_;
  };

  void addTranslationToVector(std::vector<CharToIdTranslation>& vec,
                              const char cha, const FeatureId featureId) {
    CharToIdTranslation t(cha, featureId);
    vec.push_back(t);
  }

  FeatureId translate(const char cha,
                      const std::vector<CharToIdTranslation>& translations) {
    for(unsigned int i = 0; i < translations.size(); i++) {
      if(cha == translations.at(i).cha) {
        return translations.at(i).featureId_;
      }
    }

    return FeatureId::empty;
  }

  MapTemplate stringToTemplate(
    const std::string& str,
    const std::vector<CharToIdTranslation>& translations,
    const MapTemplateId templateId);

  std::vector<MapTemplate> templates_;
};

#endif
