#include "MainMenu.h"

#include <string>

#include "Engine.h"
#include "Colors.h"
#include "Render.h"
#include "MenuInputHandler.h"
#include "SaveHandler.h"
#include "Highscore.h"
#include "Manual.h"
#include "Popup.h"
#include "TextFormatting.h"
#include "Credits.h"

using namespace std;

void MainMenu::draw(const MenuBrowser& browser) {
  trace << "MainMenu::draw()..." << endl;

  vector<string> logo;

  if(eng->config->isTilesMode == false) {
    logo.push_back("        ___  __                __  __                  ");
    logo.push_back("| |\\  | |   |  )  /\\      /\\  |  )/    /\\  |\\  |  /\\   ");
    logo.push_back("+ | \\ | +-- +--  ____    ____ +-- -   ____ | \\ | ____  ");
    logo.push_back("| |  \\| |   | \\ /    \\  /    \\| \\ \\__/    \\|  \\|/    \\ ");
    logo.push_back("               \\                 \\                      ");
  }

  Pos pos(MAP_X_CELLS / 2, 3);

  trace << "MainMenu: Calling clearWindow()" << endl;
  eng->renderer->clearScreen();

  trace << "MainMenu: Drawing random background letters" << endl;
  const int NR_X_CELLS = eng->config->screenWidth / eng->config->cellW;
  const int NR_Y_CELLS = eng->config->screenHeight / eng->config->cellH;
  const int BG_BRIGHTNESS = eng->dice.range(14, 17);
  for(int y = 0; y < NR_Y_CELLS; y++) {
    for(int x = 0; x < NR_X_CELLS; x++) {
      char cha = ' ';
      if(eng->dice.coinToss()) {
        cha = 'a' + eng->dice.range(0, 25);
      }
      SDL_Color bgClr = clrBlack;
      bgClr.r = BG_BRIGHTNESS / 2;
      bgClr.g = BG_BRIGHTNESS / 2;
      bgClr.b = BG_BRIGHTNESS;
      eng->renderer->drawGlyph(cha, panel_screen, Pos(x, y), bgClr);
    }
  }

  trace << "MainMenu: Drawing HPL quote" << endl;
  const int QUOTE_BRIGHTNESS = BG_BRIGHTNESS + 7;
  SDL_Color quoteClr = clrBlack;
  quoteClr.r = QUOTE_BRIGHTNESS / 2;
  quoteClr.g = QUOTE_BRIGHTNESS / 2;
  quoteClr.b = QUOTE_BRIGHTNESS;
  vector<string> quoteLines =
    eng->textFormatting->lineToLines(getHplQuote(), 45);
  const int Y0_LOGO = eng->config->isTilesMode ? 17 : 15;
  for(unsigned int i = 0; i < quoteLines.size(); i++) {
    eng->renderer->drawText(quoteLines.at(i), panel_screen,
                            Pos(7, Y0_LOGO + i), quoteClr);
  }

  if(eng->config->isTilesMode) {
    trace << "MainMenu: Calling drawMainMenuLogo()" << endl;
    eng->renderer->drawMainMenuLogo(4);
    pos.y += 10;
  } else {
    const int LOGO_X_POS_LEFT = (MAP_X_CELLS - logo.at(0).size()) / 2;
    for(unsigned int i = 0; i < logo.size(); i++) {
      pos.x = LOGO_X_POS_LEFT;
      for(unsigned int ii = 0; ii < logo.at(i).size(); ii++) {
        if(logo.at(i).at(ii) != ' ') {
          SDL_Color clr = clrRed;
          clr.r += eng->dice.range(-60, 100);
          clr.r = max(0, min(254, int(clr.r)));
          eng->renderer->drawGlyph(logo.at(i).at(ii), panel_screen,
                                   pos, clr);
        }
        pos.x++;
      }
      pos.y += 1;
    }
    pos.y += 3;
  }

  pos.x = 48;

  if(IS_DEBUG_MODE) {
    eng->renderer->drawText("## DEBUG MODE ##", panel_screen, Pos(1, 1),
                            clrYellow);
  }

  SDL_Color clrActive   = clrNosferatuSepiaLgt;
  SDL_Color clrInactive = clrNosferatuSepiaDrk;

  eng->renderer->drawText("New journey", panel_screen, pos,
                          browser.isPosAtKey('a') ? clrActive : clrInactive);
  pos.y += 1;
  pos.x += 1;

  eng->renderer->drawText("Resurrect", panel_screen, pos,
                          browser.isPosAtKey('b') ? clrActive : clrInactive);
  pos.y += 1;
  pos.x += 1;

  eng->renderer->drawText("Manual", panel_screen, pos,
                          browser.isPosAtKey('c') ? clrActive : clrInactive);
  pos.y += 1;
  pos.x += 1;

  eng->renderer->drawText("Options", panel_screen, pos,
                          browser.isPosAtKey('d') ? clrActive : clrInactive);
  pos.y += 1;
  pos.x += 1;

  eng->renderer->drawText("Credits", panel_screen, pos,
                          browser.isPosAtKey('e') ? clrActive : clrInactive);
  pos.y += 1;
  pos.x += 1;

  eng->renderer->drawText("High scores", panel_screen, pos,
                          browser.isPosAtKey('f') ? clrActive : clrInactive);
  pos.y += 1;
  pos.x += 1;

  eng->renderer->drawText("Escape to reality", panel_screen, pos,
                          browser.isPosAtKey('g') ? clrActive : clrInactive);
  pos.y += 1;
  pos.x += 1;

  if(IS_DEBUG_MODE) {
    eng->renderer->drawText("DEBUG: RUN BOT", panel_screen, pos,
                            browser.isPosAtKey('h') ? clrActive : clrInactive);
    pos.y += 1;
  }

  pos.x = MAP_X_CELLS / 2;

  eng->renderer->drawTextCentered(
    eng->config->GAME_VERSION + "  (c) 2011-2013 Martin Tornqvist",
    panel_character, Pos(pos.x, 1), clrWhite);

//  eng->renderer->drawTileInScreen(tile_playerMelee, 0, 0, clrRed, true, clrBlue);

  eng->renderer->updateScreen();

  trace << "MainMenu::draw() [DONE]" << endl;
}

