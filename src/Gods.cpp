#include "Gods.h"

#include "Utils.h"

namespace Gods {

namespace {

vector<God> godList;
int         currentGodElem_;

void Gods::initGodList() {
  godList.push_back(God("Abholos", "The Devourer in the Mist"));
  godList.push_back(God("Alala", "The Herald of S'glhuo"));
  godList.push_back(God("Ammutseba", "The Devourer of Stars"));
  godList.push_back(God("Aphoom-Zhah", "The Cold Flame"));
  godList.push_back(God("Apocolothoth", "The Moon-God"));
  godList.push_back(God("Atlach-Nacha", "The Spider God"));
  godList.push_back(God("Ayiig", "The Serpent Goddess"));
  godList.push_back(God("Aylith", "The Widow in the Woods"));
  godList.push_back(God("Baoht Zuqqa-Mogg", "The Bringer of Pestilence"));
  godList.push_back(God("Basatan", "The Master of the Crabs"));
  godList.push_back(God("Bgnu-Thun", "The Soul-Chilling Ice-God"));
  godList.push_back(God("Bokrug", "The Great Water Lizard"));
  godList.push_back(God("Bugg-Shash", "The Black One"));
  godList.push_back(God("Cthaat", "The Dark Water God"));
  godList.push_back(God("Cthugha", "The Living Flame"));
  godList.push_back(God("Cthylla", "The Secret Daughter of Cthulhu"));
  godList.push_back(God("Ctoggha", "The Dream-Daemon"));
  godList.push_back(God("Cyaegha", "The Destroying Eye"));
  godList.push_back(God("Dygra", "The Stone-Thing"));
  godList.push_back(God("Dythalla", "The Lord of Lizards"));
  godList.push_back(God("Eihort", "The God of the Labyrinth"));
  godList.push_back(God("Ghisguth", "The Sound of the Deep Waters"));
  godList.push_back(God("Glaaki", "The Lord of Dead Dreams"));
  godList.push_back(God("Gleeth", "The Blind God of the Moon"));
  godList.push_back(God("Gloon", "The Corrupter of Flesh"));
  godList.push_back(God("Gog-Hoor", "The Eater on the Insane"));
  godList.push_back(God("Gol-goroth", "The God of the Black Stone"));
  godList.push_back(God("Groth-Golka", "The Demon Bird-God"));
  godList.push_back(God("Gurathnaka", "The Eater of Dreams"));
  godList.push_back(God("Han", "The Dark One"));
  godList.push_back(God("Hastur", "The King in Yellow"));
  godList.push_back(God("Hchtelegoth", "The Great Tentacled God"));
  godList.push_back(God("Hziulquoigmnzhah", "The God of Cykranosh"));
  godList.push_back(God("Inpesca", "The Sea Horror"));
  godList.push_back(God("Iod", "The Shining Hunter"));
  godList.push_back(God("Istasha", "The Mistress of Darkness"));
  godList.push_back(God("Ithaqua", "The God of the Cold White Silence"));
  godList.push_back(God("Janaingo", "The Guardian of the Watery Gates"));
  godList.push_back(God("Kaalut", "The Ravenous One"));
  godList.push_back(God("Kassogtha", "The Leviathan of Disease"));
  godList.push_back(God("Kaunuzoth", "The Great One"));
  godList.push_back(God("Lam", "The Grey"));
  godList.push_back(God("Lythalia", "The Forest-Goddess"));
  godList.push_back(God("Mnomquah", "The Lord of the Black Lake"));
  godList.push_back(God("Mordiggian", "The Great Ghoul"));
  godList.push_back(God("Mormo", "The Thousand-Faced Moon"));
  godList.push_back(God("Mynoghra", "The She-Daemon of the Shadows"));
  godList.push_back(God("Ngirrthlu", "The Wolf-Thing"));
  godList.push_back(God("Northot", "The Forgotten God"));
  godList.push_back(God("Nssu-Ghahnb", "The Leech of the Aeons"));
  godList.push_back(God("Nycrama", "The Zombifying Essence"));
  godList.push_back(God("Nyogtha", "The Haunter of the Red Abyss"));
  godList.push_back(God("Obmbu", "The Shatterer"));
  godList.push_back(God("Othuum", "The Oceanic Horror"));
  godList.push_back(God("Othuyeg", "The Doom-Walker"));
  godList.push_back(God("Psuchawrl", "The Elder One"));
  godList.push_back(God("Quyagen", "He Who Dwells Beneath Our Feet"));
  godList.push_back(God("Rhan-Tegoth", "He of the Ivory Throne"));
  godList.push_back(God("Rlim Shaikorth", "The White Worm"));
  godList.push_back(God("Ruhtra Dyoll", "The Fire God"));
  godList.push_back(God("Sebek", "The Crocodile God"));
  godList.push_back(God("Sedmelluq", "The Great Manipulator"));
  godList.push_back(God("Sfatlicllp", "The Fallen Wisdom"));
  godList.push_back(God("Shaklatal", "The Eye of Wicked Sight"));
  godList.push_back(God("Sheb-Teth", "The Devourer of Souls"));
  godList.push_back(God("Shterot", "The Tenebrous One"));
  godList.push_back(God("Shudde M'ell", "The Burrower Beneath"));
  godList.push_back(God("Shuy-Nihl", "The Devourer in the Earth"));
  godList.push_back(God("Sthanee", "The Lost One"));
  godList.push_back(God("Summanus", "The Monarch of Night"));
  godList.push_back(God("Thanaroa", "The Shining One"));
  godList.push_back(God("Tharapithia", "The Shadow in the Crimson Light"));
  godList.push_back(God("Thog", "The Demon-God of Xuthal"));
  godList.push_back(God("Thrygh", "The Godbeast"));
  godList.push_back(God("Tsathoggua", "The Toad-God"));
  godList.push_back(God("Tulushuggua", "The Watery Dweller Beneath"));
  godList.push_back(God("Vibur", "The Thing from Beyond"));
  godList.push_back(God("Volgna-Gath", "The Keeper of the Secrets"));
  godList.push_back(God("Vthyarilops", "The Starfish God"));
  godList.push_back(God("Xalafu", "The Dread One"));
  godList.push_back(God("Xcthol", "The Goat God"));
  godList.push_back(God("Xinlurgash", "The Ever-Consuming"));
  godList.push_back(God("Xirdneth", "The Maker of Illusions"));
  godList.push_back(God("Xotli", "The Lord of Terror"));
  godList.push_back(God("Yegg-Ha", "The Faceless One"));
  godList.push_back(God("Ygolonac", "The Defiler"));
  godList.push_back(God("Yig", "The Father of Serpents"));
  godList.push_back(God("Ymnar", "The Dark Stalker"));
  godList.push_back(God("Yog-Sapha", "The Dweller in the Depths"));
  godList.push_back(God("Yorith", "The Oldest Dreamer"));
  godList.push_back(God("Ythogtha", "The Thing in the Pit"));
  godList.push_back(God("Yug-Siturath", "The All-Consuming Fog"));
  godList.push_back(God("Zathog", "The Black Lord of Whirling Vortices"));
  godList.push_back(God("Zindarak", "The Fiery Messenger"));
  godList.push_back(God("Zushakon", "The Dark Silent One"));
  godList.push_back(God("Zvilpogghua", "The Feaster from the Stars"));
  godList.push_back(God("Gozer", "The Destroyer"));
}

} //namespace

void init() {
  initGodList();
}

bool isGodLevel() {
  return currentGodElem_ >= 0;
}

const God* getCurrentGod() {
  if(currentGodElem_ >= 0) {return &(godList.at(currentGodElem_));}
  return NULL;
}

void Gods::setRandomGod() {
  const int NR_GODS = int(godList.size());
  currentGodElem_ = Rnd::range(0, NR_GODS - 1);
}

void Gods::setNoGod() {
  currentGodElem_ = -1;
}

} //Gods
