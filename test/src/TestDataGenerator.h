/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#ifndef TEST_DATA_GENERATOR_H
#define TEST_DATA_GENERATOR_H
#include <vector>
#include <cstdlib>
#include <string>

template<typename T>
struct TestDataGenerator
{
    static
    T* create(const int size, std::string category, const unsigned int seed = 1)
    {
        std::srand(seed);
        T* container = new T[size];

        if(category == "sequential")
        {
            for(int i = 0; i < size; i++)
            {
                container[i] = (T)i;
            }
        }else if(category == "random"){
            for(int i = 0; i < size; i++)
            {
                container[i] = (double)std::rand()/RAND_MAX*size;
            }
        }else if(category == "same"){
            for(int i = 0; i < size; i++)
            {
                container[i] = (T)size/3;
            }
        }else if(category == "sequential_vector"){
            for(int i = 0; i < size/3; i++)
            {
                container[3*i]   = (T)i;
                container[3*i+1] = (T)i;
                container[3*i+2] = (T)i;
            }
        }

        return container;
    }
};
#endif
