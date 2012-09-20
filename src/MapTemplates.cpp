#include "MapTemplates.h"

#include "Engine.h"


//---------------------------------------------------TEMPLATES
void MapTemplateHandler::initTemplates()
{
    MapTemplate templ;

    string templateString;
    vector<CharToIdTranslation> translationVector;
    coord junction;

    //---------------GENERAL TEMPLATES FOR MAPBUILDER
    templateString =
        string(".................;")+
        string(".................;")+
        string("...#.#.#.#.#.#...;")+
        string(".................;")+
        string(".................;")+
        string(".................;")+
        string("...#.#.#.#.#.#...;")+
        string(".................;")+
        string(".................;");
    templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom01);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
        string("#.#.#.#.#.#;")+
        string("...........;")+
        string("...........;")+
        string("...........;")+
        string("#.#.#.#.#.#;");
    templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom02);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
        string("###.....###;")+
        string("###.....###;")+
        string("###.....###;")+
        string("...........;")+
        string(".....#.....;")+
        string("....###....;")+
        string(".....#.....;")+
        string("...........;")+
        string("###.....###;")+
        string("###.....###;")+
        string("###.....###;");
    templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom03);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
        string("##...##;")+
        string("##...##;")+
        string(".......;")+
        string("...#...;")+
        string(".......;")+
        string("##...##;")+
        string("##...##;");
    templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom04);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
        string(".......................;")+
        string(".......................;")+
        string("..###.#####.#####.###..;")+
        string("..#.....#.....#.....#..;")+
        string(".......................;")+
        string("..#.....#.....#.....#..;")+
        string("..###.#####.######.##..;")+
        string(".......................;")+
        string(".......................;");
    templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom05);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
        string(".......................;")+
        string(".......................;")+
        string("..###.#####.#####.###..;")+
        string("..#.....#.....#.....#..;")+
        string("..#.....#.....#.....#..;")+
        string(".......................;")+
        string("..#.....#.....#.....#..;")+
        string("..#.....#.....#.....#..;")+
        string("..###.#####.#####.###..;")+
        string(".......................;")+
        string(".......................;");
    templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom06);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
        string(".................;")+
        string(".................;")+
        string("..######.######..;")+
        string("..#....#.#....#..;")+
        string("..#....#.#....#..;")+
		string("..#...........#..;")+
        string("..#....#.#....#..;")+
        string("..###.##.##.###..;")+
        string(".................;")+
        string("..###.##.##.###..;")+
        string("..#....#.#....#..;")+
        string("..#...........#..;")+
        string("..#....#.#....#..;")+
        string("..#....#.#....#..;")+
        string("..######.######..;")+
        string(".................;")+
        string(".................;");
    templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom07);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
        string("....#.#....;")+
        string("....#.#....;")+
        string("...........;")+
        string("....#.#....;")+
        string("##.##.##.##;")+
        string("...........;")+
        string("##.##.##.##;")+
        string("....#.#....;")+
        string("...........;")+
        string("....#.#....;")+
        string("....#.#....;");
    templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom08);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
        string("....#........#....;")+
        string("....#........#....;")+
        string("..................;")+
        string("....#........#....;")+
        string("##.##........##.##;")+
        string("..................;")+
        string("##.##........##.##;")+
        string("....#........#....;")+
        string("..................;")+
        string("....#........#....;")+
        string("....#........#....;");
    templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom09);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
        string("#...#...#...#...#...#...#...#;")+
        string(".............................;")+
        string("..############.############..;")+
        string("..#.......................#..;")+
        string("..#......#....#....#......#..;")+
        string("..#.......................#..;")+
        string("..#.#.#..#.........#..#.#.#..;")+
        string("..............#..............;")+
        string("..#.#.#..#.........#..#.#.#..;")+
        string("..#.......................#..;")+
        string("..#......#....#....#......#..;")+
        string("..#.......................#..;")+
        string("..############.############..;")+
        string(".............................;")+
        string("#...#...#...#...#...#...#...#;");
    templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom10);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
		string("#.#.#.#.#.#;")+
        string("...........;")+
		string("#.#.#.#.#.#;");
	templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom11);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
		string("#.#;")+
		string("...;")+
		string("#.#;")+
		string("...;")+
		string("#.#;")+
		string("...;")+
		string("#.#;")+
		string("...;")+
		string("#.#;")+
		string("...;")+
		string("#.#;");
	templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom12);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
		string(".........;")+
		string("########.;")+
		string(".......#.;")+
		string(".......#.;")+
		string(".......#.;")+
		string(".......#.;")+
		string(".......#.;")+
		string(".......#.;")+
		string(".........;");
    templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom13);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
		string(".........;")+
		string(".#.......;")+
		string(".#.......;")+
		string(".#.......;")+
		string(".#.......;")+
		string(".#.......;")+
		string(".#.......;")+
		string(".########;")+
		string(".........;");
    templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom14);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
		string(".........;")+
		string(".........;")+
		string(".........;")+
		string("...###...;")+
		string("...###...;")+
		string("...###...;")+
		string(".........;")+
		string(".........;")+
		string(".........;");
    templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom15);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
		string("#.............#;")+
		string("...............;")+
		string("#.............#;");
	templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom16);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
		string("...............;")+
		string("...#...#...#...;")+
		string("...............;");
	templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom17);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
		string("#.#;")+
		string("...;")+
		string("...;")+
		string("...;")+
		string("...;")+
		string("...;")+
		string("...;")+
		string("...;")+
		string("...;")+
		string("...;")+
		string("#.#;");
	templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom18);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
		string("...;")+
		string("...;")+
		string(".#.;")+
		string("...;")+
		string("...;")+
		string(".#.;")+
		string("...;")+
		string("...;")+
		string(".#.;")+
		string("...;")+
		string("...;");
	templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom19);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
		string(".........;")+
		string(".#######.;")+
		string(".#######.;")+
		string(".#######.;")+
		string(".#######.;")+
		string(".#######.;")+
		string(".........;");
	templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom20);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);

    templateString =
		string("............;")+
		string(".##########.;")+
		string(".##########.;")+
		string(".##########.;")+
		string(".##########.;")+
		string(".##########.;")+
		string(".##########.;")+
		string(".##########.;")+
		string("............;");
	templ = stringToTemplate(templateString, NULL, templateDevName_generalRoom21);
    addJunctionsToRoomEdge(templ);
    m_generalTemplates.push_back(templ);


    //---------------SPECIAL TEMPLATES
	//--------------------------------  Graveyard
    templateString =
        string("ooooooooooooo;")+
        string("ootototototoo;")+
        string("ooooooooooooo;")+
        string("ootototototoo;")+
        string("ooooooooooooo;")+
        string("ootototototoo;")+
        string("ooooooooooooo;")+
        string("ooooooooooooo;")+
        string("ooooooooooooo;");

    translationVector.resize(0);
    addTranslationToVector(&translationVector, 't', feature_gravestone);
    addTranslationToVector(&translationVector, 'o', feature_grass);
    m_templates.push_back( stringToTemplate(templateString, &translationVector, templateDevName_graveYard) );

    //--------------------------------  Tomb-house
