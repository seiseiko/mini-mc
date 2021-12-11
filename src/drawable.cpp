#include "drawable.h"
#include <la.h>

Drawable::Drawable(OpenGLContext* context)
    : bufIdx(), bufPos(), bufNor(), bufCol(), bufUV(),bufIdxTrans(),bufPosTrans(),bufPow(),
      idxBound(false), posBound(false), norBound(false), colBound(false),uvBound(false),idxTransBound(false),posTransBound(false),powBound(false),
      context(context)
{}

Drawable::~Drawable()
{}


void Drawable::destroy()
{
    context->glDeleteBuffers(1, &bufIdx);
    context->glDeleteBuffers(1, &bufPos);
    context->glDeleteBuffers(1, &bufNor);
    context->glDeleteBuffers(1, &bufCol);
    context->glDeleteBuffers(1, &bufVert);
    context->glDeleteBuffers(1, &bufUV);
    context->glDeleteBuffers(1, &bufIdxTrans);
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount()
{
    return count;
}

int Drawable::elemCountTrans()
{
    return countTrans;
}

void Drawable::generateIdx()
{
    idxBound = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    context->glGenBuffers(1, &bufIdx);
}

void Drawable::generatePos()
{
    posBound = true;
    // Create a VBO on our GPU and store its handle in bufPos
    context->glGenBuffers(1, &bufPos);
}

void Drawable::generateNor()
{
    norBound = true;
    // Create a VBO on our GPU and store its handle in bufNor
    context->glGenBuffers(1, &bufNor);
}

void Drawable::generateCol()
{
    colBound = true;
    // Create a VBO on our GPU and store its handle in bufCol
    context->glGenBuffers(1, &bufCol);
}
void Drawable::generateVert()
{
    vertBound = true;
    // Create a VBO on our GPU and store its handle in bufVert
    context->glGenBuffers(1, &bufVert);
}

void Drawable::generateUV(){
    uvBound = true;
    context->glGenBuffers(1,&bufUV);
}

void Drawable::generateIdxTrans(){
    idxTransBound = true;
    context->glGenBuffers(1,&bufIdxTrans);
}

void Drawable::generatePosTrans(){
    posTransBound = true;
    context -> glGenBuffers(1,&bufPosTrans);
}

void Drawable::generatePow(){
    powBound = true;
    context -> glGenBuffers(1,&bufPow);

}

bool Drawable::bindVert()
{
    if(vertBound) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufVert);
    }
    return vertBound;
}
bool Drawable::bindIdx()
{
    if(idxBound) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    }
    return idxBound;
}

bool Drawable::bindPos()
{
    if(posBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    }
    return posBound;
}

bool Drawable::bindNor()
{
    if(norBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    }
    return norBound;
}

bool Drawable::bindCol()
{
    if(colBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufCol);
    }
    return colBound;
}

bool Drawable::bindUV(){
    if(uvBound){
        context->glBindBuffer(GL_ARRAY_BUFFER,bufUV);
    }
    return uvBound;
}

bool Drawable::bindPosTrans(){
    if(posTransBound){
        context->glBindBuffer(GL_ARRAY_BUFFER,bufPosTrans);
    }
    return posTransBound;
}

bool Drawable::bindIdxTrans(){
    if(idxTransBound){
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bufIdxTrans);
    }
    return idxTransBound;
}

bool Drawable::bindPow(){
    if(powBound){
        context->glBindBuffer(GL_ARRAY_BUFFER,bufPow);
    }
    return powBound;
}
