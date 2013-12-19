#include "MapTemplates.h"

#include "Engine.h"


void MapTemplateHandler::initTemplates() {
  string templateString;
  vector<CharToIdTranslation> translationVector;

  //---------------------------------------------------- CHURCH
  templateString =
    string("             ,,,,,,,,,,,     ;") +
    string("          ,,,,,######,,,,    ;") +
    string(" ,,,,,,,,,,,,,,#v..v#,,,,,   ;") +
    string(",,,,,,,,,,,,,###....###,,,,  ;") +
    string(",,#####,#,,#,#.#....#.#,,,,, ;") +
    string(",,#v.v########.#....#.######,;") +
    string("..#...#v.................v##,;") +
    string(".,#.#.#..[.[.[.[...[.[.....#,;") +
    string(".......*****************-..#,;") +
    string(".,#.#.#..[.[.[.[...[.[.....#,;") +
    string(".,#...#v.................v##,;") +
    string(",,#v.v########.#....#.######,;") +
    string(",,#####,#,,#,#.#....#.#,,,,, ;") +
    string(",,,,,,,,,,,,,###....###,,,,  ;") +
    string(" ,,,,,,,,,,,,,,#v..v#,,,,,   ;") +
    string("         ,,,,,,######,,,,    ;") +
    string("            ,,,,,,,,,,,,     ;");


  translationVector.resize(0);
  addTranslationToVector(translationVector, ',', feature_grassWithered);
  addTranslationToVector(translationVector, 'v', feature_brazierGolden);
  addTranslationToVector(translationVector, '[', feature_churchBench);
  addTranslationToVector(translationVector, '-', feature_altar);
  addTranslationToVector(translationVector, '*', feature_churchCarpet);

  templates_.push_back(stringToTemplate(templateString, translationVector, mapTemplate_church));

  //---------------------------------------------------- PHARAOH CHAMBER

  templateString =
    //                      1         2         3         4         5         6         7
    //            01234567890123456789012345678901234567890123456789012345678901234567890123456789
    string(/* 0*/"################################################################################;"/* 0*/) +
    string(/* 1*/"###...##########################################################################;"/* 1*/) +
    string(/* 2*/"###...##########################################################################;"/* 2*/) +
    string(/* 3*/"###...################################........................##################;"/* 3*/) +
    string(/* 4*/"####.################################..######################..#################;"/* 4*/) +
    string(/* 5*/"####.###############################..#########################.################;"/* 5*/) +
    string(/* 6*/"####.##############################..####v....################1....1############;"/* 6*/) +
    string(/* 7*/"####.#############################..####..###v..##############......############;"/* 7*/) +
    string(/* 8*/"####.##########################.....####..######.v############1....1############;"/* 8*/) +
    string(/* 9*/"#####.########################..######v..#######..###############.##############;"/* 9*/) +
    string(/*10*/"######.##1.........1#########.#######..##########..############..###############;"/*10*/) +
    string(/*11*/"#######.#.....P.....#########.#####...############v.##########..################;"/*11*/) +
    string(/*12*/"########............##v#v##v#.####..###....########..########..#################;"/*12*/) +
    string(/*13*/"#########.....................####.####.............v#######..##################;"/*13*/) +
    string(/*14*/"########............##v#v##v#.##1..1###...#################.v###################;"/*14*/) +
    string(/*15*/"#######.#.....P.....#########.##....######################...###################;"/*15*/) +
    string(/*16*/"######.##1.........1#########.##1..1########......#######.v##.##################;"/*16*/) +
    string(/*17*/"#####.#######################.####.########..###v..#####..####.#################;"/*17*/) +
    string(/*18*/"####.########################.####...#####..#####v..###..######.################;"/*18*/) +
    string(/*19*/"####.########################..#####..###..#######v.....########.###############;"/*19*/) +
    string(/*20*/"####.#########################...####.....#############.#########.###1...1######;"/*20*/) +
    string(/*21*/"####.###########################...####################..#########........######;"/*21*/) +
    string(/*22*/"###...############################v...##################..###########1...1######;"/*22*/) +
    string(/*23*/"###...###############################...v##############....#####################;"/*23*/) +
    string(/*24*/"###...###############################......................#####################;"/*24*/) +
    string(/*25*/"################################################################################;"/*25*/);
  //              01234567890123456789012345678901234567890123456789012345678901234567890123456789
  //                        1         2         3         4         5         6         7
  translationVector.resize(0);
  addTranslationToVector(translationVector, 'v', feature_brazierGolden);
  addTranslationToVector(translationVector, '1', feature_pillar);
  addTranslationToVector(translationVector, 'P', feature_statue);
  templates_.push_back(stringToTemplate(templateString, translationVector, mapTemplate_pharaohsChamber));
}

MapTemplate* MapTemplateHandler::getTemplate(MapTemplateId_t templateId) {
  for(MapTemplate & t : templates_) {
    if(t.templateId == templateId) return &t;
  }
  return NULL;
}


MapTemplate MapTemplateHandler::stringToTemplate(const string str, const vector<CharToIdTranslation>& translations,
    const MapTemplateId_t templateId) {
  MapTemplate             result(templateId);
  unsigned int            strPos = 0;
  const unsigned int      strSize = str.size();
  vector< vector<Feature_t> > resultVector;
  vector<Feature_t> curInnerVector;

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
  result.width  = result.featureVector.at(0).size();
  result.height = result.featureVector.size();

  return result;
}