//    templateString =
//        string("ggggggg;")+
//        string("ggggggg;")+
//        string("ggtotgg;")+
//        string("ggtotgg;")+
//        string("ggtttgg;")+
//        string("ggggggg;")+
//        string("ggggggg;");
//
//    translationVector.resize(0);
//    addTranslationToVector(&translationVector, 't', landscDevToId(feature_stoneWall));
//    addTranslationToVector(&translationVector, 'o', landscDevToId(feature_stoneFloor));
//    addTranslationToVector(&translationVector, 'g', landscDevToId(feature_grass));
//    m_templates.push_back( stringToTemplate(templateString, &translationVector, templateDevName_tombHouse) );

    //--------------------------------  Church
    templateString =
        string("_____________,,,,,,,,,,,_____;")+
        string("__________,,,,,XXXXXX,,,,____;")+
        string("_,,,,,,,,,,,,,,XI..IX,,,,,___;")+
        string(",,,,,,,,,,,,,XXX....XXX,,,,__;")+
        string(",,XXXXX,X,,X,X.X....X.X,,,,,_;")+
        string(",,XI.IXXXXXXXX.X....X.XXXXXX,;")+
        string("..X...XI.................IXX,;")+
        string(".,X.X.X..[.[.[.[...[.[.....X,;")+
        string(".......*****************-..X,;")+
        string(".,X.X.X..[.[.[.[...[.[.....X,;")+
        string(".,X...XI.................IXX,;")+
        string(",,XI.IXXXXXXXX.X....X.XXXXXX,;")+
        string(",,XXXXX,X,,X,X_X....X.X,,,,,_;")+
        string(",,,,,,,,,,,,,XXX....XXX,,,,__;")+
        string("_,,,,,,,,,,,,,,XI..IX,,,,,___;")+
        string("_________,,,,,,XXXXXX,,,_____;")+
        string("____________,,,,,,,,,,,,_____;");


    translationVector.resize(0);
    addTranslationToVector(&translationVector, 'X', feature_stoneWall);
    addTranslationToVector(&translationVector, '.', feature_stoneFloor);
    addTranslationToVector(&translationVector, ',', feature_grassWithered);
    addTranslationToVector(&translationVector, 'I', feature_brazierGolden);
    addTranslationToVector(&translationVector, '[', feature_churchBench);
    addTranslationToVector(&translationVector, '-', feature_altar);
    addTranslationToVector(&translationVector, '*', feature_churchCarpet);

    m_templates.push_back( stringToTemplate(templateString, &translationVector, templateDevName_church) );
    //--------------------------------
}

