#include "main_menu.hpp"

#include <string>

#include "init.hpp"
#include "colors.hpp"
#include "render.hpp"
#include "menu_input.hpp"
#include "save_handling.hpp"
#include "highscore.hpp"
#include "manual.hpp"
#include "popup.hpp"
#include "text_format.hpp"
#include "credits.hpp"
#include "audio.hpp"
#include "game_time.hpp"
#include "map_travel.hpp"
#include "actor.hpp"
#include "actor_player.hpp"
#include "utils.hpp"
#include "map.hpp"


namespace main_menu
{

namespace
{

std::string quote = "";

std::string hpl_quote()
{
    std::vector<std::string> quotes;

    quotes.push_back(
        "Happy is the tomb where no wizard hath lain and happy the town at night "
        "whose wizards are all ashes.");
    quotes.push_back(
        "Our means of receiving impressions are absurdly few, and our notions of "
        "surrounding objects infinitely narrow. We see things only as we are "
        "constructed to see them, and can gain no idea of their absolute nature.");
    quotes.push_back(
        "Disintegration is quite painless, I assure you.");
    quotes.push_back(
        "I am writing this under an appreciable mental strain, since by tonight I "
        "shall be no more...");
    quotes.push_back(
        "The end is near. I hear a noise at the door, as of some immense slippery "
        "body lumbering against it. It shall not find me...");
    quotes.push_back(
        "Sometimes I believe that this less material life is our truer life, and "
        "that our vain presence on the terraqueous globe is itself the secondary "
        "or merely virtual phenomenon.");
    quotes.push_back(
        "Life is a hideous thing, and from the background behind what we know of "
        "it peer daemoniacal hints of truth which make it sometimes a "
        "thousandfold more hideous.");
    quotes.push_back(
        "Science, already oppressive with its shocking revelations, will perhaps "
        "be the ultimate exterminator of our human species, if separate species "
        "we be, for its reserve of unguessed horrors could never be borne by "
        "mortal brains if loosed upon the world....");
    quotes.push_back(
        "Madness rides the star-wind... claws and teeth sharpened on centuries of "
        "corpses... dripping death astride a bacchanale of bats from nigh-black "
        "ruins of buried temples of Belial...");
    quotes.push_back(
        "Memories and possibilities are ever more hideous than realities.");
    quotes.push_back(
        "Yog-Sothoth knows the gate. Yog-Sothoth is the gate. Yog-Sothoth is the "
        "key and guardian of the gate. Past, present, future, all are one in "
        "Yog-Sothoth. He knows where the Old Ones broke through of old, and where "
        "They shall break through again.");
    quotes.push_back(
        "Slowly but inexorably crawling upon my consciousness and rising above "
        "every other impression, came a dizzying fear of the unknown; not death, "
        "but some nameless, unheard-of thing inexpressibly more ghastly and "
        "abhorrent.");
    quotes.push_back(
        "I felt that some horrible scene or object lurked beyond the silk-hung "
        "walls, and shrank from glancing through the arched, latticed windows "
        "that opened so bewilderingly on every hand.");
    quotes.push_back(
        "There now ensued a series of incidents which transported me to the "
        "opposite extremes of ecstasy and horror; incidents which I tremble to "
        "recall and dare not seek to interpret...");
    quotes.push_back(
        "From the new-flooded lands it flowed again, uncovering death and decay; "
        "and from its ancient and immemorial bed it trickled loathsomely, "
        "uncovering nighted secrets of the years when Time was young and the gods "
        "unborn.");
    quotes.push_back(
        "The moon is dark, and the gods dance in the night; there is terror in "
        "the sky, for upon the moon hath sunk an eclipse foretold in no books of "
        "men or of earth's gods...");
    quotes.push_back(
        "May the merciful gods, if indeed there be such, guard those hours when "
        "no power of the will can keep me from the chasm of sleep. With him who "
        "has come back out of the nethermost chambers of night, haggard and "
        "knowing, peace rests nevermore.");
    quotes.push_back(
        "What I learned and saw in those hours of impious exploration can never "
        "be told, for want of symbols or suggestions in any language.");
    quotes.push_back(
        "From even the greatest of horrors irony is seldom absent.");
    quotes.push_back(
        "The most merciful thing in the world, I think, is the inability of the "
        "human mind to correlate all its contents.");
    quotes.push_back(
        "In his house at R'lyeh dead Cthulhu waits dreaming.");
    quotes.push_back(
        "Ph'nglui mglw'nafh Cthulhu R'lyeh wgah'nagl fhtagn");
    quotes.push_back(
        "They worshipped, so they said, the Great Old Ones who lived ages before "
        "there were any men, and who came to the young world out of the sky...");
    quotes.push_back(
        "That is not dead which can eternal lie, and with strange aeons even "
        "death may die.");
    quotes.push_back(
        "I have looked upon all that the universe has to hold of horror, and even "
        "the skies of spring and the flowers of summer must ever afterward be "
        "poison to me. But I do not think my life will be long. I know too much, "
        "and the cult still lives.");
    quotes.push_back(
        "Something terrible came to the hills and valleys on that meteor, and "
        "something terrible, though I know not in what proportion, still remains.");
    quotes.push_back(
        "Man's respect for the imponderables varies according to his mental "
        "constitution and environment. Through certain modes of thought and "
        "training it can be elevated tremendously, yet there is always a limit.");
    quotes.push_back(
        "The oldest and strongest emotion of mankind is fear, and the oldest and "
        "strongest kind of fear is fear of the unknown.");
    quotes.push_back(
        "I have seen the dark universe yawning, where the black planets roll "
        "without aim, where they roll in their horror unheeded, without "
        "knowledge, or lustre, or name.");
    quotes.push_back(
        "Searchers after horror haunt strange, far places.");
    quotes.push_back(
        "The sciences have hitherto harmed us little; but some day the piecing "
        "together of dissociated knowledge will open up such terrifying vistas of "
        "reality, that we shall either go mad from the revelation or flee from "
        "the deadly light into the peace and safety of a new dark age.");
    quotes.push_back(
        "There are horrors beyond life's edge that we do not suspect, and once in "
        "a while man's evil prying calls them just within our range.");
    quotes.push_back(
        "We live on a placid island of ignorance in the midst of black seas of "
        "infinity, and it was not meant that we should voyage far.");
    quotes.push_back(
        "There are black zones of shadow close to our daily paths, and now and "
        "then some evil soul breaks a passage through. When that happens, the man "
        "who knows must strike before reckoning the consequences.");
    quotes.push_back(
        "Non-Euclidean calculus and quantum physics are enough to stretch any "
        "brain; and when one mixes them with folklore, and tries to trace a "
        "strange background of multi-dimensional reality behind the ghoulish "
        "hints of Gothic tales and the wild whispers of the chimney-corner, one "
        "can hardly expect to be wholly free from mental tension.");
    quotes.push_back(
        "I could not help feeling that they were evil things-- mountains of "
        "madness whose farther slopes looked out over some accursed ultimate "
        "abyss.");
    quotes.push_back(
        "That seething, half luminous cloud background held ineffable suggestions "
        "of a vague, ethereal beyondness far more than terrestrially spatial; "
        "and gave appalling reminders of the utter remoteness, separateness, "
        "desolation, and aeon-long death of this untrodden and unfathomed austral "
        "world.");
    quotes.push_back(
        "With five feeble senses we pretend to comprehend the boundlessly complex "
        "cosmos, yet other beings might not only see very differently, but might "
        "see and study whole worlds of matter, energy, and life which lie close "
        "at hand yet can never be detected with the senses we have.");
    quotes.push_back(
        "It is absolutely necessary, for the peace and safety of mankind, that "
        "some of earth's dark, dead corners and unplumbed depths be left alone; "
        "lest sleeping abnormalities wake to resurgent life, and blasphemously "
        "surviving nightmares squirm and splash out of their black lairs to newer "
        "and wider conquests.");
    quotes.push_back(
        "I felt myself on the edge of the world; peering over the rim into a "
        "fathomless chaos of eternal night.");
    quotes.push_back(
        "And where Nyarlathotep went, rest vanished, for the small hours were "
        "rent with the screams of nightmare.");
    quotes.push_back(
        "It was just a colour out of space - a frightful messenger from unformed "
        "realms of infinity beyond all Nature as we know it; from realms whose "
        "mere existence stuns the brain and numbs us with the black extra-cosmic "
        "gulfs it throws open before our frenzied eyes.");
    quotes.push_back(
        "It lumbered slobberingly into sight and gropingly squeezed its "
        "gelatinous green immensity through the black doorway into the tainted "
        "outside air of that poison city of madness.");
    quotes.push_back(
        "The Thing cannot be described - there is no language for such abysms of "
        "shrieking and immemorial lunacy, such eldritch contradictions of all "
        "matter, force, and cosmic order.");
    quotes.push_back(
        "I could tell I was at the gateway of a region half-bewitched through "
        "the piling-up of unbroken time-accumulations; a region where old, "
        "strange things have had a chance to grow and linger because they have "
        "never been stirred up.");
    return quotes[rnd::range(0, quotes.size() - 1)];
}

void draw(const Menu_browser& browser)
{
    TRACE_FUNC_BEGIN;

    Pos pos(MAP_W_HALF, 3);

    TRACE << "Calling clear_window()" << std::endl;
    render::clear_screen();

    render::draw_box(Rect(Pos(0, 0), Pos(SCREEN_W - 1, SCREEN_H - 1)));

    if (config::is_tiles_mode())
    {
        TRACE << "Calling draw_main_menu_logo()" << std::endl;
        render::draw_main_menu_logo(0);
        pos.y += 10;
    }
    else
    {
        std::vector<std::string> logo;

        if (!config::is_tiles_mode())
        {
            logo.push_back("        ___  __                __  __                  ");
            logo.push_back("| |\\  | |   |  )  /\\      /\\  |  )/    /\\  |\\  |  /\\   ");
            logo.push_back("+ | \\ | +-- +--  ____    ____ +-- -   ____ | \\ | ____  ");
            logo.push_back("| |  \\| |   | \\ /    \\  /    \\| \\ \\__/    \\|  \\|/    \\ ");
            logo.push_back("               \\                 \\                      ");
        }

        const int LOGO_X_POS_LEFT = (MAP_W - logo[0].size()) / 2;

        for (const std::string& row : logo)
        {
            pos.x = LOGO_X_POS_LEFT;

            for (const char& glyph : row)
            {
                if (glyph != ' ')
                {
                    Clr clr = clr_green_lgt;

                    clr.g += rnd::range(-50, 100);

                    utils::constr_in_range(0, int(clr.g), 254);

                    render::draw_glyph(glyph, Panel::screen, pos, clr);
                }
                pos.x++;
            }
            pos.y += 1;
        }
        pos.y += 3;
    }

    if (IS_DEBUG_MODE)
    {
        render::draw_text("## DEBUG MODE ##", Panel::screen, Pos(1, 1), clr_yellow);
    }

    TRACE << "Drawing main menu" << std::endl;

    pos.set(48, 13);

    render::draw_text("New journey", Panel::screen, pos,
                      browser.is_at_idx(0) ? clr_menu_highlight : clr_menu_drk);
    pos += 1;

    render::draw_text("Resurrect", Panel::screen, pos,
                      browser.is_at_idx(1) ? clr_menu_highlight : clr_menu_drk);
    pos += 1;

    render::draw_text("Tome of Wisdom", Panel::screen, pos,
                      browser.is_at_idx(2) ? clr_menu_highlight : clr_menu_drk);
    pos += 1;

    render::draw_text("Options", Panel::screen, pos,
                      browser.is_at_idx(3) ? clr_menu_highlight : clr_menu_drk);
    pos += 1;

    render::draw_text("Credits", Panel::screen, pos,
                      browser.is_at_idx(4) ? clr_menu_highlight : clr_menu_drk);
    pos += 1;

    render::draw_text("Graveyard", Panel::screen, pos,
                      browser.is_at_idx(5) ? clr_menu_highlight : clr_menu_drk);
    pos += 1;

    render::draw_text("Escape to reality", Panel::screen, pos,
                      browser.is_at_idx(6) ? clr_menu_highlight : clr_menu_drk);
    pos += 1;

    if (IS_DEBUG_MODE)
    {
        render::draw_text("DEBUG: RUN BOT", Panel::screen, pos,
                          browser.is_at_idx(7) ? clr_menu_highlight : clr_menu_drk);
        pos += 1;
    }


    TRACE << "Drawing quote" << std::endl;
    Clr quote_clr = clr_gray;
    quote_clr.r /= 8;
    quote_clr.g /= 8;
    quote_clr.b /= 8;

    std::vector<std::string> quote_lines;

    int quote_w = 45;

    //Decrease quote width until we find a width that doesn't leave a tiny string on the last line
    //(looks very ugly),
    while (quote_w != 0)
    {
        text_format::line_to_lines(quote, quote_w, quote_lines);

        const size_t MIN_STR_W_LAST_LINE = 20;

        const std::string& last_line = quote_lines.back();

        //Is the length of the current last line at least as long as the minimum required?
        if (last_line.length() >= MIN_STR_W_LAST_LINE)
        {
            break;
        }

        --quote_w;
    }

    if (quote_w > 0)
    {
        pos.set((quote_w / 2) + 2, 15);

        for (const std::string& line : quote_lines)
        {
            render::draw_text_centered(line, Panel::screen, pos, quote_clr);
            ++pos.y;
        }
    }

    render::draw_text_centered(
        game_version_str + " - " + __DATE__ + " (c) 2011-2015 Martin Tornqvist",
        Panel::screen, Pos(MAP_W_HALF, SCREEN_H - 1), clr_gray_drk);

    render::update_screen();

    TRACE_FUNC_END;
}

} //namespace

Game_entry_mode run(bool& quit, int& intro_mus_channel)
{
    TRACE_FUNC_BEGIN;

    quote = hpl_quote();

    Menu_browser browser(IS_DEBUG_MODE ? 8 : 7);

    intro_mus_channel = audio::play(Sfx_id::mus_cthulhiana_Madness);

    draw(browser);

    while (true)
    {
        const Menu_action action = menu_input:: action(browser, Menu_input_mode::scroll);

        switch (action)
        {
        case Menu_action::moved:
            draw(browser);
            break;

        case Menu_action::esc:
            quit = true;
            TRACE_FUNC_END;
            return Game_entry_mode::new_game;

        case Menu_action::space:
            break;

        case Menu_action::selected:
        case Menu_action::selected_shift:
            if (browser.is_at_idx(0))
            {
                TRACE_FUNC_END;
                return Game_entry_mode::new_game;
            }

            if (browser.is_at_idx(1))
            {
                if (save_handling::is_save_available())
                {
                    save_handling::load_game();
                    map_travel::go_to_nxt();
                    TRACE_FUNC_END;
                    return Game_entry_mode::load_game;
                }
                else //No save available
                {
                    popup::show_msg("No saved game found", false);
                    draw(browser);
                }
            }

            if (browser.is_at_idx(2))
            {
                manual::run();
                draw(browser);
            }

            if (browser.is_at_idx(3))
            {
                config::run_options_menu();
                draw(browser);
            }

            if (browser.is_at_idx(4))
            {
                credits::run();
                draw(browser);
            }

            if (browser.is_at_idx(5))
            {
                highscore::run_highscore_screen();
                draw(browser);
            }

            if (browser.is_at_idx(6))
            {
                quit    = true;
                TRACE_FUNC_END;
                return Game_entry_mode::new_game;
            }

            if (IS_DEBUG_MODE)
            {
                if (browser.is_at_idx(7))
                {
                    config::toggle_bot_playing();
                    TRACE_FUNC_END;
                    return Game_entry_mode::new_game;
                }
            }
            break;
        }
    }

    TRACE_FUNC_END;
    return Game_entry_mode::new_game;
}

} //Main_menu
