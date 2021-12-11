#ifndef GRAMMAR_H
#define GRAMMAR_H
#include<scene/turtle.h>
void move_forward(turtle& t)
{
    t.pos += glm::vec3(cos(t.orientation * RAD) * 5, 0 , sin(t.orientation * RAD) * 5);
}
void rotate_pos(turtle& t)
{
    t.orientation += t.turning_angle;
}
void rotate_negative(turtle& t)
{
    t.orientation -= t.turning_angle;
}
#endif // GRAMMAR_H
