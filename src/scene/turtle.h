#ifndef TURTLE_H
#define TURTLE_H
#include <glm/glm.hpp>
#include<QHash>
#include<QChar>
#define RAD 3.14/180.0
class turtle
{
public:
    turtle();
    turtle(glm::vec3 pos, double orientation);  // init with given pos and angle
    glm::vec3 pos;
    double orientation;  // in angle
    int depth;  // recursion depth

    void update(glm::vec3 p, double o); // update position and orientation

    double turning_angle = 30.0; // default turning angle
    float walk_length = 15.f;     // default walk length for each moving forward

};

#endif // TURTLE_H
