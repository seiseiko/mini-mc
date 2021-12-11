#include "turtle.h"
#include <cmath>
turtle::turtle():
    pos(glm::vec3(20.f, 128.f, 20.f)),
    orientation(0),
    depth(0)
{
}



turtle::turtle(glm::vec3 pos, double orientation):
    pos(pos), orientation(orientation), depth(0)
{
    pos[1] = 128.f;
}


void turtle::update(glm::vec3 p, double o)
{
    this->pos = p;
    this->orientation = o;
    depth ++;
}


