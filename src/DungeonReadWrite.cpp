//#include "DUNGEON_WRITE_READ.h"
//
//#include "Engine.h"
//
//#include "ConstRenderSettings.h"
//#include "ConstDungeonSettings.h"
//
//#include "Item.h"
//
//
////Number of levels in main dungeon. Does not include
////negative levels and forest.
//const int DUNGEON_LEVELS = 35;
//
//extern void profiler(bool init);
//
//extern int stairX, stairY;
//
//extern vector<MonsterSpawnData> monsterSpawnList;
//
////File for storing landscape and what's explored
//ofstream outfileDungeon     ( "save/dungeon",       ios::out | ios::binary );
//
//void DungeonWriterLoader::dungeon_clear()
//{
//    for (int xx = 0; xx < MAP_X_CELLS; xx++) {
//        for (int yy = 0; yy < MAP_Y_CELLS; yy++) {
//            if (eng->map->items[xx][yy] != NULL) {delete  eng->map->items[xx][yy];}
//            eng->map->items           [xx][yy] = NULL;
//            eng->map->blood           [xx][yy] = false;
//            eng->map->explored        [xx][yy] = false;
//            eng->map->playerVision    [xx][yy] = false;
//            *(eng->map->getLandscape) (xx, yy, " ") = *m_currentWallID;
//        }
//    }
//
//    eng->dungeonFeatureFactory->deleteAllDungeonFeatures();
//    eng->actorFactory->deleteAllMonsters();
//    monsterSpawnList.resize(0);
//}
//
//
//void DungeonWriterLoader::dungeon_write(const bool thisIsWorldGen)
//{
//    char            buffer[1];
//    buffer[0]           = 0;
//
//    //File for storing spawned items
//    ofstream outfileItems( "save/spawned_items", ios::out | ios::app );
//
//    //File for storing dungeon features
//    ofstream outfileDungeonFeatures( "save/dungeon_features", ios::out | ios::app);
//
//    //File for storing characters
//    ofstream outfileActors( "save/actors", ios::out | ios::app) ;
//
//
//    //Find correct place in file
//    //DUNGEON_START_NUMBER is a negative number
//    const int   dataTypesStored = 3;
//    const long  pos             = dataTypesStored * MAP_X_CELLS * MAP_Y_CELLS *
//        ( eng->map->getDungeonLevel() - DUNGEON_START_NUMBER);
//    outfileDungeon.seekp( pos );
//
//    //Landscape
//    for (int xx=0; xx < MAP_X_CELLS; xx++)
//        for (int yy=0; yy < MAP_Y_CELLS; yy++) {
//            buffer[0]   = *(eng->map->getLandscape(xx,yy, ""));
//            outfileDungeon.write(buffer, 1);
//        }
//
//    //Explored
//    for (int xx=0; xx < MAP_X_CELLS; xx++) {
//        for (int yy=0; yy < MAP_Y_CELLS; yy++) {
//            if (eng->map->explored[xx][yy] == false) buffer[0]=0; else buffer[0]=1;
//            outfileDungeon.write(buffer,1);
//        }
//    }
//
//    //Player visual memory
//    for (int xx=0; xx < MAP_X_CELLS; xx++)
//        for (int yy=0; yy < MAP_Y_CELLS; yy++) {
//            buffer[0]   = eng->map->playerVisualMemory[xx][yy].glyph;
//            outfileDungeon.write(buffer, 1);
//        }
//
//    int devName, use;
//    Item* item = NULL;
//    for (int xx=0; xx < MAP_X_CELLS; xx++) {
//        for (int yy=0; yy < MAP_Y_CELLS; yy++) {
//            item    = eng->map->items[xx][yy];
//            if( item != NULL ) {
//                if (item->m_removeAtLevelChange == true) {
//                    delete item;
//                    eng->map->items[xx][yy] = NULL;
//                }
//                else {
//                    devName = item->m_devName;
//
//                    outfileItems << intToString(eng->map->getDungeonLevel()) << endl;
//                    outfileItems << intToString(devName) << endl;
//                    outfileItems << intToString(xx) << endl;
//                    outfileItems << intToString(yy) << endl;
//
//                    vector<string> extra = item->itemSpecificWriteToFile();
//                    for (unsigned int k = 0; k < extra.size(); k++) {
//                        outfileItems << extra.at(k) << endl;
//                    }
//                    outfileItems << "S" << endl;
//
//                    for (unsigned int k = 0; k < item->uses.size(); k++) {
//                        use     = item->uses.at(k);
//                        outfileItems << intToString(use) << endl;
//                    }
//                    outfileItems << "E" << endl;
//                }
//            }
//        }
//    }
//    outfileItems .close();
//
//    //----------------------------------------------------------- WRITE DUNGEON FEATURES TO FILE
//    vector<string> lines = eng->dungeonFeatureFactory->writeAllDungeonFeaturesToFile();
//    for (unsigned int i = 0; i < lines.size(); i++) {
//        outfileDungeonFeatures << lines.at(i) << endl;
//    }
//    outfileDungeonFeatures.close();
//
//    //----------------------------------------------------------- WRITE ACTORS TO FILE
//    lines.resize(0);
//    if (thisIsWorldGen == true)
//        eng->actorFactory->writeAllActorsToFileFromLightStructs(&lines);
//    else
//        eng->actorFactory->writeAllActorsToFile(&lines);
//
//    for (unsigned int i = 0; i < lines.size(); i++) {
//        outfileActors << lines.at(i) << endl;
//    }
//
//    outfileActors.close();
//
//    //profiler(false);
//}
//
//void DungeonWriterLoader::dungeon_create_and_write_world()
//{
////    drawClearArea(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
////    SDL_Flip(screen);
//
//    remove ("save/dungeon");
//    remove ("save/spawned_items");
//    remove ("save/dungeon_features");
//    remove ("save/actors");
//
//    eng->map->resetDungeonLevel();
//
////    *m_currentWallID  = eng->landscapeData->getLandscapeID(landscapeDevName_stoneWall);
////    *m_currentFloorID = eng->landscapeData->getLandscapeID(landscapeDevName_stoneFloor);
////    dungeon_clear();
//
//    //The crypt you wake up in
////    cout << "Creating and writing start area 1..." << endl;
////    eng->mapBuild->buildStartArea1();
////    dungeon_write(true);
////    ++*m_dungeonLevel;
//
//    //The small catacomb upstairs from your crypt
////    cout << "Creating and writing start area 2..." << endl;
////    *m_currentWallID  = eng->landscapeData->getLandscapeID(landscapeDevName_stoneWall);
////    *m_currentFloorID = eng->landscapeData->getLandscapeID(landscapeDevName_stoneFloor);
////    dungeon_clear();
////    eng->mapBuild->buildDungeonLevel();
////    dungeon_write(true);
////    ++*m_dungeonLevel;
//
//    //Forest
////    cout << "Creating and writing Forest..." << endl;
//    *m_currentWallID    = landscapeDevName_tree;
//    *m_currentFloorID   = landscapeDevName_grass;
//    dungeon_clear();
//    cout << "Building forest..." << endl;
//    eng->mapBuild->buildForest();
//    cout << "Building forest [DONE]" << endl;
//    dungeon_write(true);
//    dungeon_clear();
//    eng->map->incrDungeonLevel();
//
//    bool inCaverns   = false;
//    bool cavernsDone = false;
//    //The main dungeon
//    for (int z = 0; z < DUNGEON_LEVELS; z++) {
//
//        if (inCaverns == false && cavernsDone == false) {
//            if (eng->map->getDungeonLevel() > FIRST_CAVERN_LEVEL_MIN)
//                if (eng->basicUtils->dice(1,100) > 50) {
//                    inCaverns   = true;
//                }
//
//            if (eng->map->getDungeonLevel() >= FIRST_CAVERN_LEVEL_MAX) {
//                inCaverns   = true;
//            }
//        }
//
//        if (inCaverns == true) {
//            if (eng->map->getDungeonLevel() > LAST_CAVERN_LEVEL_MIN)
//                if (eng->basicUtils->dice(1,100) > 50) {
//                    inCaverns   = false;
//                    cavernsDone = true;
//                }
//
//            if (eng->map->getDungeonLevel() >= LAST_CAVERN_LEVEL_MAX) {
//                inCaverns   = false;
//                cavernsDone = true;
//            }
//        }
//
//        if (inCaverns == true) {
//            *m_currentWallID  = landscapeDevName_caveWall;
//            *m_currentFloorID = landscapeDevName_caveFloor;
//            dungeon_clear();
////            cout << "Building cave..." << endl;
//            eng->mapBuild->buildCavern();
////            cout << "Building cave [DONE]" << endl;
//        }
//        else {
//            *m_currentFloorID = landscapeDevName_stoneFloor;
//
//            if ( eng->basicUtils->dice(1,100) < 20 && eng->map->getDungeonLevel() > 0)
//                *m_currentWallID = landscapeDevName_stoneWallGreen;
//            else
//                *m_currentWallID = landscapeDevName_stoneWall;
//
//            dungeon_clear();
////            cout << "Building dungeon level..." << endl;
//            eng->mapBuild->buildDungeonLevel();
////            cout << "Building dungeon level [DONE]" << endl;
//        }
//
////        cout << "Writing level..." << endl;
//        dungeon_write(true);
////        cout << "Writing level [DONE]" << endl;
//
//        dungeon_clear();
//
//        eng->map->incrDungeonLevel();
//    }
//
//    eng->map->resetDungeonLevel();
//}
//
//
//struct Line{
//    char t[255];
//};
//
//
//void DungeonWriterLoader::dungeon_read()
//{
//    ifstream infileDungeon          ("save/dungeon");
//    infileDungeon           .seekg(0, ios_base::beg);
//
//    ifstream infileItems            ("save/spawned_items");
//    infileItems      .seekg(0, ios_base::beg);
//
//    ifstream infileDungeonFeatures  ("save/dungeon_features");
//    infileDungeonFeatures   .seekg(0, ios_base::beg);
//
//    ifstream infileActors           ("save/actors");
//    infileActors            .seekg(0, ios_base::beg);
//
//    char buffer[1];
//    dungeon_clear();
//
//    //Skip dungeon levels until current
//    unsigned int DLVL = eng->map->getDungeonLevel();
//    for(unsigned int i = DUNGEON_START_NUMBER; i < DLVL; i++)
//        for(int xx=0; xx < MAP_X_CELLS; xx++)
//            for(int yy=0; yy < MAP_Y_CELLS; yy++) {
//                //Skip landscape
//                infileDungeon.read(buffer,1);
//                //Skip explored
//                infileDungeon.read(buffer,1);
//                //Skip visual memory
//                infileDungeon.read(buffer,1);
//            }
//
//    //landscape
//    for(int xx=0; xx < MAP_X_CELLS; xx++)
//        for(int yy=0; yy < MAP_Y_CELLS; yy++)
//        {
//            infileDungeon.read(buffer,1);
////            cout << buffer[0] << ", ";
//            *(eng->map->getLandscape(xx,yy, " ")) = static_cast<LandscapeDevName_t>(buffer[0]);
//        }
//
//    //explored
//    for(int xx=0; xx < MAP_X_CELLS; xx++)
//        for(int yy=0; yy < MAP_Y_CELLS; yy++)
//        {
//            infileDungeon.read(buffer,1);
//            eng->map->explored[xx][yy] = buffer[0] != 0;
//        }
//
//    //Player visual memory
//    for(int xx=0; xx < MAP_X_CELLS; xx++)
//        for(int yy=0; yy < MAP_Y_CELLS; yy++)
//        {
//            infileDungeon.read(buffer,1);
////            cout << buffer[0] << ", ";
//            eng->map->playerVisualMemory[xx][yy].glyph = buffer[0];
//            eng->map->playerVisualMemory[xx][yy].color = clrOutsideFovDefault;
//        }
//
//
//    infileDungeon       .close();
//
////    cout << "Reading items from file..." << endl;
//    //---------------------------------------------- READ ITEMS FROM FILE
//    Item* item = NULL;
//    ItemUses_t itemUse;
//    int iDungeonLevel   = 0;
//    int devName         = 0;
//    int iX              = 0;
//    int iY              = 0;
//    bool _done          = false;
//
//    //Every line that is NOT an item on the current level
//    //is stored in this vector. The spawned item-file is
//    //deleted, and then recreated from this vector.
//    //Result: Lines corresponding to current level are removed.
//
//    Line            line;
//    vector<Line>    textBackup;
//    textBackup      .resize(0);
//
//    char bufferItems[255];
//    for (int ii = 0; ii < 255; ii++) {
//        bufferItems[ii] = 0;
//        line.t[ii]      = ' ';
//    }
//
//    while(infileItems.getline(bufferItems,255)) {
//        item = NULL;
//
//        iDungeonLevel = atoi(bufferItems);
//
////        cout    << "Dungeon Level        : " << *m_dungeonLevel              << endl
////                << "Item found on        : " <<  iDungeonLevel               << endl;
//
//        //Item found on current level
//        if( iDungeonLevel == eng->map->getDungeonLevel()) {
//
//            infileItems.getline(bufferItems,255);
//            devName     = atoi(bufferItems);
//
//            item        = eng->itemFactory->spawnItemFromFile(static_cast<ItemDevNames_t>(devName));
//
//            infileItems.getline(bufferItems,255);
//            iX          = atoi(bufferItems);
//            infileItems.getline(bufferItems,255);
//            iY          = atoi(bufferItems);
//
//            //Read item specific lines
//            vector<string> extra;
//            while(bufferItems[0] != 'S') {
//                infileItems.getline(bufferItems,255);
//                if (bufferItems[0] != 'S') {
//                    extra.push_back(bufferItems);
//                    item->itemSpecificReadFromFile(extra);
//                }
//            }
//
//            //Read item uses
//            while(bufferItems[0] != 'E') {
//                infileItems.getline(bufferItems,255);
//                if (bufferItems[0] != 'E') {
//                    itemUse         = static_cast<ItemUses_t> (atoi(bufferItems));
//                    item->uses.push_back(itemUse);
//                }
//            }
//        }
//        //If item found on wrong level, skip to next item
//        else {
//            for (int z = 0; z < 10; z++) {
//                if (bufferItems[z] == ' ') {
//                    z = 99999;
//                }
//                else {
//                    line.t[z] = bufferItems[z];
//                }
//            }
//            textBackup.push_back(line);
//            _done = false;
//            while(_done == false) {
//                infileItems.getline(bufferItems,255);
//                for (int z = 0; z < 10; z++) {
//                    if (bufferItems[z] == ' ') {
//                        z = 99999;
//                    }
//                    else {
//                        line.t[z] = bufferItems[z];
//                    }
//                }
//                textBackup.push_back(line);
//                if (bufferItems[0] == 'E')
//                    _done = true;
//            }
//        }
//        //If an item was read on current dungeon level,
//        //add it on the item map.
//        if (item != NULL) {
//            eng->map->items[iX][iY] = item;
//        }
//
//        for (int ii = 0; ii < 255; ii++) {
//            bufferItems[ii] = ' ';
//            line.t[ii]      = ' ';
//        }
//    }
//
//    //Delete and recreate spawned item-file.
//    infileItems  .close();
////    remove ("save/spawned_items");
//    ofstream outfileItems ("save/spawned_items", ios::trunc);
//
//    for (unsigned int i = 0; i < textBackup.size(); i++) {
//        for (unsigned int ii = 0; ii < 10; ii++) {
//            if ( (textBackup[i].t[ii] >= '0' &&  textBackup[i].t[ii] <= '9') || textBackup[i].t[ii] == '-' || textBackup[i].t[ii] == 'E' || textBackup[i].t[ii] == 'S') {
//                outfileItems << textBackup[i].t[ii];
//            }
//        }
//        outfileItems << endl;
//    }
//
//    outfileItems.close();
//    //----------------------------------------------
////    cout << "Reading items from file [DONE]" << endl;
//
//
////    cout << "Reading dungeon features from file..." << endl;
//    //---------------------------------------------- READ DUNGEON FEATURES FROM FILE
//
////    infileDungeonFeatures.seekg (0, ios::end);
////    cout << "Length of dungeon feature file before reading: " << infileDungeonFeatures.tellg() << endl;
////    infileDungeonFeatures.seekg (0, ios::beg);
//
//    char bufferFeatures[255];
//    for (int ii = 0; ii < 255; ii++) {
//        bufferFeatures[ii]  = 0;
//        line.t[ii]          = ' ';
//    }
//
//    int featureDungeonLevel = 0;
//    _done                   = false;
//
//    //Every line that is NOT a feature on the current level
//    //is stored in this vector. The feature-file is
//    //deleted, and then recreated from this vector.
//    //Result: Lines corresponding to current level are removed.
//
//    textBackup      .resize(0);
//
//    vector<string> featureLines;
//    featureLines.resize(0);
//
//    while(infileDungeonFeatures.getline(bufferFeatures,255)) {
//        featureDungeonLevel = atoi(bufferFeatures);
//
//        //Feature found on current level
//        if( featureDungeonLevel == eng->map->getDungeonLevel()) {
//            featureLines.resize(0);
//            while (bufferFeatures[0] != 'E') {
//                infileDungeonFeatures.getline(bufferFeatures, 255);
//
//                if (bufferFeatures[0] != 'E') {
//                    featureLines.push_back(bufferFeatures);
//                }
//            }
//            eng->dungeonFeatureFactory->spawnDungeonFeatureFromFile( featureLines );
//        }
//        //If feature found on wrong level, skip to next feature
//        else {
//            for (int z = 0; z < 10; z++) {
//                if (bufferFeatures[z] == ' ') {
//                    z = 99999;
//                }
//                else {
//                    line.t[z] = bufferFeatures[z];
//                }
//            }
//            textBackup.push_back(line);
//            _done = false;
//            while(_done == false) {
//                infileDungeonFeatures.getline(bufferFeatures,255);
//                for (int z = 0; z < 10; z++) {
//                    if (bufferFeatures[z] == ' ') {
//                        z = 99999;
//                    }
//                    else {
//                        line.t[z] = bufferFeatures[z];
//                    }
//                }
//                textBackup.push_back(line);
//                if (bufferFeatures[0] == 'E')
//                    _done = true;
//            }
//        }
//
//        for (int ii = 0; ii < 255; ii++) {
//            bufferFeatures[ii] = ' ';
//            line.t[ii]         = ' ';
//        }
//    }
//
//    //Delete and recreate feature file.
//    infileDungeonFeatures  .close();
////    remove ("save/dungeon_features");
//    ofstream outfileDungeonFeatures ("save/dungeon_features", ios::trunc);
//
//    for (unsigned int i = 0; i < textBackup.size(); i++) {
//        for (unsigned int ii = 0; ii < 10; ii++) {
//            if ( (textBackup[i].t[ii] >= '0' &&  textBackup[i].t[ii] <= '9') || textBackup[i].t[ii] == '-' || textBackup[i].t[ii] == 'E') {
//                outfileDungeonFeatures << textBackup[i].t[ii];
//            }
//        }
//        outfileDungeonFeatures << endl;
//    }
//
////    outfileDungeonFeatures.seekp (0, ios::end);
////    cout << "Length of dungeon feature file after reading: " << outfileDungeonFeatures.tellp() << endl;
//    outfileDungeonFeatures.close();
//    //----------------------------------------------
////    cout << "Reading dungeon features from file [DONE]" << endl;
//
////    cout << "Reading actors from file..." << endl;
//    //---------------------------------------------- READ ACTORS FROM FILE
//
////    infileActors.seekg (0, ios::end);
////    cout << "Length of actor file before reading: " << infileActors.tellg() << endl;
////    infileActors.seekg (0, ios::beg);
//
//
//    char bufferActors[255];
//    for (int ii = 0; ii < 255; ii++) {
//        bufferActors[ii]    = 0;
//    }
//
//    int actorDungeonLevel   = 0;
//    _done                   = false;
//
//    //Every line that is NOT an actor on the current level
//    //is stored in this vector. The actor-file is
//    //deleted, and then recreated from this vector.
//    //Result: Lines corresponding to current level are removed.
//
//    textBackup      .resize(0);
//
//    vector<string> actorLines;
//    actorLines.resize(0);
//
//    while(infileActors.getline(bufferActors,255)) {
//        actorDungeonLevel = atoi(bufferActors);
//        //Actor found on current level
//        if( actorDungeonLevel == eng->map->getDungeonLevel()) {
//            actorLines.resize(0);
//            while (bufferActors[0] != 'E') {
//                infileActors.getline(bufferActors, 255);
//
//                if (bufferActors[0] != 'E') {
//                    actorLines.push_back(bufferActors);
//                }
//            }
////            cout << "spawning actor..." << endl;
//            eng->actorFactory->spawnActorFromFile( actorLines );
////            cout << "spawning actor [DONE]" << endl;
//        }
//        //If actor found on wrong level, skip to next actor
//        else {
//            for (int z = 0; z < 10; z++) {
//                if (bufferActors[z] == ' ') {
//                    z = 99999;
//                }
//                else {
//                    line.t[z] = bufferActors[z];
//                }
//            }
//            textBackup.push_back(line);
//            _done = false;
//            while(_done == false) {
//                infileActors.getline(bufferActors,255);
//                for (int z = 0; z < 10; z++) {
//                    if (bufferActors[z] == ' ') {
//                        z = 99999;
//                    }
//                    else {
//                        line.t[z] = bufferActors[z];
//                    }
//                }
//                textBackup.push_back(line);
//                if (bufferActors[0] == 'E') {
//                    _done = true;
//                }
//            }
//        }
//
//        for (int ii = 0; ii < 255; ii++) {
//            bufferActors[ii] = ' ';
//            line.t[ii]       = ' ';
//        }
//    }
//
//    //Delete and recreate actor file.
//    infileActors.close();
//    ofstream outfileActors ("save/actors", ios::trunc);
//
//    for (unsigned int i = 0; i < textBackup.size(); i++) {
//        for (unsigned int ii = 0; ii < 10; ii++) {
//            if ( (textBackup[i].t[ii] >= '0' &&  textBackup[i].t[ii] <= '9') || textBackup[i].t[ii] == '-' || textBackup[i].t[ii] == 'E') {
//                outfileActors << textBackup[i].t[ii];
//            }
//            else {
//                ii = 99999;
//            }
//        }
//        outfileActors << endl;
//    }
//
////    cout << "Nr of lines in text backup after reading: " << textBackup.size() << endl;
////    outfileActors.seekp (0, ios::end);
////    cout << "Length of actor file after reading: " << outfileActors.tellp() << endl;
//    outfileActors.close();
//    //----------------------------------------------
////    cout << "Reading actors from file [DONE]" << endl;
//}
//
//
