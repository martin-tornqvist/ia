#include <string>
#include <fstream>

#include "Colors.h"

extern SDL_Event event;
extern Uint8 *keystates;

void intro_text_run()
{
//    bool read_next = false;
//
//    std::string  str;
//    char*   file;
//
//    file = "intro_text";
//
//    ifstream f (file);
//
//    //drw=TTF_RenderText_Solid(font_UI_medium,"Press <n> to continue",clrWhite);
//    //applySurface(SCREEN_WIDTH/2 - string_get_width("Press <n> to continue")/2, SCREEN_HEIGHT - 80 , drw, screen);
//    //SDL_Flip(screen);
//    //SDL_FreeSurface(drw);
//
//    for (int i=0; i < 12; i++) {
//        getline(f,str);
//        drw=TTF_RenderText_Solid(font, str.data() ,clrInterfaceBlood);
//        applySurface(80, 144 + i*22 , drw, screen);
//        SDL_FreeSurface(drw);
//    }
//    SDL_Flip(screen);
//
//    while (read_next == false) {
//        while(SDL_PollEvent(&event)) {
//            switch (event.type) {
//                case SDL_KEYDOWN:
//                    if (keystates[SDLK_n])
//                        read_next = true;
//                    break;
//            }
//        }
//    }
//    read_next = false;
//    drawClearArea(0,0,SCREEN_WIDTH,SCREEN_HEIGHT-128);
//
//    for (int i=0; i < 10; i++) {
//        getline(f,str);
//        drw=TTF_RenderText_Solid(font, str.data() ,clrInterfaceBlood);
//        applySurface(80, 144 + i*22 , drw, screen);
//        SDL_FreeSurface(drw);
//    }
//    SDL_Flip(screen);
//
//    while (read_next == false) {
//        while(SDL_PollEvent(&event)) {
//            switch (event.type) {
//                case SDL_KEYDOWN:
//                    if (keystates[SDLK_n])
//                        read_next = true;
//                    break;
//            }
//        }
//    }
//    read_next = false;
//    drawClearArea(0,0,SCREEN_WIDTH,SCREEN_HEIGHT-128);
//
//    for (int i=0; i < 14; i++) {
//        getline(f,str);
//        drw=TTF_RenderText_Solid(font, str.data() ,clrInterfaceBlood);
//        applySurface(80, 144 + i*22 , drw, screen);
//        SDL_FreeSurface(drw);
//    }
//    SDL_Flip(screen);
//
//    while (read_next == false) {
//        while(SDL_PollEvent(&event)) {
//            switch (event.type) {
//                case SDL_KEYDOWN:
//                    if (keystates[SDLK_n])
//                        read_next = true;
//                    break;
//            }
//        }
//    }
//    read_next = false;
//    drawClearArea(0,0,SCREEN_WIDTH,SCREEN_HEIGHT-128);
//
//    for (int i=0; i < 10; i++) {
//        getline(f,str);
//        drw=TTF_RenderText_Solid(font, str.data() ,clrInterfaceBlood);
//        applySurface(80, 144 + i*22 , drw, screen);
//        SDL_FreeSurface(drw);
//    }
//    SDL_Flip(screen);
//
//    while (read_next == false) {
//        while(SDL_PollEvent(&event)) {
//            switch (event.type) {
//                case SDL_KEYDOWN:
//                    if (keystates[SDLK_n])
//                        read_next = true;
//                    break;
//            }
//        }
//    }
//    read_next = false;
//    drawClearArea(0,0,SCREEN_WIDTH,SCREEN_HEIGHT-128);
//
//    for (int i=0; i < 7; i++) {
//        getline(f,str);
//        drw=TTF_RenderText_Solid(font, str.data() ,clrInterfaceBlood);
//        applySurface(80, 144 + i*22 , drw, screen);
//        SDL_FreeSurface(drw);
//    }
//    SDL_Flip(screen);
//
//    while (read_next == false) {
//        while(SDL_PollEvent(&event)) {
//            switch (event.type) {
//                case SDL_KEYDOWN:
//                    if (keystates[SDLK_n])
//                        read_next = true;
//                    break;
//            }
//        }
//    }
//    read_next = false;
//    drawClearArea(0,0,SCREEN_WIDTH,SCREEN_HEIGHT-128);
//
//    for (int i=0; i < 13; i++) {
//        getline(f,str);
//        drw=TTF_RenderText_Solid(font, str.data() ,clrInterfaceBlood);
//        applySurface(80, 144 + i*22 , drw, screen);
//        SDL_FreeSurface(drw);
//    }
//    SDL_Flip(screen);
//
//    f.close();
//
//    bool proceed = false;
//    while (proceed == false) {
//        while(SDL_PollEvent(&event)) {
//            switch (event.type) {
//                case SDL_KEYDOWN:
//                    if (keystates[SDLK_n])
//                        proceed = true;
//                    break;
//            }
//        }
//    }
}

