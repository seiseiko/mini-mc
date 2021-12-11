#ifndef BIOME_H
#define BIOME_H

#include "terrain.h"
#include "noise.h"

#include <iostream>

BlockType getBiomeBlockType(int x, int y, int z, int upperBound);


BlockType getBiomeBlockType(int x, int y, int z, int upperBound){
    float moisture = fbm(x,z,3.f,1.2f,4);
    std::cout<<moisture<<std::endl;
}


#endif // BIOME_H
