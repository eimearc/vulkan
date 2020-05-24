#pragma once

#include <string>
#include <iostream>
#include <fstream>

class Bench
{
    public:
    Bench()=default;
    ~Bench()=default;

    void open(std::string file);
    void numCubes(size_t _num);
    void numThreads(size_t num);
    void frame(float _time);
    void updateVBO(float _time);
    void record();
    void close();

    private:
    std::fstream m_file;
    size_t m_numCubes=0;
    size_t m_numThreads=0;
    float m_frame=0.0f;
    float m_updateVBO=0.0f;
};