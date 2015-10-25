#ifndef INSANITY
#define INSANITY

#include <vector>
#include <string>

#include "cmn_data.hpp"
#include "converters.hpp"

enum class Ins_sympt_id
{
    scream,
    babbling,
    faint,
    laugh,
    phobia_rat,
    phobia_spider,
//    phobia_snake,
    phobia_canine,
    phobia_dead,
    phobia_open,
    phobia_confined,
    phobia_deep,
    phobia_dark,
    masoch,
    sadism,
    shadows,
    confusion,
    frenzy,
    strange_sensation,
    END
};

class Actor;

class Ins_sympt
{
public:
    Ins_sympt(const Ins_sympt_id id) :
        id_ (id) {}

    virtual ~Ins_sympt() {}

    Ins_sympt_id id() const
    {
        return id_;
    }

    virtual void save() const {}

    virtual void load() {}

    virtual bool is_permanent() const = 0;

    virtual bool allow_gain() const
    {
        return true;
    }

    void on_start();

    void on_end();

    virtual void on_new_player_turn(const std::vector<Actor*>& seen_foes)
    {
        (void)seen_foes;
    }

    virtual std::string char_descr_msg() const
    {
        return "";
    }

    virtual std::string postmortem_msg() const
    {
        return "";
    }

protected:
    virtual void on_start_hook() {}

    virtual std::string start_msg() const = 0;
    virtual std::string start_heading() const = 0;

    virtual std::string end_msg() const
    {
        return "";
    }

    virtual std::string history_msg() const = 0;

    virtual std::string history_msg_end() const
    {
        return "";
    }

    const Ins_sympt_id id_;
};

class Ins_scream : public Ins_sympt
{
public:
    Ins_scream() :
        Ins_sympt(Ins_sympt_id::scream) {}

    bool is_permanent() const override
    {
        return false;
    }

    bool allow_gain() const override;

protected:
    void on_start_hook() override;

    std::string start_msg() const override;

    std::string start_heading() const override
    {
        return "Screaming!";
    }

    std::string history_msg() const override
    {
        return "Screamed in terror.";
    }
};

class Ins_babbling : public Ins_sympt
{
public:
    Ins_babbling() :
        Ins_sympt(Ins_sympt_id::babbling) {}

    bool is_permanent() const override
    {
        return false;
    }

    void on_new_player_turn(const std::vector<Actor*>& seen_foes) override;

protected:
    void on_start_hook() override;

    std::string start_msg() const override
    {
        return "I find myself babbling incoherently.";
    }

    std::string start_heading() const override
    {
        return "Babbling!";
    }

    std::string history_msg() const override
    {
        return "Started babbling incoherently.";
    }
};

class Ins_faint : public Ins_sympt
{
public:
    Ins_faint() :
        Ins_sympt(Ins_sympt_id::faint) {}

    bool is_permanent() const override
    {
        return false;
    }

    bool allow_gain() const override;

protected:
    void on_start_hook() override;

    std::string start_msg() const override
    {
        return "Everything is blacking out.";
    }

    std::string start_heading() const override
    {
        return "Fainting!";
    }

    std::string history_msg() const override
    {
        return "Fainted.";
    }
};

class Ins_laugh : public Ins_sympt
{
public:
    Ins_laugh() :
        Ins_sympt(Ins_sympt_id::laugh) {}

    bool is_permanent() const override
    {
        return false;
    }

protected:
    void on_start_hook() override;

    std::string start_msg() const override
    {
        return "I laugh maniacally.";
    }

    std::string start_heading() const override
    {
        return "HAHAHA!";
    }

    std::string history_msg() const override
    {
        return "Laughed maniacally.";
    }
};

class Ins_phobia_rat : public Ins_sympt
{
public:
    Ins_phobia_rat() :
        Ins_sympt(Ins_sympt_id::phobia_rat) {}

    bool is_permanent() const override
    {
        return true;
    }

    void on_new_player_turn(const std::vector<Actor*>& seen_foes) override;

    bool allow_gain() const override;

