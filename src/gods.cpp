#include "gods.hpp"

#include <vector>
#include "rl_utils.hpp"

// -----------------------------------------------------------------------------
// Private
// -----------------------------------------------------------------------------
static const std::vector<God> god_list_ =
{
        {"Abholos", "The Devourer in the Mist"},
        {"Alala", "The Herald of S'glhuo"},
        {"Ammutseba", "The Devourer of Stars"},
        {"Aphoom-Zhah", "The Cold Flame"},
        {"Apocolothoth", "The Moon-God"},
        {"Atlach-Nacha", "The Spider God"},
        {"Ayiig", "The Serpent Goddess"},
        {"Aylith", "The Widow in the Woods"},
        {"Baoht Zuqqa-Mogg", "The Bringer of Pestilence"},
        {"Basatan", "The Master of the Crabs"},
        {"Bgnu-Thun", "The Soul-Chilling Ice-God"},
        {"Bokrug", "The Great Water Lizard"},
        {"Bugg-Shash", "The Black One"},
        {"Cthaat", "The Dark Water God"},
        {"Cthugha", "The Living Flame"},
        {"Cthylla", "The Secret Daughter of Cthulhu"},
        {"Ctoggha", "The Dream-Daemon"},
        {"Cyaegha", "The Destroying Eye"},
        {"Dygra", "The Stone-Thing"},
        {"Dythalla", "The Lord of Lizards"},
        {"Eihort", "The God of the Labyrinth"},
        {"Ghisguth", "The Sound of the Deep Waters"},
        {"Glaaki", "The Lord of Dead Dreams"},
        {"Gleeth", "The Blind God of the Moon"},
        {"Gloon", "The Corrupter of Flesh"},
        {"Gog-Hoor", "The Eater on the Insane"},
        {"Gol-goroth", "The God of the Black Stone"},
        {"Groth-Golka", "The Demon Bird-God"},
        {"Gurathnaka", "The Eater of Dreams"},
        {"Han", "The Dark One"},
        {"Hastur", "The King in Yellow"},
        {"Hchtelegoth", "The Great Tentacled God"},
        {"Hziulquoigmnzhah", "The God of Cykranosh"},
        {"Inpesca", "The Sea Horror"},
        {"Iod", "The Shining Hunter"},
        {"Istasha", "The Mistress of Darkness"},
        {"Ithaqua", "The God of the Cold White Silence"},
        {"Janaingo", "The Guardian of the Watery Gates"},
        {"Kaalut", "The Ravenous One"},
        {"Kassogtha", "The Leviathan of Disease"},
        {"Kaunuzoth", "The Great One"},
        {"Lam", "The Grey"},
        {"Lythalia", "The Forest-Goddess"},
        {"Mnomquah", "The Lord of the Black Lake"},
        {"Mordiggian", "The Great Ghoul"},
        {"Mormo", "The Thousand-Faced Moon"},
        {"Mynoghra", "The She-Daemon of the Shadows"},
        {"Ngirrthlu", "The Wolf-Thing"},
        {"Northot", "The Forgotten God"},
        {"Nssu-Ghahnb", "The Leech of the Aeons"},
        {"Nycrama", "The Zombifying Essence"},
        {"Nyogtha", "The Haunter of the Red Abyss"},
        {"Obmbu", "The Shatterer"},
        {"Othuum", "The Oceanic Horror"},
        {"Othuyeg", "The Doom-Walker"},
        {"Psuchawrl", "The Elder One"},
        {"Quyagen", "He Who Dwells Beneath Our Feet"},
        {"Rhan-Tegoth", "He of the Ivory Throne"},
        {"Rlim Shaikorth", "The White Worm"},
        {"Ruhtra Dyoll", "The Fire God"},
        {"Sebek", "The Crocodile God"},
        {"Sedmelluq", "The Great Manipulator"},
        {"Sfatlicllp", "The Fallen Wisdom"},
        {"Shaklatal", "The Eye of Wicked Sight"},
        {"Sheb-Teth", "The Devourer of Souls"},
        {"Shterot", "The Tenebrous One"},
        {"Shudde M'ell", "The Burrower Beneath"},
        {"Shuy-Nihl", "The Devourer in the Earth"},
        {"Sthanee", "The Lost One"},
        {"Summanus", "The Monarch of Night"},
        {"Thanaroa", "The Shining One"},
        {"Tharapithia", "The Shadow in the Crimson Light"},
        {"Thog", "The Demon-God of Xuthal"},
        {"Thrygh", "The Godbeast"},
        {"Tsathoggua", "The Toad-God"},
        {"Tulushuggua", "The Watery Dweller Beneath"},
        {"Vibur", "The Thing from Beyond"},
        {"Volgna-Gath", "The Keeper of the Secrets"},
        {"Vthyarilops", "The Starfish God"},
        {"Xalafu", "The Dread One"},
        {"Xcthol", "The Goat God"},
        {"Xinlurgash", "The Ever-Consuming"},
        {"Xirdneth", "The Maker of Illusions"},
        {"Xotli", "The Lord of Terror"},
        {"Yegg-Ha", "The Faceless One"},
        {"Ygolonac", "The Defiler"},
        {"Yig", "The Father of Serpents"},
        {"Ymnar", "The Dark Stalker"},
        {"Yog-Sapha", "The Dweller in the Depths"},
        {"Yorith", "The Oldest Dreamer"},
        {"Ythogtha", "The Thing in the Pit"},
        {"Yug-Siturath", "The All-Consuming Fog"},
        {"Zathog", "The Black Lord of Whirling Vortices"},
        {"Zindarak", "The Fiery Messenger"},
        {"Zushakon", "The Dark Silent One"},
        {"Zvilpogghua", "The Feaster from the Stars"},
        {"Gozer", "The Destroyer"}
};

static int current_god_idx_ = 0;

// -----------------------------------------------------------------------------
// gods
// -----------------------------------------------------------------------------
namespace gods
{

const God& current_god()
{
        return god_list_[current_god_idx_];
}

void set_random_god()
{
        const int nr_gods = (int)god_list_.size();

        current_god_idx_ = rnd::range(0, nr_gods - 1);
}

} // gods
