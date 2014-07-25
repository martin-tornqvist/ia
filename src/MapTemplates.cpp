#include "MapTemplates.h"

#include <vector>
#include <assert.h>

using namespace std;

namespace MapTemplHandling {

namespace {

MapTempl templates_[int(MapTemplId::END)];

struct Translation {
  Translation(const char CH_, const MapTemplCell& mapTemplCell) :
    CH(CH_), cell(mapTemplCell) {}

  const char          CH;
  const MapTemplCell  cell;
};

MapTemplCell chToCell(const char CH, const vector<Translation>& translations) {
  for(const Translation& translation : translations) {
    if(translation.CH == CH) {return translation.cell;}
  }
  assert(false && "Failed to translate char");
  return MapTemplCell();
}

void mkTempl(const string& str, const MapTemplId id,
             const vector<Translation>& translations) {
  MapTempl& templ = templates_[int(id)];

  vector<MapTemplCell> inner;

  for(const auto ch : str) {
    switch(ch) {
      case ';': {
        //Delimiting character (";") found, inner vector is pushed to outer
        templ.addRow(inner);
        inner.resize(0);
      } break;
      case '#': inner.push_back({FeatureId::wall});           break;
      case '.': inner.push_back({FeatureId::floor});          break;
      case ' ': inner.push_back({});                          break;
      default:  inner.push_back(chToCell(ch, translations));  break;
    }
  }
}

void initTempls() {
  //------------------------------------------------------------------------- CHURCH
  string str =
    "             ,,,,,,,,,,,     ;"
    "          ,,,,,######,,,,    ;"
    " ,,,,,,,,,,,,,,#v..v#,,,,,   ;"
    ",,,,,,,,,,,,,###....###,,,,  ;"
    ",,#####,#,,#,#.#....#.#,,,,, ;"
    ",,#v.v########.#....#.######,;"
    "..#...#v.................v##,;"
    ".,#.#.#..[.[.[.[...[.[....>#,;"
    ".......*****************-..#,;"
    ".,#.#.#..[.[.[.[...[.[.....#,;"
    ".,#...#v.................v##,;"
    ",,#v.v########.#....#.######,;"
    ",,#####,#,,#,#.#....#.#,,,,, ;"
    ",,,,,,,,,,,,,###....###,,,,  ;"
    " ,,,,,,,,,,,,,,#v..v#,,,,,   ;"
    "         ,,,,,,######,,,,    ;"
    "            ,,,,,,,,,,,,     ;";

  mkTempl(str, MapTemplId::church, vector<Translation> {
    {',', {FeatureId::grass}},
    {'v', {FeatureId::brazier}},
    {'[', {FeatureId::churchBench}},
    {'-', {FeatureId::altar}},
    {'*', {FeatureId::carpet}},
    {'>', {FeatureId::stairs}}
  });

  //------------------------------------------------------------------------- EGYPT
  str =
    "################################################################################;"
    "###...################################........................##################;"
    "###.1.###############################..######################..#################;"
    "###...##############################..#########################.################;"
    "####.##############################..####¤....################|....|############;"
    "####.#############################..####..###¤..##############......############;"
    "####.##########################.....####..######.¤############|....|############;"
    "#####.########################..######¤..#######..###############.##############;"
    "######.##|.........|#########.#######..##########..############..###############;"
    "#######.#.....S.....#########.#####...############¤.##########..################;"
    "########....M....C..##¤#¤##¤#.####..###....########..########..#################;"
    "#########..P..................####.####.@...........¤#######..##################;"
    "########....M....C..##¤#¤##¤#.##|..|###...#################.¤###################;"
    "#######.#.....S.....#########.##....######################...###################;"
    "######.##|.........|#########.##|..|########......#######.¤##.##################;"
    "#####.#######################.####.########..###¤..#####..####.#################;"
    "####.########################.####...#####..#####¤..###..######.################;"
    "####.########################..#####..###..#######¤.....########.###############;"
    "###...########################...####.....#############.#########.###|...|######;"
    "###.2.##########################...##################¤..##########........######;"
    "###...############################¤....................##############|...|######;"
    "################################################################################;";

  mkTempl(str, MapTemplId::egypt, vector<Translation> {
    {'@', {FeatureId::floor, ActorId::empty, ItemId::empty, 1}},  //Player starting pos
    {'¤', {FeatureId::brazier}},
    {'|', {FeatureId::pillar}},
    {'S', {FeatureId::statue}},
    {'P', {FeatureId::floor, ActorId::khephren}},
    {'M', {FeatureId::floor, ActorId::mummy}},
    {'C', {FeatureId::floor, ActorId::cultist}},
    {'1', {FeatureId::floor, ActorId::empty, ItemId::empty, 2}},  //Stair candidate #1
    {'2', {FeatureId::floor, ActorId::empty, ItemId::empty, 3}}   //Stair candidate #2
  });

  //------------------------------------------------------------------------- LENG
  str =
    "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX;"
    "%%%%%%%%%%-%%--%--%%--%%%--%%--%%-%-%%%--%-%%-X,,,,,X,,,,,X,,,,,X,,,,,,,,,,,,,,X;"
    "%@----%--------%---%---%--------%----%-----%--X,,,,,X,,,,,X,,,,,X,,,,,,,,,,,,,,X;"
    "%%--------------------------------------------XXX,XXXXX,XXXXX,XXX,,,,,,,,,,,,,,X;"
    "%-------------%--------------------------%----X,,,,,,,,,,,,,,,,,X,,,,,,,,,,,,,,X;"
    "%%------------%-------------------------------X,,,XXX,XXXXX,XXX,,,,,,,,,,,,,,,,X;"
    "%%---------------------%%%--------------%-----X,,,X,,,,,X,,,,,XXXXXXXXXXXXXXXXXX;"
    "%%%----%%----------------%--------------------X,,,X,,,,,X,,,,,XXXXXXXXXXXXXXXXXX;"
    "%%%------------------%%--%-------%------------X,,,XXXXXXXXXXXXXXXX,,,,,,,,,,,,XX;"
    "%%%%-----------------%--%%-%----%%------------X,,,,,,X,,,X,,,,,,XX,,,,,X,,,,,,XX;"
    "%%%--------------------%%---------------------XX,XXX,X,,,X,,,,,,XXX,X,,,,,X,,,XX;"
    "%%------------------%-%%-----------------------,,,,,,X,,,XXXXXX,,,1,,,,X,,,,,XXX;"
    "%%%--------------------%%--%------------------XX,XXX,X,,,X,,,,,,XXX,X,,,,,X,,,XX;"
    "%%-------------------%--%%-%----%-------------X,,,X,,XXX,X,XXXXXXX,,,,,X,,,,,,XX;"
    "%%----------------------%%--------------------X,,,X,,,,,,X,,,,,,XX,,,,,,,,,,,,XX;"
    "%%%------------------%-%%---------------%-----X,XXXXXXXXXXXXXXX,XXXXXXXXXXXXX2XX;"
    "%%-----%---------------------------%----------X,X,,,,,,,,,,,,,,,XXXXXXXXXXXXX,XX;"
    "%%%---------%---------------------------------X,X,XXX,XXX,XXX,X,XXXX,,,,XXXXX,,X;"
    "%%--%-----------------------------------------X,X,XXX,XXX,XXX,X,XX,,,XX,,,XXXX,X;"
    "%%%--------------%----------------%---%-----%-X,X,XXX,XXX,XXX,X,X,,XXXXXX,,,,X,X;"
    "%%%%%%--%---%--%%%-%--%%%%%%%-%-%%%--%%--%-%%%X,,,,,,,,,,,,,,,X,,,XXXXXXXXXX,,,X;"
    "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX;";

  mkTempl(str, MapTemplId::leng, vector<Translation> {
    {'@', {FeatureId::floor,  ActorId::empty, ItemId::empty, 1}}, //Player starting pos
    {'%', {FeatureId::wall,   ActorId::empty, ItemId::empty, 2}},
    {'X', {FeatureId::wall,   ActorId::empty, ItemId::empty, 3}},
    {',', {FeatureId::floor,  ActorId::empty, ItemId::empty, 3}},
    {'1', {FeatureId::floor,  ActorId::empty, ItemId::empty, 4}},
    {'2', {FeatureId::wall,   ActorId::empty, ItemId::empty, 5}},
    {'-', {FeatureId::grass}}
  });
}

} //namespace

void init() {
  initTempls();
}

const MapTempl& getTempl(const MapTemplId id) {
  assert(id != MapTemplId::END);
  return templates_[int(id)];
}

} //MapTemplHandling