    std::string char_descr_msg() const override
    {
        return "Phobia of rats";
    }

    std::string postmortem_msg() const override
    {
        return "Had a phobia of rats";
    }

protected:
    std::string start_msg() const override
    {
        return "Rats suddenly seem terrifying.";
    }

    std::string start_heading() const override
    {
        return "Murophobia!";
    }

    std::string end_msg() const override
    {
        return "I am no longer terrified of rats.";
    }

    std::string history_msg() const override
    {
        return "Gained a phobia of rats.";
    }

    std::string history_msg_end() const override
    {
        return "My phobia of rats was cured.";
    }
};

class Ins_phobia_spider : public Ins_sympt
{
public:
    Ins_phobia_spider() :
        Ins_sympt(Ins_sympt_id::phobia_spider) {}

    bool is_permanent() const override
    {
        return true;
    }

    void on_new_player_turn(const std::vector<Actor*>& seen_foes) override;

    bool allow_gain() const override;

    std::string char_descr_msg() const override
    {
        return "Phobia of spiders";
    }

    std::string postmortem_msg() const override
    {
        return "Had a phobia of spiders";
    }

protected:
    std::string start_msg() const override
    {
        return "Spiders suddenly seem terrifying.";
    }

    std::string start_heading() const override
    {
        return "Arachnophobia!";
    }

    std::string end_msg() const override
    {
        return "I am no longer terrified of spiders.";
    }

    std::string history_msg() const override
    {
        return "Gained a phobia of spiders.";
    }

    std::string history_msg_end() const override
    {
        return "My phobia of spiders was cured.";
    }
};

class Ins_phobia_canine : public Ins_sympt
{
public:
    Ins_phobia_canine() :
        Ins_sympt(Ins_sympt_id::phobia_canine) {}

    bool is_permanent() const override
    {
        return true;
    }

    void on_new_player_turn(const std::vector<Actor*>& seen_foes) override;

    bool allow_gain() const override;

    std::string char_descr_msg() const override
    {
        return "Phobia of canines";
    }

    std::string postmortem_msg() const override
    {
        return "Had a phobia of canines";
    }

protected:
    std::string start_msg() const override
    {
        return "Canines suddenly seem terrifying.";
    }

    std::string start_heading() const override
    {
        return "Cynophobia!";
    }

    std::string end_msg() const override
    {
        return "I am no longer terrified of canines.";
    }

    std::string history_msg() const override
    {
        return "Gained a phobia of canines.";
    }

    std::string history_msg_end() const override
    {
        return "My phobia of canines was cured.";
    }
};

class Ins_phobia_dead : public Ins_sympt
{
public:
    Ins_phobia_dead() :
        Ins_sympt(Ins_sympt_id::phobia_dead) {}

    bool is_permanent() const override
    {
        return true;
    }

    void on_new_player_turn(const std::vector<Actor*>& seen_foes) override;

    bool allow_gain() const override;

    std::string char_descr_msg() const override
    {
        return "Phobia of the dead";
    }

    std::string postmortem_msg() const override
    {
        return "Had a phobia of the dead";
    }

protected:
    std::string start_msg() const override
    {
        return "The dead suddenly seem far more terrifying.";
    }

    std::string start_heading() const override
    {
        return "Necrophobia!";
    }

    std::string end_msg() const override
    {
        return "I am no longer terrified of the dead.";
    }

    std::string history_msg() const override
    {
        return "Gained a phobia of the dead.";
    }

    std::string history_msg_end() const override
    {
        return "My phobia of the dead was cured.";
    }
};

class Ins_phobia_open : public Ins_sympt
{
public:
    Ins_phobia_open() :
        Ins_sympt(Ins_sympt_id::phobia_open) {}

    bool is_permanent() const override
    {
        return true;
    }

    void on_new_player_turn(const std::vector<Actor*>& seen_foes) override;

    bool allow_gain() const override;

    std::string char_descr_msg() const override
    {
        return "Phobia of open places";
    }

    std::string postmortem_msg() const override
    {
        return "Had a phobia of open places";
    }

protected:
    std::string start_msg() const override
    {
        return "Open places suddenly seem terrifying.";
    }