GameEntry_t MainMenu::run(bool& quit) {
  trace << "MainMenu::run()" << endl;

  MenuBrowser browser(IS_DEBUG_MODE ? 8 : 7, 0);

  const bool IS_SAVE_AVAILABLE = eng->saveHandler->isSaveAvailable();

  draw(browser);

  bool proceed = false;
  while(proceed == false) {
    const MenuAction_t action = eng->menuInputHandler->getAction(browser);
    switch(action) {
      case menuAction_browsed: {
        draw(browser);
      }
      break;

      case menuAction_canceled: {
      }
      break;

      case menuAction_selected: {
        if(browser.isPosAtKey('a')) {
          proceed = true;
          return gameEntry_new;
        }
        if(browser.isPosAtKey('b')) {
          if(IS_SAVE_AVAILABLE) {
            eng->saveHandler->load();
            proceed = true;
            return gameEntry_load;
          } else {
            eng->popup->showMessage(
              "No save available. Starting a new character instead.", false);
            proceed = true;
            return gameEntry_new;
          }
        }
        if(browser.isPosAtKey('c')) {
          eng->manual->run();
          draw(browser);
        }
        if(browser.isPosAtKey('d')) {
          eng->config->runOptionsMenu();
          draw(browser);
        }
        if(browser.isPosAtKey('e')) {
          eng->credits->run();
          draw(browser);
        }
        if(browser.isPosAtKey('f')) {
          eng->highScore->runHighScoreScreen();
          draw(browser);
        }
        if(browser.isPosAtKey('g')) {
          proceed = true;
          quit = true;
        }
        if(IS_DEBUG_MODE) {
          if(browser.isPosAtKey('h')) {
            proceed = true;
            eng->config->isBotPlaying = true;
          }
        }
      }
      break;

      default: {
      }
      break;

    }
  }
  return gameEntry_new;
}

