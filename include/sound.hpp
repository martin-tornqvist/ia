#ifndef SOUND_HPP
#define SOUND_HPP

#include <string>

#include "item_data.hpp"

class Actor;

enum class SndVol
{
    low,
    high
};

enum class AlertsMon
{
    no,
    yes
};

enum class IgnoreMsgIfOriginSeen
{
    no,
    yes
};

class Snd
{
public:
    Snd(const std::string&              msg,
        const SfxId                     sfx,
        const IgnoreMsgIfOriginSeen     ignore_msg_if_origin_seen,
        const P&                        origin,
        Actor* const                    actor_who_made_sound,
        const SndVol                    vol,
        const AlertsMon                 alerting_mon,
        const MorePromptOnMsg           add_more_prompt_on_msg = MorePromptOnMsg::no);

    Snd() {}
    ~Snd() {}

    const std::string& msg() const
    {
        return msg_;
    }

    SfxId sfx() const
    {
        return sfx_;
    }

    void clear_msg()
    {
        msg_ = "";
    }

    bool is_msg_ignored_if_origin_seen() const
    {
        return is_msg_ignored_if_origin_seen_ == IgnoreMsgIfOriginSeen::yes;
    }

    bool is_alerting_mon() const
    {
        return is_alerting_mon_ == AlertsMon::yes;
    }

    MorePromptOnMsg should_add_more_prompt_on_msg() const
    {
        return add_more_prompt_on_msg_;
    }

    P origin() const
    {
        return origin_;
    }

    Actor* actor_who_made_sound() const
    {
        return actor_who_made_sound_;
    }

    int is_loud() const
    {
        return vol_ == SndVol::high;
    }

    void add_string(const std::string& str)
    {
        msg_ += str;
    }

private:
    std::string msg_;
    SfxId sfx_;
    IgnoreMsgIfOriginSeen is_msg_ignored_if_origin_seen_;
    P origin_;
    Actor* actor_who_made_sound_;
    SndVol vol_;
    AlertsMon is_alerting_mon_;
    MorePromptOnMsg add_more_prompt_on_msg_;
};

namespace snd_emit
{

void run(Snd snd);

void reset_nr_snd_msg_printed_current_turn();

} //snd_emit

#endif