    std::string start_heading() const override
    {
        return "Agoraphobia!";
    }

    std::string end_msg() const override
    {
        return "I am no longer terrified of open places.";
    }

    std::string history_msg() const override
    {
        return "Gained a phobia of open places.";
    }

    std::string history_msg_end() const override
    {
        return "My phobia of open places was cured.";
    }
};

class Ins_phobia_confined : public Ins_sympt
{
public:
    Ins_phobia_confined() :
        Ins_sympt(Ins_sympt_id::phobia_confined) {}

    bool is_permanent() const override
    {
        return true;
    }

    void on_new_player_turn(const std::vector<Actor*>& seen_foes) override;

    bool allow_gain() const override;

    std::string char_descr_msg() const override
    {
        return "Phobia of confined places";
    }

    std::string postmortem_msg() const override
    {
        return "Had a phobia of confined places";
    }

protected:
    std::string start_msg() const override
    {
        return "Confined places suddenly seem terrifying.";
    }

    std::string start_heading() const override
    {
        return "Claustrophobia!";
    }

    std::string end_msg() const override
    {
        return "I am no longer terrified of confined places.";
    }

    std::string history_msg() const override
    {
        return "Gained a phobia of confined places.";
    }

    std::string history_msg_end() const override
    {
        return "My phobia of confined places was cured.";
    }
};

class Ins_phobia_deep : public Ins_sympt
{
public:
    Ins_phobia_deep() :
        Ins_sympt(Ins_sympt_id::phobia_deep) {}

    bool is_permanent() const override
    {
        return true;
    }

    void on_new_player_turn(const std::vector<Actor*>& seen_foes) override;

    bool allow_gain() const override;

    std::string char_descr_msg() const override
    {
        return "Phobia of deep places";
    }

    std::string postmortem_msg() const override
    {
        return "Had a phobia of deep places";
    }

protected:
    std::string start_msg() const override
    {
        return "It suddenly seems far more terrifying to delve deeper.";
    }

    std::string start_heading() const override
    {
        return "Bathophobia!";
    }

    std::string end_msg() const override
    {
        return "I am no longer terrified of deep places.";
    }

    std::string history_msg() const override
    {
        return "Gained a phobia of deep places.";
    }

    std::string history_msg_end() const override
    {
        return "My phobia of deep places was cured.";
    }
};

class Ins_phobia_dark : public Ins_sympt
{
public:
    Ins_phobia_dark() :
        Ins_sympt(Ins_sympt_id::phobia_dark) {}

    bool is_permanent() const override
    {
        return true;
    }

    void on_new_player_turn(const std::vector<Actor*>& seen_foes) override;

    bool allow_gain() const override;

    std::string char_descr_msg() const override
    {
        return "Phobia of darkness";
    }

    std::string postmortem_msg() const override
    {
        return "Had a phobia of darkness";
    }

protected:
    std::string start_msg() const override
    {
        return "Darkness suddenly seems far more terrifying.";
    }

    std::string start_heading() const override
    {
        return "Nyctophobia!";
    }

    std::string end_msg() const override
    {
        return "I am no longer terrified of darkness.";
    }

    std::string history_msg() const override
    {
        return "Gained a phobia of darkness.";
    }

    std::string history_msg_end() const override
    {
        return "My phobia of darkness was cured.";
    }
};

class Ins_masoch : public Ins_sympt
{
public:
    Ins_masoch() :
        Ins_sympt(Ins_sympt_id::masoch) {}

    bool is_permanent() const override
    {
        return true;
    }

    std::string char_descr_msg() const override
    {
        return "Masochistic obsession";
    }

    std::string postmortem_msg() const override
    {
        return "Had a masochistic obsession";
    }

protected:
    std::string start_msg() const override
    {
        return "To my alarm, I find myself encouraged by the sensation of pain. Physical "
               "suffering does not bother me at all. However, my depraved mind can never find "
               "complete peace (no shock from taking damage, but permanent +" +
               to_str(SHOCK_FROM_OBSESSION) + "% shock).";
    }

