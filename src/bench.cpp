#include "bench.h"

void Bench::open(std::string file, bool overwrite)
{
    std::ios_base::openmode mode = std::fstream::out;
    if (overwrite) std::cout << "Overwriting " << file << std::endl;
    else mode |= std::fstream::app;
    m_file.open(file, mode);
    if (overwrite)
    {
        m_file<<"cubes,";
        m_file<<"threads,";
        m_file<<"frame,";
        m_file<<"updateVBO\n";
    }
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

void Bench::start()
{
    m_start = std::chrono::high_resolution_clock::now();
}

void Bench::record()
{
    m_file<<m_numCubes<<",";
    m_file<<m_numThreads<<",";
    m_file<<m_frame<<",";
    m_file<<m_updateVBO;
    m_file<<"\n";
}