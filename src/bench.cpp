#include "bench.h"

void Bench::open(std::string file)
{
    m_file.open(file, std::fstream::out | std::fstream::app); // TODO: Handle appends.
    m_file<<"numCubes,";
    m_file<<"numThreads,";
    m_file<<"frame,";
    m_file<<"updateVBO";
    m_file<<"\n";
}

void Bench::close()
{
    m_file.close();
}

void Bench::numCubes(size_t _num)
{
    m_numCubes = _num;
}

void Bench::numThreads(size_t _num)
{
    m_numThreads = _num;
}

void Bench::frame(float _time)
{
    m_frame = _time;
}

void Bench::updateVBO(float _time)
{
    m_updateVBO = _time;
}

void Bench::record()
{
    std::cout << "Recording\n";
    m_file<<m_numCubes<<",";
    m_file<<m_numThreads<<",";
    m_file<<m_frame<<",";
    m_file<<m_updateVBO;
    m_file<<"\n";
}