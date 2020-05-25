#pragma once

#include <string>
#include <iostream>
#include <fstream>

class Bench
{
    public:
    Bench()=default;
    ~Bench();

    void open(std::string file, bool overwrite=false);
    void numCubes(size_t _num);
    void numThreads(size_t num);

    void start();
    inline void frameTime()
    {
        float d = duration();
        m_frame = d;
    }
    inline void updateVBOTime()
    {
        float d = duration();
        m_updateVBO = d;
    }
    void record();
    void close();

    private:
    std::fstream m_file;
    size_t m_numCubes=0;
    size_t m_numThreads=0;
    std::chrono::steady_clock::time_point m_start;
    float m_frame=0.0f;
    float m_updateVBO=0.0f;

    float duration()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<float,std::chrono::milliseconds::period>(end - m_start).count();
        return duration;
    }
};