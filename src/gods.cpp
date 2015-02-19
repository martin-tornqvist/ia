#include "Gods.h"

#include <vector>

#include "Utils.h"

using namespace std;

namespace Gods
{

namespace
{

vector<God> god_list;
int         cur_god_elem_;

void init_god_list()
{
    god_list.push_back(God("Abholos", "The Devourer in the Mist"));
    god_list.push_back(God("Alala", "The Herald of S'glhuo"));
    god_list.push_back(God("Ammutseba", "The Devourer of Stars"));
    god_list.push_back(God("Aphoom-Zhah", "The Cold Flame"));
    god_list.push_back(God("Apocolothoth", "The Moon-God"));
    god_list.push_back(God("Atlach-Nacha", "The Spider God"));
    god_list.push_back(God("Ayiig", "The Serpent Goddess"));
    god_list.push_back(God("Aylith", "The Widow in the Woods"));
    god_list.push_back(God("Baoht Zuqqa-Mogg", "The Bringer of Pestilence"));
    god_list.push_back(God("Basatan", "The Master of the Crabs"));
    god_list.push_back(God("Bgnu-Thun", "The Soul-Chilling Ice-God"));
    god_list.push_back(God("Bokrug", "The Great Water Lizard"));
    god_list.push_back(God("Bugg-Shash", "The Black One"));
    god_list.push_back(God("Cthaat", "The Dark Water God"));
    god_list.push_back(God("Cthugha", "The Living Flame"));
    god_list.push_back(God("Cthylla", "The Secret Daughter of Cthulhu"));
    god_list.push_back(God("Ctoggha", "The Dream-Daemon"));
    god_list.push_back(God("Cyaegha", "The Destroying Eye"));
    god_list.push_back(God("Dygra", "The Stone-Thing"));
    god_list.push_back(God("Dythalla", "The Lord of Lizards"));
    god_list.push_back(God("Eihort", "The God of the Labyrinth"));
    god_list.push_back(God("Ghisguth", "The Sound of the Deep Waters"));
    god_list.push_back(God("Glaaki", "The Lord of Dead Dreams"));
    god_list.push_back(God("Gleeth", "The Blind God of the Moon"));
    god_list.push_back(God("Gloon", "The Corrupter of Flesh"));
    god_list.push_back(God("Gog-Hoor", "The Eater on the Insane"));
    god_list.push_back(God("Gol-goroth", "The God of the Black Stone"));
    god_list.push_back(God("Groth-Golka", "The Demon Bird-God"));
    god_list.push_back(God("Gurathnaka", "The Eater of Dreams"));
    god_list.push_back(God("Han", "The Dark One"));
    god_list.push_back(God("Hastur", "The King in Yellow"));
    god_list.push_back(God("Hchtelegoth", "The Great Tentacled God"));
    god_list.push_back(God("Hziulquoigmnzhah", "The God of Cykranosh"));
    god_list.push_back(God("Inpesca", "The Sea Horror"));
    god_list.push_back(God("Iod", "The Shining Hunter"));
    god_list.push_back(God("Istasha", "The Mistress of Darkness"));
    god_list.push_back(God("Ithaqua", "The God of the Cold White Silence"));
    god_list.push_back(God("Janaingo", "The Guardian of the Watery Gates"));
    god_list.push_back(God("Kaalut", "The Ravenous One"));
    god_list.push_back(God("Kassogtha", "The Leviathan of Disease"));
    god_list.push_back(God("Kaunuzoth", "The Great One"));
    god_list.push_back(God("Lam", "The Grey"));
    god_list.push_back(God("Lythalia", "The Forest-Goddess"));
    god_list.push_back(God("Mnomquah", "The Lord of the Black Lake"));
    god_list.push_back(God("Mordiggian", "The Great Ghoul"));
    god_list.push_back(God("Mormo", "The Thousand-Faced Moon"));
    god_list.push_back(God("Mynoghra", "The She-Daemon of the Shadows"));
    god_list.push_back(God("Ngirrthlu", "The Wolf-Thing"));
    god_list.push_back(God("Northot", "The Forgotten God"));
    god_list.push_back(God("Nssu-Ghahnb", "The Leech of the Aeons"));
    god_list.push_back(God("Nycrama", "The Zombifying Essence"));
    god_list.push_back(God("Nyogtha", "The Haunter of the Red Abyss"));
    god_list.push_back(God("Obmbu", "The Shatterer"));
    god_list.push_back(God("Othuum", "The Oceanic Horror"));
    god_list.push_back(God("Othuyeg", "The Doom-Walker"));
    god_list.push_back(God("Psuchawrl", "The Elder One"));
    god_list.push_back(God("Quyagen", "He Who Dwells Beneath Our Feet"));
    god_list.push_back(God("Rhan-Tegoth", "He of the Ivory Throne"));
    god_list.push_back(God("Rlim Shaikorth", "The White Worm"));
    god_list.push_back(God("Ruhtra Dyoll", "The Fire God"));
    god_list.push_back(God("Sebek", "The Crocodile God"));
    god_list.push_back(God("Sedmelluq", "The Great Manipulator"));
    god_list.push_back(God("Sfatlicllp", "The Fallen Wisdom"));
    god_list.push_back(God("Shaklatal", "The Eye of Wicked Sight"));
    god_list.push_back(God("Sheb-Teth", "The Devourer of Souls"));
    god_list.push_back(God("Shterot", "The Tenebrous One"));
    god_list.push_back(God("Shudde M'ell", "The Burrower Beneath"));
    god_list.push_back(God("Shuy-Nihl", "The Devourer in the Earth"));
    god_list.push_back(God("Sthanee", "The Lost One"));
    god_list.push_back(God("Summanus", "The Monarch of Night"));
    god_list.push_back(God("Thanaroa", "The Shining One"));
    god_list.push_back(God("Tharapithia", "The Shadow in the Crimson Light"));
    god_list.push_back(God("Thog", "The Demon-God of Xuthal"));
    god_list.push_back(God("Thrygh", "The Godbeast"));
    god_list.push_back(God("Tsathoggua", "The Toad-God"));
    god_list.push_back(God("Tulushuggua", "The Watery Dweller Beneath"));
    god_list.push_back(God("Vibur", "The Thing from Beyond"));
    god_list.push_back(God("Volgna-Gath", "The Keeper of the Secrets"));
    god_list.push_back(God("Vthyarilops", "The Starfish God"));
    god_list.push_back(God("Xalafu", "The Dread One"));
    god_list.push_back(God("Xcthol", "The Goat God"));
    god_list.push_back(God("Xinlurgash", "The Ever-Consuming"));
    god_list.push_back(God("Xirdneth", "The Maker of Illusions"));
    god_list.push_back(God("Xotli", "The Lord of Terror"));
    god_list.push_back(God("Yegg-Ha", "The Faceless One"));
    god_list.push_back(God("Ygolonac", "The Defiler"));
    god_list.push_back(God("Yig", "The Father of Serpents"));
    god_list.push_back(God("Ymnar", "The Dark Stalker"));
    god_list.push_back(God("Yog-Sapha", "The Dweller in the Depths"));
    god_list.push_back(God("Yorith", "The Oldest Dreamer"));
    god_list.push_back(God("Ythogtha", "The Thing in the Pit"));
    god_list.push_back(God("Yug-Siturath", "The All-Consuming Fog"));
    god_list.push_back(God("Zathog", "The Black Lord of Whirling Vortices"));
    god_list.push_back(God("Zindarak", "The Fiery Messenger"));
    god_list.push_back(God("Zushakon", "The Dark Silent One"));
    god_list.push_back(God("Zvilpogghua", "The Feaster from the Stars"));
    god_list.push_back(God("Gozer", "The Destroyer"));
}

} //namespace

void init()
{
    init_god_list();
}

bool is_god_lvl()
{
    return cur_god_elem_ >= 0;
}

const God* get_cur_god()
{
    if (cur_god_elem_ >= 0) {return &(god_list[cur_god_elem_]);}
    return nullptr;
}

void set_random_god()
{
    const int NR_GODS = int(god_list.size());
    cur_god_elem_ = Rnd::range(0, NR_GODS - 1);
}

void set_no_god()
{
    cur_god_elem_ = -1;
}

} //Gods