void MapTemplateHandler::addJunctionsToRoomEdge(MapTemplate& t)
{
    coord junction;

    junction.x = t.width+1;     junction.y =  (t.height-1)/2;
    t.junctionPositions.push_back(junction);

    junction.x = (t.width-1)/2; junction.y = -2;
    t.junctionPositions.push_back(junction);

    junction.x = -2;            junction.y =  (t.height-1)/2;
    t.junctionPositions.push_back(junction);

    junction.x = (t.width-1)/2; junction.y =  t.height+1;
    t.junctionPositions.push_back(junction);
}

MapTemplate* MapTemplateHandler::getTemplate(TemplateDevName_t templateDevName, bool generalTemplate)
{
    MapTemplate* te = NULL;

    if (generalTemplate == true) {
        for (unsigned int i=0; i < m_generalTemplates.size(); i++) {
            if (m_generalTemplates.at(i).devName == templateDevName) {
                te = &(m_generalTemplates.at(i));
                i = 9999;
            }
        }
    }
    else {
        for (unsigned int i=0; i < m_templates.size(); i++) {
            if (m_templates.at(i).devName == templateDevName) {
                te = &(m_templates.at(i));
                i = 9999;
            }
        }
    }

    if (te == NULL) {
      tracer << "[WARNING] Invalid maptemplate or wrong section (general or not?), in getTemplate()" << endl;
    }
    return te;
}


MapTemplate* MapTemplateHandler::getRandomTemplateForMapBuilder()
{
    const int siz = m_generalTemplates.size();
    //If no templates exists, pos = -1. If one template exist pos = 0. Else random.
    const int pos = siz == 0 ? -1 : (siz == 1 ? 0 : eng->dice(1,siz)-1);

    if (pos == -1) {
        tracer <<"[WARNING] No templates exists, in getRandomTemplateForMapbuilder()" << endl;
        return NULL;
    }

    return &(m_generalTemplates.at(pos));
}


MapTemplate MapTemplateHandler::stringToTemplate(
    string str, vector<CharToIdTranslation>* translations, TemplateDevName_t devName)
{
    MapTemplate             result(devName);
    unsigned int            strPos = 0;
    const unsigned int      strSize = str.size();
    vector< vector<Feature_t> > resultVector;
    vector<Feature_t> curInnerVec;

    while (strPos != strSize) {
        //If delim character found push inner vector to outer
        if (str[strPos] == ';') {
            resultVector.push_back(curInnerVec);
            curInnerVec.resize(0);
        }
        else {
            if (str[strPos] == '#' || str[strPos] == '_') {
                curInnerVec.push_back(feature_empty);
            }
            else {
                if( str[strPos] == '.') {
                    curInnerVec.push_back(feature_stoneFloor);
                }
                else {
                    if( translations != NULL ) {
                        curInnerVec.push_back(translate(str[strPos], translations));
                    }
                }
            }
        }
        strPos++;
    }

    result.featureVector.swap(resultVector);
    result.width  = result.featureVector.at(0).size();
    result.height = result.featureVector.size();

    return result;
}

//void MapTemplateHandler::initSize(int width, int height, MapTemplate &mapTemplate)
//{
//    //Outer vector is rows
//    mapTemplate.m.resize(width);
//
//    //Inner vector is columns
//    for (int x = 0; x < width; x++) {
//        mapTemplate.m[x].resize(height);
//
//        for (int y = 0; y < height; y++)
//            mapTemplate.m[x][y] = 0;
//    }
//}
