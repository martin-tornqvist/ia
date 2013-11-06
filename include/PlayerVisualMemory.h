#ifndef PLAYER_VISUAL_MEMORY_H
#define PLAYER_VISUAL_MEMORY_H

class Engine;

class PlayerVisualMemory
{
    public:
        PlayerVisualMemory(Engine* engine) {eng = engine;}

        //Makes a copy of the renderers current array
        void updateVisualMemory();

    private:
        Engine* eng;
};


#endif
