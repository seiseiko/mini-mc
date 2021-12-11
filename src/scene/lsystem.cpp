#include "lsystem.h"
#include "scene/grammar.h"
#include <iostream>
Lsystem::Lsystem(Terrain* terrain, OpenGLContext* context, glm::vec3 pos):
    terrain(terrain), context(context)
{
    // flags
    branch = false;
    main_wid = false;

    // rule list mapping
    rulelist['F'] = &Lsystem::move_forward;
    rulelist['-'] = &Lsystem::rotate_negative;
    rulelist['+'] = &Lsystem::rotate_pos;
    rulelist['['] = &Lsystem::left;
    rulelist[']'] = &Lsystem::right;
    turtles.push_back(turtle(pos,0));
}

void Lsystem::lsystemParse(int iterations)
{


    // iterate and come out complete axiom
    QString new_axiom = "";
    for(int i = 0; i < iterations; i++)
    {
        for(int j = 0; j < axiom.length(); j++)
        {
            QString symbol = QString(axiom[j]);
            QString new_syms;
            auto  i = expansion.find(symbol);
            if (i != expansion.end() && i.key() == symbol)
            {

                if(symbol=="F")
                {
                    int p = rand()%100;
                    // 40% possibility
                    if( p<= 40)
                    {
                        new_syms = expansion[symbol];
                    }
                    // 20% possibility
                    else if(p<=60)
                    {
                        new_syms = expansion[symbol+"1"];
                    }
                    // 20% possibility
                    else if(p<=80)
                    {
                        new_syms = expansion[symbol+"2"];
                    }
                    // 20% possibility
                    else
                    {

                        new_syms = expansion[symbol+"3"];
                    }
                }
                else
                {
                    new_syms = expansion[symbol];
                }

            }
            else
            {
                new_syms = QString(symbol);
            }
            new_axiom += new_syms;
        }
        axiom = new_axiom;
    }


    curr = turtles.top();
    branch = false;
    for(int i = 0; i < axiom.length(); i++)
    {
        QChar symbol = axiom[i];
        Rule r = rulelist[symbol];
        (this->*r)();
    }

    std::cout<<"lsystem end" << std::endl;
}


// carve and clean up

// clean the upper blocks of a given pos
void Lsystem::clean(glm::vec3 pos)
{
    int x = pos[0];
    int y = pos[1];
    int z = pos[2];

    for(int j = y +1; j<256; j++)
    {
        BlockType &t = terrain->getBlockAt2(x, j, z);
        t = EMPTY;
    }
}

void Lsystem::carve_away(glm::vec3 pos)
{

    int x = pos[0];
    int y = pos[1];
    int z = pos[2];

    // gradually carve surronding
    int left = z - 1;
    int right = z + 1;
    int front = x - 1;
    int back = x + 1;
    int height;

    // carving surronding 7x7x7
    for(int i = 0; i < 7; i++)
    {
        height = 127 + i;
        for(int i = front; i <= back; i++)
        {
            // left block
            BlockType &t_l = terrain->getBlockAt2(i, height, left);
            if( t_l != EMPTY && t_l !=WATER)
            {
                t_l = GRASS;
                clean(glm::vec3(i,height+1,left));
            }
            // right block
            BlockType &t_r = terrain->getBlockAt2(i, height, right);
            if( t_r != EMPTY && t_r !=WATER)
            {
                t_r = GRASS;
                clean(glm::vec3(i,height+1,right));
            }
        }
        for(int j = left; j <=right; j++)
        {
            // back and front
            BlockType &t_b = terrain->getBlockAt2(back, height, j);
            BlockType &t_f = terrain->getBlockAt2(front, height, j);
            if( t_b != EMPTY && t_b !=WATER)
            {
                t_b = GRASS;
                clean(glm::vec3(back,height+1,j));
            }
            if( t_f != EMPTY && t_f !=WATER)
            {
                t_f = GRASS;
                clean(glm::vec3(front,height+1,j));
            }
        }
        left--;
        right++;
        front--;
        back++;
    }

}


// grammar related
void Lsystem::move_forward()
{

    // start point
    glm::vec3 temp_pos = curr.pos;
    // end point
    curr.pos += glm::vec3(round(cos(curr.orientation * RAD) * curr.walk_length), 0 , round(sin(curr.orientation * RAD) * curr.walk_length));

    float diff_z = ceil(curr.pos[2]) - floor(temp_pos[2]);
    float diff_x = ceil(curr.pos[0]) - floor(temp_pos[0]);
    float step_z, step_x;
    int iter;

    // since we are not sure the step is positive/negtaive,
    // we calcuate the iteration time and the step of x, z here
    if(abs(diff_z) >= abs(diff_x))
    {
        step_z = diff_z / abs(diff_z);
        step_x = diff_x / abs(diff_z);
        iter =  abs(diff_z);
    }
    else
    {
        step_z = diff_z / abs(diff_x);
        step_x = diff_x / abs(diff_x);
        iter =  abs(diff_x);
    }

    int i = 0;
    glm::vec3 pos;
    while(iter--)
    {

        // calcuate the position of the center river block
        pos = glm::vec3(floor(temp_pos[0]) + step_x * i,
                        curr.pos[1],
                        floor(temp_pos[2]) + step_z * i);

        glm::vec3 temp_pos;
        int wid = river_wid;

        // if the mainstream is wider
        if((main_wid==true) && (branch == false))
        {
             wid += 5;
        }

        // normal width
        for(int i = ceil(-wid/2.0); i <= floor(wid/2.0); i++)
        {
            temp_pos = glm::vec3(pos[0],
                            pos[1],
                            pos[2]+i);
            terrain->setBlockAt(temp_pos[0],temp_pos[1],temp_pos[2],WATER,context);
            clean(temp_pos);

            //  only carve away the edge
            if(i == ceil(-wid/2.0)||i == floor(wid/2.0))
            {
                carve_away(temp_pos);
            }
        }
        i++;
    }
}
void Lsystem::rotate_pos()
{
    curr.orientation += curr.turning_angle;
}
void Lsystem::rotate_negative()
{
    curr.orientation -= curr.turning_angle;
}
void Lsystem::left()
{
    turtles.push_back(curr);
    branch = true;
}
void Lsystem::right()
{
    curr = turtles.pop();
    // judge if we are now in main stream
    if(turtles.size()<=1)
    {
        branch = false;
    }
}



