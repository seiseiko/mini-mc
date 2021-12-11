
#ifndef LSYSTEM_H
#define LSYSTEM_H
#include<QChar>
#include<QHash>
#include<QString>
#include<QMap>
#include<QStack>
#include<scene/turtle.h>
#include<scene/terrain.h>

class Lsystem;

// type for our grammar rules
typedef void (Lsystem::*Rule)(void);


class Lsystem
{
public:

    Lsystem(Terrain* terrain, OpenGLContext* context, glm::vec3 pos);
    QStack<turtle> turtles;
    QHash<QChar, Rule> rulelist;      // operation interpreation
    QHash<QString, QString> expansion;  // grammar rules
    QString axiom;


    // parse system
    void lsystemParse(int iterations);
    turtle  curr;            // the copy of current turtle
    Terrain* terrain;       // pointer to the terrain we are drawing
    OpenGLContext* context; // context to draw
    bool branch;            // boolean : if the turtle drawing is a branch
    bool main_wid;          // the flag of whether the mainstream is wider.
    int river_wid;          // half of the river width

    // function
    void carve_away(glm::vec3 pos);  // carve the terrain nearby a given position
    void clean(glm::vec3 pos);       // clean up all the upper part of a given position

    // rule funcs
    void move_forward();    // 'F'
    void rotate_pos();      // '-'
    void rotate_negative(); // '+'
    void left();   // '['
    void right();  // ']'

};

#endif // LSYSTEM_H
