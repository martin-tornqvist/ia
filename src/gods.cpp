#include "gods.hpp"

#include <vector>
#include "rl_utils.hpp"

namespace gods
{

namespace
{

std::vector<God>    god_list_;
int                 cur_god_elem_;

void init_god_list()
{
    god_list_.push_back({"Abholos", "The Devourer in the Mist"});
    god_list_.push_back({"Alala", "The Herald of S'glhuo"});
    god_list_.push_back({"Ammutseba", "The Devourer of Stars"});
    god_list_.push_back({"Aphoom-Zhah", "The Cold Flame"});
    god_list_.push_back({"Apocolothoth", "The Moon-God"});
    god_list_.push_back({"Atlach-Nacha", "The Spider God"});
    god_list_.push_back({"Ayiig", "The Serpent Goddess"});
    god_list_.push_back({"Aylith", "The Widow in the Woods"});
    god_list_.push_back({"Baoht Zuqqa-Mogg", "The Bringer of Pestilence"});
    god_list_.push_back({"Basatan", "The Master of the Crabs"});
    god_list_.push_back({"Bgnu-Thun", "The Soul-Chilling Ice-God"});
    god_list_.push_back({"Bokrug", "The Great Water Lizard"});
    god_list_.push_back({"Bugg-Shash", "The Black One"});
    god_list_.push_back({"Cthaat", "The Dark Water God"});
    god_list_.push_back({"Cthugha", "The Living Flame"});
    god_list_.push_back({"Cthylla", "The Secret Daughter of Cthulhu"});
    god_list_.push_back({"Ctoggha", "The Dream-Daemon"});
    god_list_.push_back({"Cyaegha", "The Destroying Eye"});
    god_list_.push_back({"Dygra", "The Stone-Thing"});
    god_list_.push_back({"Dythalla", "The Lord of Lizards"});
    god_list_.push_back({"Eihort", "The God of the Labyrinth"});
    god_list_.push_back({"Ghisguth", "The Sound of the Deep Waters"});
    god_list_.push_back({"Glaaki", "The Lord of Dead Dreams"});
    god_list_.push_back({"Gleeth", "The Blind God of the Moon"});
    god_list_.push_back({"Gloon", "The Corrupter of Flesh"});
    god_list_.push_back({"Gog-Hoor", "The Eater on the Insane"});
    god_list_.push_back({"Gol-goroth", "The God of the Black Stone"});
    god_list_.push_back({"Groth-Golka", "The Demon Bird-God"});
    god_list_.push_back({"Gurathnaka", "The Eater of Dreams"});
    god_list_.push_back({"Han", "The Dark One"});
    god_list_.push_back({"Hastur", "The King in Yellow"});
    god_list_.push_back({"Hchtelegoth", "The Great Tentacled God"});
    god_list_.push_back({"Hziulquoigmnzhah", "The God of Cykranosh"});
    god_list_.push_back({"Inpesca", "The Sea Horror"});
    god_list_.push_back({"Iod", "The Shining Hunter"});
    god_list_.push_back({"Istasha", "The Mistress of Darkness"});
    god_list_.push_back({"Ithaqua", "The God of the Cold White Silence"});
    god_list_.push_back({"Janaingo", "The Guardian of the Watery Gates"});
    god_list_.push_back({"Kaalut", "The Ravenous One"});
    god_list_.push_back({"Kassogtha", "The Leviathan of Disease"});
    god_list_.push_back({"Kaunuzoth", "The Great One"});
    god_list_.push_back({"Lam", "The Grey"});
    god_list_.push_back({"Lythalia", "The Forest-Goddess"});
    god_list_.push_back({"Mnomquah", "The Lord of the Black Lake"});
    god_list_.push_back({"Mordiggian", "The Great Ghoul"});
    god_list_.push_back({"Mormo", "The Thousand-Faced Moon"});
    god_list_.push_back({"Mynoghra", "The She-Daemon of the Shadows"});
    god_list_.push_back({"Ngirrthlu", "The Wolf-Thing"});
    god_list_.push_back({"Northot", "The Forgotten God"});
    god_list_.push_back({"Nssu-Ghahnb", "The Leech of the Aeons"});
    god_list_.push_back({"Nycrama", "The Zombifying Essence"});
    god_list_.push_back({"Nyogtha", "The Haunter of the Red Abyss"});
    god_list_.push_back({"Obmbu", "The Shatterer"});
    god_list_.push_back({"Othuum", "The Oceanic Horror"});
    god_list_.push_back({"Othuyeg", "The Doom-Walker"});
    god_list_.push_back({"Psuchawrl", "The Elder One"});
    god_list_.push_back({"Quyagen", "He Who Dwells Beneath Our Feet"});
    god_list_.push_back({"Rhan-Tegoth", "He of the Ivory Throne"});
    god_list_.push_back({"Rlim Shaikorth", "The White Worm"});
    god_list_.push_back({"Ruhtra Dyoll", "The Fire God"});
    god_list_.push_back({"Sebek", "The Crocodile God"});
    god_list_.push_back({"Sedmelluq", "The Great Manipulator"});
    god_list_.push_back({"Sfatlicllp", "The Fallen Wisdom"});
    god_list_.push_back({"Shaklatal", "The Eye of Wicked Sight"});
    god_list_.push_back({"Sheb-Teth", "The Devourer of Souls"});
    god_list_.push_back({"Shterot", "The Tenebrous One"});
    god_list_.push_back({"Shudde M'ell", "The Burrower Beneath"});
    god_list_.push_back({"Shuy-Nihl", "The Devourer in the Earth"});
    god_list_.push_back({"Sthanee", "The Lost One"});
    god_list_.push_back({"Summanus", "The Monarch of Night"});
    god_list_.push_back({"Thanaroa", "The Shining One"});
    god_list_.push_back({"Tharapithia", "The Shadow in the Crimson Light"});
    god_list_.push_back({"Thog", "The Demon-God of Xuthal"});
    god_list_.push_back({"Thrygh", "The Godbeast"});
    god_list_.push_back({"Tsathoggua", "The Toad-God"});
    god_list_.push_back({"Tulushuggua", "The Watery Dweller Beneath"});
    god_list_.push_back({"Vibur", "The Thing from Beyond"});
    god_list_.push_back({"Volgna-Gath", "The Keeper of the Secrets"});
    god_list_.push_back({"Vthyarilops", "The Starfish God"});
    god_list_.push_back({"Xalafu", "The Dread One"});
    god_list_.push_back({"Xcthol", "The Goat God"});
    god_list_.push_back({"Xinlurgash", "The Ever-Consuming"});
    god_list_.push_back({"Xirdneth", "The Maker of Illusions"});
    god_list_.push_back({"Xotli", "The Lord of Terror"});
    god_list_.push_back({"Yegg-Ha", "The Faceless One"});
    god_list_.push_back({"Ygolonac", "The Defiler"});
    god_list_.push_back({"Yig", "The Father of Serpents"});
    god_list_.push_back({"Ymnar", "The Dark Stalker"});
    god_list_.push_back({"Yog-Sapha", "The Dweller in the Depths"});
    god_list_.push_back({"Yorith", "The Oldest Dreamer"});
    god_list_.push_back({"Ythogtha", "The Thing in the Pit"});
    god_list_.push_back({"Yug-Siturath", "The All-Consuming Fog"});
    god_list_.push_back({"Zathog", "The Black Lord of Whirling Vortices"});
    god_list_.push_back({"Zindarak", "The Fiery Messenger"});
    god_list_.push_back({"Zushakon", "The Dark Silent One"});
    god_list_.push_back({"Zvilpogghua", "The Feaster from the Stars"});
    god_list_.push_back({"Gozer", "The Destroyer"});
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

const God* cur_god()
{
    if (cur_god_elem_ >= 0)
    {
        return &(god_list_[cur_god_elem_]);
    }

    return nullptr;
}

void set_random_god()
{
    const int nr_gods   = int(god_list_.size());
    cur_god_elem_       = rnd::range(0, nr_gods - 1);
}

void set_no_god()
{
    cur_god_elem_ = -1;
}

} //gods
