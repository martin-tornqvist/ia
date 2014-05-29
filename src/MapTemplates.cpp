#include "MapTemplates.h"

#include <vector>
#include <assert.h>

using namespace std;

namespace MapTemplHandling {

namespace {

MapTempl templates_[int(MapTemplId::endOfMapTemplId)];

struct CharAndFeatureId {
  CharAndFeatureId(char cha_, FeatureId featureId_) :
    cha(cha_), featureId(featureId_) {}

  CharAndFeatureId() : cha(0), featureId(FeatureId::empty) {}

  char      cha;
  FeatureId featureId;
};

FeatureId translate(const char cha,
                    const vector<CharAndFeatureId>& translations) {
  for(const CharAndFeatureId& translation : translations) {
    if(cha == translation.cha) {return translation.featureId;}
  }
  return FeatureId::empty;
}

MapTempl strToTempl(const string& str,
                    const vector<CharAndFeatureId>& translations) {
  MapTempl                    result;
  int                         strPos = 0;
  const int                   STR_SIZE = str.size();
  vector< vector<FeatureId> > resultVector;
  vector<FeatureId>           curInnerVector;

  while(strPos != STR_SIZE) {
    //If delim character found push inner vector to outer
    if(str[strPos] == ';') {
      resultVector.push_back(curInnerVector);
      curInnerVector.resize(0);
    } else if(str[strPos] == '#') {
      curInnerVector.push_back(FeatureId::wall);
    } else if(str[strPos] == '.') {
      curInnerVector.push_back(FeatureId::floor);
    } else if(str[strPos] == ' ') {
      curInnerVector.push_back(FeatureId::empty);
    } else {
      curInnerVector.push_back(translate(str[strPos], translations));
    }
    strPos++;
  }

  result.featureVector.swap(resultVector);
  result.w  = result.featureVector.back().size();
  result.h  = result.featureVector.size();

  return result;
}

void initTempls() {
  vector<CharAndFeatureId> translations;

  //---------------------------------------------------- CHURCH
  string templStr =
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


  translations.resize(0);
  translations.push_back(CharAndFeatureId(',', FeatureId::grassWithered));
  translations.push_back(CharAndFeatureId(',', FeatureId::grassWithered));
  translations.push_back(CharAndFeatureId('v', FeatureId::brazierGolden));
  translations.push_back(CharAndFeatureId('[', FeatureId::churchBench));
  translations.push_back(CharAndFeatureId('-', FeatureId::altar));
  translations.push_back(CharAndFeatureId('*', FeatureId::churchCarpet));

  templates_[int(MapTemplId::church)] = strToTempl(templStr, translations);

  //---------------------------------------------------- PHARAOH CHAMBER

  templStr =
    //               1         2         3         4         5         6         7
    //     01234567890123456789012345678901234567890123456789012345678901234567890123456789
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
  //       01234567890123456789012345678901234567890123456789012345678901234567890123456789
  //                 1         2         3         4         5         6         7
  translations.resize(0);
  translations.push_back(CharAndFeatureId('v', FeatureId::brazierGolden));
  translations.push_back(CharAndFeatureId('1', FeatureId::pillar));
  translations.push_back(CharAndFeatureId('P', FeatureId::statue));
  templates_[int(MapTemplId::egypt)] = strToTempl(templStr, translations);
}

} //namespace

void init() {
  initTempls();
}

const MapTempl& getTempl(const MapTemplId id) {
  assert(id != MapTemplId::endOfMapTemplId);
  return templates_[int(id)];
}

} //MapTemplHandling
