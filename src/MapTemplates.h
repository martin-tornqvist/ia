#ifndef MAPTEMPLATES_H
#define MAPTEMPLATES_H

#include <vector>
#include <string>
#include <iostream>

#include "FeatureData.h"
#include "MapArea.h"

using namespace std;

class Engine;

enum TemplateDevName_t {
	//    templateDevName_calebsGraveLeft,
	//    templateDevName_calebsGraveRight,
	templateDevName_graveYard,
	//    templateDevName_tombHouse,
	templateDevName_church,

	templateDevName_generalRoom01,
	templateDevName_generalRoom02,
	templateDevName_generalRoom03,
	templateDevName_generalRoom04,
	templateDevName_generalRoom05,
	templateDevName_generalRoom06,
	templateDevName_generalRoom07,
	templateDevName_generalRoom08,
	templateDevName_generalRoom09,
	templateDevName_generalRoom10,
	templateDevName_generalRoom11,
	templateDevName_generalRoom12,
	templateDevName_generalRoom13,
	templateDevName_generalRoom14,
	templateDevName_generalRoom15,
	templateDevName_generalRoom16,
	templateDevName_generalRoom17,
	templateDevName_generalRoom18,
	templateDevName_generalRoom19,
	templateDevName_generalRoom20,
	templateDevName_generalRoom21,
	templateDevName_generalRoom22
};

struct MapTemplate {
	MapTemplate(TemplateDevName_t devName_) :
		devName(devName_) {
	}

	MapTemplate() {
	}

    vector<vector<Feature_t> > featureVector;

	vector<coord> junctionPositions;
	TemplateDevName_t devName;
	int width, height;
};

class MapTemplateHandler {
public:
	MapTemplateHandler(Engine* engine) :
		eng(engine) {
		initTemplates();
	}

	MapTemplate* getTemplate(TemplateDevName_t templateDevName, bool generalTemplate);

	MapTemplate* getRandomTemplateForMapBuilder();

private:
	void initTemplates();

	void addJunctionsToRoomEdge(MapTemplate& t);

	//Associates a char with a landscape devName,
	//These are passed from initTemplates to stringToTemplate.
	struct CharToIdTranslation {
		CharToIdTranslation(char cha_, Feature_t featureId) :
			cha(cha_), featureId_(featureId) {
		}

		CharToIdTranslation() {
		}

		char cha;
		Feature_t featureId_;
	};

	void addTranslationToVector(vector<CharToIdTranslation>* vec, const char cha, Feature_t featureId) {
		CharToIdTranslation t(cha, featureId);
		vec->push_back(t);
	}

	Feature_t translate(const char cha, vector<CharToIdTranslation>* translations) {
		for(unsigned int i = 0; i < translations->size(); i++) {
			if(cha == translations->at(i).cha) {
				return translations->at(i).featureId_;
			}
		}

		return feature_empty;
	}

	//Converts a string of map symbols to a template of devNames, with the aid of
	//a vector of translations from symbols to devNames. Semicolon is delim char,
	//underscore means none (ie don't overwrite that mapcell)
	MapTemplate stringToTemplate(string str, vector<CharToIdTranslation>* translations, TemplateDevName_t devName);

	//        void initSize(int width, int height, MapTemplate &mapTemplate);

	vector<MapTemplate> m_templates;
	vector<MapTemplate> m_generalTemplates;

	Engine* eng;
};

#endif
