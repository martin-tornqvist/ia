#include "CharacterDescr.h"

#include <algorithm>

#include "PlayerBon.h"
#include "ActorPlayer.h"
#include "Render.h"
#include "TextFormatting.h"
#include "Input.h"
#include "ItemPotion.h"
#include "ItemScroll.h"
#include "ItemFactory.h"
#include "Item.h"
#include "Map.h"

using namespace std;

namespace CharacterDescr
{

namespace
{

vector<StrAndClr> lines_;

void mkLines()
{
    lines_.clear();

    const string offset = "   ";
    const Clr& clrHeading   = clrWhiteHigh;
    const Clr& clrText      = clrWhite;
    const Clr& clrTextDark  = clrGray;

    lines_.push_back({Map::player->getNameThe(), clrMenuHighlight});
    lines_.push_back({" ", clrText});

    const AbilityVals& abilities = Map::player->getData().abilityVals;

    lines_.push_back({"Combat skills", clrHeading});

    const int BASE_MELEE          = min(100, abilities.getVal(AbilityId::melee,
                                        true, *(Map::player)));

    const int BASE_RANGED         = min(100, abilities.getVal(AbilityId::ranged,
                                        true, *(Map::player)));

    const int BASE_DODGE_ATTACKS  = min(100, abilities.getVal(AbilityId::dodgeAtt,
                                        true, *(Map::player)));

    lines_.push_back({offset + "Melee    : " + toStr(BASE_MELEE)         + "%", clrText});
    lines_.push_back({offset + "Ranged   : " + toStr(BASE_RANGED)        + "%", clrText});
    lines_.push_back({offset + "Dodging  : " + toStr(BASE_DODGE_ATTACKS) + "%", clrText});

    lines_.push_back({" ", clrText});

    lines_.push_back({"Mental conditions", clrHeading});
    const int NR_LINES_BEFORE_MENTAL = lines_.size();

    const auto& phobias = Map::player->phobias;

    if (phobias[int(Phobia::dog)])
        lines_.push_back({offset + "Phobia of dogs",            clrText});

    if (phobias[int(Phobia::rat)])
        lines_.push_back({offset + "Phobia of rats",            clrText});

    if (phobias[int(Phobia::spider)])
        lines_.push_back({offset + "Phobia of spiders",         clrText});

    if (phobias[int(Phobia::undead)])
        lines_.push_back({offset + "Phobia of the dead",        clrText});

    if (phobias[int(Phobia::crampedPlace)])
        lines_.push_back({offset + "Phobia of cramped spaces",  clrText});

    if (phobias[int(Phobia::openPlace)])
        lines_.push_back({offset + "Phobia of open places",     clrText});

    if (phobias[int(Phobia::deepPlaces)])
        lines_.push_back({offset + "Phobia of deep places",     clrText});

    if (phobias[int(Phobia::dark)])
        lines_.push_back({offset + "Phobia of darkness",        clrText});

    if (Map::player->obsessions[int(Obsession::masochism)])
        lines_.push_back({offset + "Masochistic obsession",     clrText});

    if (Map::player->obsessions[int(Obsession::sadism)])
        lines_.push_back({offset + "Sadistic obsession",        clrText});

    const int NR_LINES_AFTER_MENTAL = lines_.size();

    if (NR_LINES_BEFORE_MENTAL == NR_LINES_AFTER_MENTAL)
    {
        lines_.push_back({offset + "No special symptoms", clrText});
    }

    lines_.push_back({" ", clrText});

    lines_.push_back({"Potion knowledge", clrHeading});
    vector<StrAndClr> potionList;
    vector<StrAndClr> manuscriptList;

    for (int i = 0; i < int(ItemId::END); ++i)
    {
        const ItemDataT* const d = ItemData::data[i];

        if (d->type == ItemType::potion && (d->isTried || d->isIdentified))
        {
            Item* item = ItemFactory::mk(d->id);
            potionList.push_back({offset + item->getName(ItemRefType::plain), d->clr});
            delete item;
        }
        else
        {
            if (d->type == ItemType::scroll && (d->isTried || d->isIdentified))
            {
                Item* item = ItemFactory::mk(d->id);
                manuscriptList.push_back(StrAndClr(offset + item->getName(ItemRefType::plain),
                                                   item->getInterfaceClr()));
                delete item;
            }
        }
    }

    auto strAndClrSort = [](const StrAndClr & e1, const StrAndClr & e2)
    {
        return e1.str < e2.str;
    };

    if (potionList.empty())
    {
        lines_.push_back({offset + "No known potions", clrText});
    }
    else
    {
        sort(potionList.begin(), potionList.end(), strAndClrSort);
        for (StrAndClr& e : potionList) {lines_.push_back(e);}
    }
    lines_.push_back({" ", clrText});


    lines_.push_back({"Manuscript knowledge", clrHeading});
    if (manuscriptList.size() == 0)
    {
        lines_.push_back({offset + "No known manuscripts", clrText});
    }
    else
    {
        sort(manuscriptList.begin(), manuscriptList.end(), strAndClrSort);
        for (StrAndClr& e : manuscriptList) {lines_.push_back(e);}
    }
    lines_.push_back({" ", clrText});

    lines_.push_back({"Traits gained", clrHeading});

    const int MAX_W_DESCR = (MAP_W * 2) / 3;

    for (int i = 0; i < int(Trait::END); ++i)
    {
        if (PlayerBon::traitsPicked[i])
        {
            const Trait trait = Trait(i);
            string title = "", descr = "";
            PlayerBon::getTraitTitle(trait, title);
            PlayerBon::getTraitDescr(trait, descr);
            lines_.push_back({offset + title, clrText});
            vector<string> descrLines;
            TextFormatting::lineToLines(descr, MAX_W_DESCR, descrLines);
            for (string& descrLine : descrLines)
            {
                lines_.push_back({offset + descrLine, clrTextDark});
            }
            lines_.push_back({" ", clrText});
        }
    }
}

void getShockResSrcTitle(const ShockSrc shockSrc, string& strRef)
{
    strRef = "";
    switch (shockSrc)
    {
    case ShockSrc::time:            strRef = "Time";                    break;
    case ShockSrc::castIntrSpell:   strRef = "Casting learned spells";  break;
    case ShockSrc::seeMon:          strRef = "Seeing monsters";         break;
    case ShockSrc::useStrangeItem:  strRef = "Using strange items";     break;
    case ShockSrc::misc:            strRef = "Other";                   break;
    case ShockSrc::END: {} break;
    }
}

void drawInterface()
{
    const string decorationLine(MAP_W, '-');

    const int X_LABEL = 3;

    Render::drawText(decorationLine, Panel::screen, Pos(0, 0), clrGray);

    Render::drawText(" Displaying character description ", Panel::screen,
                     Pos(X_LABEL, 0), clrWhite);

    Render::drawText(decorationLine, Panel::screen, Pos(0, SCREEN_H - 1),
                     clrGray);

    Render::drawText(infoScrCmdInfo, Panel::screen, Pos(X_LABEL, SCREEN_H - 1), clrWhite);
}

} //namespace

void run()
{
    mkLines();

    const int LINE_JUMP           = 3;
    const int NR_LINES_TOT        = lines_.size();
    const int MAX_NR_LINES_ON_SCR = SCREEN_H - 2;

    int topNr = 0;
    int btmNr = min(topNr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);

    while (true)
    {
        Render::clearScreen();
        drawInterface();
        int yPos = 1;
        for (int i = topNr; i <= btmNr; ++i)
        {
            const StrAndClr& line = lines_[i];
            Render::drawText(line.str , Panel::screen, Pos(0, yPos++), line.clr);
        }
        Render::updateScreen();

        const KeyData& d = Input::getInput();

        if (d.key == '2' || d.sdlKey == SDLK_DOWN || d.key == 'j')
        {
            topNr += LINE_JUMP;
            if (NR_LINES_TOT <= MAX_NR_LINES_ON_SCR)
            {
                topNr = 0;
            }
            else
            {
                topNr = min(NR_LINES_TOT - MAX_NR_LINES_ON_SCR, topNr);
            }
        }
        else if (d.key == '8' || d.sdlKey == SDLK_UP || d.key == 'k')
        {
            topNr = max(0, topNr - LINE_JUMP);
        }
        else if (d.sdlKey == SDLK_SPACE || d.sdlKey == SDLK_ESCAPE)
        {
            break;
        }
        btmNr = min(topNr + MAX_NR_LINES_ON_SCR - 1, NR_LINES_TOT - 1);
    }
    Render::drawMapAndInterface();
}

} //CharacterDescr