    std::string start_heading() const override
    {
        return "Masochistic obsession!";
    }

    std::string end_msg() const override
    {
        return "I am cured of my masochistic obsession.";
    }

    std::string history_msg() const override
    {
        return "Gained a masochistic obsession.";
    }

    std::string history_msg_end() const override
    {
        return "My masochistic obsession was cured.";
    }
};

class Ins_sadism : public Ins_sympt
{
public:
    Ins_sadism() :
        Ins_sympt(Ins_sympt_id::sadism) {}

    bool is_permanent() const override
    {
        return true;
    }

    std::string char_descr_msg() const override
    {
        return "Sadistic obsession";
    }

    std::string postmortem_msg() const override
    {
        return "Had a sadistic obsession";
    }

protected:
    std::string start_msg() const override
    {
        return "To my alarm, I find myself encouraged by the pain I cause in others. For every "
               "life I take, I find a little relief. However, my depraved mind can no longer "
               "find complete peace (permanent +" + to_str(SHOCK_FROM_OBSESSION) + "% shock).";
    }

    std::string start_heading() const override
    {
        return "Sadistic obsession!";
    }

    std::string end_msg() const override
    {
        return "I am cured of my sadistic obsession.";
    }

    std::string history_msg() const override
    {
        return "Gained a sadistic obsession.";
    }

    std::string history_msg_end() const override
    {
        return "My sadistic obsession was cured.";
    }
};

class Ins_shadows : public Ins_sympt
{
public:
    Ins_shadows() :
        Ins_sympt(Ins_sympt_id::shadows) {}

    bool is_permanent() const override
    {
        return false;
    }

protected:
    void on_start_hook() override;

    std::string start_msg() const override
    {
        return "The shadows are closing in on me!";
    }

    std::string start_heading() const override
    {
        return "Haunted by shadows!";
    }

    std::string history_msg() const override
    {
        return "Was haunted by shadows.";
    }
};

class Ins_confusion : public Ins_sympt
{
public:
    Ins_confusion() :
        Ins_sympt(Ins_sympt_id::confusion) {}

    bool is_permanent() const override
    {
        return false;
    }

    bool allow_gain() const override;

protected:
    void on_start_hook() override;

    std::string start_msg() const override
    {
        return "I find myself in a peculiar trance. I struggle to recall where I am, and what "
               "is happening.";
    }

    std::string start_heading() const override
    {
        return "Confusion!";
    }

    std::string history_msg() const override
    {
        return "Suddenly felt deeply confused for no reason.";
    }
};

class Ins_frenzy : public Ins_sympt
{
public:
    Ins_frenzy() :
        Ins_sympt(Ins_sympt_id::frenzy) {}

    bool is_permanent() const override
    {
        return false;
    }

    bool allow_gain() const override;

protected:
    void on_start_hook() override;

    std::string start_msg() const override
    {
        return "I fall into an uncontrollable rage!";
    }

    std::string start_heading() const override
    {
        return "Frenzy!";
    }

    std::string history_msg() const override
    {
        return "Fell into an uncontrollable rage.";
    }
};

class Ins_strange_sensation : public Ins_sympt
{
public:
    Ins_strange_sensation() :
        Ins_sympt(Ins_sympt_id::strange_sensation) {}

    bool is_permanent() const override
    {
        return false;
    }

protected:
    std::string start_msg() const override
    {
        return "There is a strange itch, as if something is crawling on the back of my neck.";
    }

    std::string start_heading() const override
    {
        return "Strange sensation!";
    }

    std::string history_msg() const override
    {
        return "Had a sensation of something crawling on my neck.";
    }
};

namespace insanity
{

void init();
void cleanup();

void save();
void load();

void gain_sympt();

bool has_sympt(const Ins_sympt_id id);

std::vector<const Ins_sympt*> active_sympts();

void on_new_player_turn(const std::vector<Actor*>& seen_foes);

void end_sympt(const Ins_sympt_id id);

} //insanity

#endif // INSANITY
