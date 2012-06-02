#ifndef SPAWN_COUNTER_H
#define SPAWN_COUNTER_H

//Object counter using Curiously Recurring Template Pattern (CRTP).
//let objects to be counted inherit from SpawnCounter, instanced
//as their own type.

template<class T>
class SpawnCounter
{
    public:
        SpawnCounter() {
            m_spawned++;
            m_live++;
        }

        ~SpawnCounter() {
            m_live--;
            m_destroyed++;
        }

        static int getSpawned()    {return m_spawned;}
        static int getLive()       {return m_live;}
        static int getDestroyed()  {return m_destroyed;}

    private:
        static int m_spawned;
        static int m_live;
        static int m_destroyed;
};

template<class T> int SpawnCounter<T>::m_spawned(0);
template<class T> int SpawnCounter<T>::m_live(0);
template<class T> int SpawnCounter<T>::m_destroyed(0);

#endif
