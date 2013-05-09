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
  tracer << "MainMenu::draw()..." << endl;

  vector<string> logo;

  if(eng->config->USE_TILE_SET == false) {
    logo.push_back("        ___  __                __  __                  ");
    logo.push_back("| |\\  | |   |  )  /\\      /\\  |  )/    /\\  |\\  |  /\\   ");
    logo.push_back("+ | \\ | +-- +--  ____    ____ +-- -   ____ | \\ | ____  ");
    logo.push_back("| |  \\| |   | \\ /    \\  /    \\| \\ \\__/    \\|  \\|/    \\ ");
    logo.push_back("               \\                 \\                      ");
  }

  int xPos = MAP_X_CELLS / 2;
  int yPos = 3;
//  const int X_POS_LEFT = xPos - 11;

  tracer << "MainMenu: Calling clearWindow()" << endl;
  eng->renderer->clearWindow();

  tracer << "MainMenu: Drawing random background letters" << endl;
  const int NR_X_CELLS = eng->config->SCREEN_WIDTH / eng->config->CELL_W;
  const int NR_Y_CELLS = eng->config->SCREEN_HEIGHT / eng->config->CELL_H;
  const int BG_BRIGHTNESS = eng->dice.getInRange(9, 13);
  for(int y = 0; y < NR_Y_CELLS; y++) {
    for(int x = 0; x < NR_X_CELLS; x++) {
      char cha = ' ';
      if(eng->dice.coinToss()) {
        cha = 'a' + eng->dice.getInRange(0, 25);
      }
      sf::Color bgClr = clrGray;
      bgClr.r = bgClr.g = bgClr.b = BG_BRIGHTNESS;
      eng->renderer->drawCharacter(cha, renderArea_screen, x, y, bgClr);
    }
  }

  tracer << "MainMenu: Drawing HPL quote" << endl;
  const int QUOTE_BRIGHTNESS = BG_BRIGHTNESS + 7;
  sf::Color quoteClr = clrGray;
  quoteClr.r = quoteClr.g = quoteClr.b = QUOTE_BRIGHTNESS;
  vector<string> quoteLines = eng->textFormatting->lineToLines(getHplQuote(), 28);
  for(unsigned int i = 0; i < quoteLines.size(); i++) {
    eng->renderer->drawText(quoteLines.at(i), renderArea_screen, 2, 13 + i, quoteClr);
  }

  sf::Color clrGeneral = clrRed;
  sf::Color clrBright = clrRedLight;

  if(eng->config->USE_TILE_SET) {
    tracer << "MainMenu: Calling drawMainMenuLogo()" << endl;
    eng->renderer->drawMainMenuLogo(1);
    yPos += 10;
  } else {
    const int LOGO_X_POS_LEFT = (MAP_X_CELLS - logo.at(0).size()) / 2;
    for(unsigned int i = 0; i < logo.size(); i++) {
      xPos = LOGO_X_POS_LEFT;
      for(unsigned int ii = 0; ii < logo.at(i).size(); ii++) {
        if(logo.at(i).at(ii) != ' ') {
          sf::Color clr = clrRed;
          clr.r += eng->dice.getInRange(-60, 100);
          clr.r = max(0, min(254, int(clr.r)));
          eng->renderer->drawCharacter(logo.at(i).at(ii), renderArea_screen, xPos, yPos, clr);
        }
        xPos++;
      }
      yPos += 1;
    }
    yPos += 3;
  }

  xPos = MAP_X_CELLS / 2;

  if(IS_DEBUG_MODE) {
    eng->renderer->drawText("## DEBUG MODE ##", renderArea_screen, 1, 1, clrYellow);
  }

  yPos += 3;

  eng->renderer->drawTextCentered("New journey", renderArea_screen, xPos, yPos, browser.isPosAtKey('a') ? clrBright : clrGeneral);
  yPos += 1;

  eng->renderer->drawTextCentered("Resurrect", renderArea_screen, xPos, yPos,  browser.isPosAtKey('b') ? clrBright : clrGeneral);
  yPos += 1;

  eng->renderer->drawTextCentered("Manual", renderArea_screen, xPos, yPos, browser.isPosAtKey('c') ? clrBright : clrGeneral);
  yPos += 1;

  eng->renderer->drawTextCentered("Options", renderArea_screen, xPos, yPos, browser.isPosAtKey('d') ? clrBright : clrGeneral);
  yPos += 1;

  eng->renderer->drawTextCentered("Credits", renderArea_screen, xPos, yPos, browser.isPosAtKey('e') ? clrBright : clrGeneral);
  yPos += 1;

  eng->renderer->drawTextCentered("High scores", renderArea_screen, xPos, yPos, browser.isPosAtKey('f') ? clrBright : clrGeneral);
  yPos += 1;

  eng->renderer->drawTextCentered("Escape to reality", renderArea_screen, xPos, yPos, browser.isPosAtKey('g') ? clrBright : clrGeneral);
  yPos += 1;

  if(IS_DEBUG_MODE) {
    eng->renderer->drawTextCentered("DEBUG: RUN BOT", renderArea_screen, xPos, yPos, browser.isPosAtKey('h') ? clrBright : clrGeneral);
    yPos += 1;
  }

  eng->renderer->drawTextCentered(eng->config->GAME_VERSION + "  (c) 2011-2013 Martin Tornqvist", renderArea_characterLines, xPos, 1, clrGeneral);

  eng->renderer->updateWindow();

  tracer << "MainMenu::draw() [DONE]" << endl;
}

GameEntry_t MainMenu::run(bool& quit) {
  tracer << "MainMenu::run()" << endl;

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
            eng->popup->showMessage("Sorry, no save available. Starting a new character instead.", false);
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
            eng->config->BOT_PLAYING = true;
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
  quotes.push_back("...the prosaic materialism of the majority condemns as madness the flashes of super-sight which penetrate the common veil of obvious empiricism...");
  quotes.push_back("I am writing this under an appreciable mental strain, since by tonight I shall be no more...");
  quotes.push_back("The end is near. I hear a noise at the door, as of some immense slippery body lumbering against it. It shall not find me...");
  quotes.push_back("In broad daylight, and at most seasons I am apt to think the greater part of it a mere dream; but sometimes in the autumn, about two in the morning when winds and animals howl dismally, there comes from inconceivable depths below a damnable suggestions of rhythmical throbbing ... and I feel that the transition of Juan Romero was a terrible one indeed...");
  quotes.push_back("Sometimes I believe that this less material life is our truer life, and that our vain presence on the terraqueous globe is itself the secondary or merely virtual phenomenon.");
  quotes.push_back("Life is a hideous thing, and from the background behind what we know of it peer daemoniacal hints of truth which make it sometimes a thousandfold more hideous.");
  quotes.push_back("Science, already oppressive with its shocking revelations, will perhaps be the ultimate exterminator of our human species, if separate species we be, for its reserve of unguessed horrors could never be borne by mortal brains if loosed upon the world....");
  quotes.push_back("Madness rides the star-wind... claws and teeth sharpened on centuries of corpses... dripping death astride a bacchanale of bats from nigh-black ruins of buried temples of Belial...");
  quotes.push_back("Memories and possibilities are ever more hideous than realities.");
  quotes.push_back("The only saving grace of the present is that it's too damned stupid to question the past very closely.");
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
  return "\"" + quotes.at(eng->dice.getInRange(0, quotes.size() - 1)) + "\"";
}
