#include "MapTemplates.h"

#include "Engine.h"


void MapTemplateHandler::initTemplates() {
  string templStr;
  vector<CharToIdTranslation> translationVector;

  //---------------------------------------------------- CHURCH
  templStr =
    "             ,,,,,,,,,,,     ;"
    "          ,,,,,######,,,,    ;"
    " ,,,,,,,,,,,,,,#v..v#,,,,,   ;"
    ",,,,,,,,,,,,,###....###,,,,  ;"
    ",,#####,#,,#,#.#....#.#,,,,, ;"
    ",,#v.v########.#....#.######,;"
    "..#...#v.................v##,;"
    ".,#.#.#..[.[.[.[...[.[.....#,;"
    ".......*****************-..#,;"
    ".,#.#.#..[.[.[.[...[.[.....#,;"
    ".,#...#v.................v##,;"
    ",,#v.v########.#....#.######,;"
    ",,#####,#,,#,#.#....#.#,,,,, ;"
    ",,,,,,,,,,,,,###....###,,,,  ;"
    " ,,,,,,,,,,,,,,#v..v#,,,,,   ;"
    "         ,,,,,,######,,,,    ;"
    "            ,,,,,,,,,,,,     ;";


  translationVector.resize(0);
  addTranslationToVector(translationVector, ',', feature_grassWithered);
  addTranslationToVector(translationVector, 'v', feature_brazierGolden);
  addTranslationToVector(translationVector, '[', feature_churchBench);
  addTranslationToVector(translationVector, '-', feature_altar);
  addTranslationToVector(translationVector, '*', feature_churchCarpet);

  templates_.push_back(stringToTemplate(templStr, translationVector, mapTemplate_church));

  //---------------------------------------------------- PHARAOH CHAMBER

  templStr =
    //                      1         2         3         4         5         6         7
    //            01234567890123456789012345678901234567890123456789012345678901234567890123456789
    /* 0*/"################################################################################;"/* 0*/
    /* 1*/"###...################################........................##################;"/* 1*/
    /* 2*/"###...###############################..######################..#################;"/* 2*/
    /* 3*/"###...##############################..#########################.################;"/* 3*/
    /* 4*/"####.##############################..####v....################1....1############;"/* 4*/
    /* 5*/"####.#############################..####..###v..##############......############;"/* 5*/
    /* 6*/"####.##########################.....####..######.v############1....1############;"/* 6*/
    /* 7*/"#####.########################..######v..#######..###############.##############;"/* 7*/
    /* 8*/"######.##1.........1#########.#######..##########..############..###############;"/* 8*/
    /* 9*/"#######.#.....P.....#########.#####...############v.##########..################;"/* 9*/
    /*10*/"########............##v#v##v#.####..###....########..########..#################;"/*10*/
    /*11*/"#########.....................####.####.............v#######..##################;"/*11*/
    /*12*/"########............##v#v##v#.##1..1###...#################.v###################;"/*12*/
    /*13*/"#######.#.....P.....#########.##....######################...###################;"/*13*/
    /*14*/"######.##1.........1#########.##1..1########......#######.v##.##################;"/*14*/
    /*15*/"#####.#######################.####.########..###v..#####..####.#################;"/*15*/
    /*16*/"####.########################.####...#####..#####v..###..######.################;"/*16*/
    /*17*/"####.########################..#####..###..#######v.....########.###############;"/*17*/
    /*18*/"###...########################...####.....#############.#########.###1...1######;"/*18*/
    /*19*/"###...##########################...##################v..##########........######;"/*19*/
    /*20*/"###...############################v....................##############1...1######;"/*20*/
    /*21*/"################################################################################;"/*21*/;
  //              01234567890123456789012345678901234567890123456789012345678901234567890123456789
  //                        1         2         3         4         5         6         7
  translationVector.resize(0);
  addTranslationToVector(translationVector, 'v', feature_brazierGolden);
  addTranslationToVector(translationVector, '1', feature_pillar);
  addTranslationToVector(translationVector, 'P', feature_statue);
  templates_.push_back(stringToTemplate(templStr, translationVector, mapTemplate_pharaohsChamber));
}

MapTemplate* MapTemplateHandler::getTemplate(MapTemplateId templateId) {
  for(MapTemplate & t : templates_) {
    if(t.templateId == templateId) return &t;
  }
  return NULL;
}


MapTemplate MapTemplateHandler::stringToTemplate(
  const string& str, const vector<CharToIdTranslation>& translations,
  const MapTemplateId templateId) {

  MapTemplate             result(templateId);
  unsigned int            strPos = 0;
  const unsigned int      strSize = str.size();
  vector< vector<FeatureId> > resultVector;
  vector<FeatureId> curInnerVector;

  while(strPos != strSize) {
    //If delim character found push inner vector to outer
    if(str[strPos] == ';') {
      resultVector.push_back(curInnerVector);
      curInnerVector.resize(0);
    } else if(str[strPos] == '#') {
      curInnerVector.push_back(feature_stoneWall);
    } else if(str[strPos] == '.') {
      curInnerVector.push_back(feature_stoneFloor);
    } else if(str[strPos] == ' ') {
      curInnerVector.push_back(feature_empty);
    } else {
      curInnerVector.push_back(translate(str[strPos], translations));
    }
    strPos++;
  }

  result.featureVector.swap(resultVector);
  result.w  = result.featureVector.at(0).size();
  result.h = result.featureVector.size();

  return result;
}