string MainMenu::getHplQuote() {
  vector<string> quotes;
  quotes.resize(0);
  quotes.push_back("Happy is the tomb where no wizard hath lain and happy the town at night whose wizards are all ashes.");
  quotes.push_back("Our means of receiving impressions are absurdly few, and our notions of surrounding objects infinitely narrow. We see things only as we are constructed to see them, and can gain no idea of their absolute nature. With five feeble senses we pretend to comprehend the boundlessly complex cosmos...");
  quotes.push_back("Disintegration is quite painless, I assure you.");
  quotes.push_back("I am writing this under an appreciable mental strain, since by tonight I shall be no more...");
  quotes.push_back("The end is near. I hear a noise at the door, as of some immense slippery body lumbering against it. It shall not find me...");
  quotes.push_back("Sometimes I believe that this less material life is our truer life, and that our vain presence on the terraqueous globe is itself the secondary or merely virtual phenomenon.");
  quotes.push_back("Life is a hideous thing, and from the background behind what we know of it peer daemoniacal hints of truth which make it sometimes a thousandfold more hideous.");
  quotes.push_back("Science, already oppressive with its shocking revelations, will perhaps be the ultimate exterminator of our human species, if separate species we be, for its reserve of unguessed horrors could never be borne by mortal brains if loosed upon the world....");
  quotes.push_back("Madness rides the star-wind... claws and teeth sharpened on centuries of corpses... dripping death astride a bacchanale of bats from nigh-black ruins of buried temples of Belial...");
  quotes.push_back("Memories and possibilities are ever more hideous than realities.");
  quotes.push_back("Yog-Sothoth knows the gate. Yog-Sothoth is the gate. Yog-Sothoth is the key and guardian of the gate. Past, present, future, all are one in Yog-Sothoth. He knows where the Old Ones broke through of old, and where They shall break through again. He knows where They have trod earth's fields, and where They still tread them, and why no one can behold Them as They tread.");
  quotes.push_back("Slowly but inexorably crawling upon my consciousness and rising above every other impression, came a dizzying fear of the unknown; a fear all the greater because I could not analyse it, and seeming to concern a stealthily approaching menace; not death, but some nameless, unheard-of thing inexpressibly more ghastly and abhorrent.");
  quotes.push_back("I felt that some horrible scene or object lurked beyond the silk-hung walls, and shrank from glancing through the arched, latticed windows that opened so bewilderingly on every hand.");
  quotes.push_back("There now ensued a series of incidents which transported me to the opposite extremes of ecstasy and horror; incidents which I tremble to recall and dare not seek to interpret...");
  quotes.push_back("From the new-flooded lands it flowed again, uncovering death and decay; and from its ancient and immemorial bed it trickled loathsomely, uncovering nighted secrets of the years when Time was young and the gods unborn.");
  quotes.push_back("The moon is dark, and the gods dance in the night; there is terror in the sky, for upon the moon hath sunk an eclipse foretold in no books of men or of earth's gods...");
  quotes.push_back("May the merciful gods, if indeed there be such, guard those hours when no power of the will, or drug that the cunning of man devises, can keep me from the chasm of sleep. Death is merciful, for there is no return therefrom, but with him who has come back out of the nethermost chambers of night, haggard and knowing, peace rests nevermore.");
  quotes.push_back("What I learned and saw in those hours of impious exploration can never be told, for want of symbols or suggestions in any language.");
  quotes.push_back("From even the greatest of horrors irony is seldom absent.");
  quotes.push_back("The most merciful thing in the world, I think, is the inability of the human mind to correlate all its contents.");
  quotes.push_back("In his house at R'lyeh dead Cthulhu waits dreaming.");
  quotes.push_back("Ph'nglui mglw'nafh Cthulhu R'lyeh wgah'nagl fhtagn");
  quotes.push_back("They worshipped, so they said, the Great Old Ones who lived ages before there were any men, and who came to the young world out of the sky...");
  quotes.push_back("That is not dead which can eternal lie, and with strange aeons even death may die.");
  quotes.push_back("I have looked upon all that the universe has to hold of horror, and even the skies of spring and the flowers of summer must ever afterward be poison to me. But I do not think my life will be long. (...) I know too much, and the cult still lives.");
  quotes.push_back("Something terrible came to the hills and valleys on that meteor, and something terrible, though I know not in what proportion, still remains.");
  quotes.push_back("Man's respect for the imponderables varies according to his mental constitution and environment. Through certain modes of thought and training it can be elevated tremendously, yet there is always a limit.");
  quotes.push_back("As human beings, our only sensible scale of values is one based on lessening the agony of existence.");
  quotes.push_back("The oldest and strongest emotion of mankind is fear, and the oldest and strongest kind of fear is fear of the unknown.");
  quotes.push_back("I have seen the dark universe yawning, where the black planets roll without aim, where they roll in their horror unheeded, without knowledge, or lustre, or name.");
  quotes.push_back("Searchers after horror haunt strange, far places.");
  quotes.push_back("The sciences, each straining in its own direction, have hitherto harmed us little; but some day the piecing together of dissociated knowledge will open up such terrifying vistas of reality, and of our frightful position therein, that we shall either go mad from the revelation or flee from the deadly light into the peace and safety of a new dark age.");
  quotes.push_back("There are horrors beyond life's edge that we do not suspect, and once in a while man's evil prying calls them just within our range.");
  quotes.push_back("We live on a placid island of ignorance in the midst of black seas of infinity, and it was not meant that we should voyage far.");
  quotes.push_back("There are black zones of shadow close to our daily paths, and now and then some evil soul breaks a passage through. When that happens, the man who knows must strike before reckoning the consequences.");
  quotes.push_back("Non-Euclidean calculus and quantum physics are enough to stretch any brain; and when one mixes them with folklore, and tries to trace a strange background of multi-dimensional reality behind the ghoulish hints of Gothic tales and the wild whispers of the chimney-corner, one can hardly expect to be wholly free from mental tension.");
  quotes.push_back("I could not help feeling that they were evil things-- mountains of madness whose farther slopes looked out over some accursed ultimate abyss. That seething, half-luminous cloud-background held ineffable suggestions of a vague, ethereal beyondness far more than terrestrially spatial; and gave appalling reminders of the utter remoteness, separateness, desolation, and aeon-long death of this untrodden and unfathomed austral world.");
  quotes.push_back("With five feeble senses we pretend to comprehend the boundlessly complex cosmos, yet other beings with wider, stronger, or different range of senses might not only see very differently the things we see, but might see and study whole worlds of matter, energy, and life which lie close at hand yet can never be detected with the senses we have.");
  quotes.push_back("It is absolutely necessary, for the peace and safety of mankind, that some of earth's dark, dead corners and unplumbed depths be left alone; lest sleeping abnormalities wake to resurgent life, and blasphemously surviving nightmares squirm and splash out of their black lairs to newer and wider conquests.");
  quotes.push_back("I felt myself on the edge of the world; peering over the rim into a fathomless chaos of eternal night.");
  quotes.push_back("And where Nyarlathotep went, rest vanished, for the small hours were rent with the screams of nightmare.");
  quotes.push_back("It was just a colour out of space - a frightful messenger from unformed realms of infinity beyond all Nature as we know it; from realms whose mere existence stuns the brain and numbs us with the black extra-cosmic gulfs it throws open before our frenzied eyes.");
  quotes.push_back("It lumbered slobberingly into sight and gropingly squeezed its gelatinous green immensity through the black doorway into the tainted outside air of that poison city of madness. The Thing cannot be described - there is no language for such abysms of shrieking and immemorial lunacy, such eldritch contradictions of all matter, force, and cosmic order.");
  quotes.push_back("I could tell I was at the gateway of a region half-bewitched through the piling-up of unbroken time-accumulations; a region where old, strange things have had a chance to grow and linger because they have never been stirred up.");
  return "\"" + quotes.at(eng->dice.range(0, quotes.size() - 1)) + "\"";
}
