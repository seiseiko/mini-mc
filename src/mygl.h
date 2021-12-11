#ifndef MYGL_H
#define MYGL_H

#include <openglcontext.h>
#include <utils.h>
#include <shaderprogram.h>
#include <scene/cube.h>
#include <scene/worldaxes.h>
#include "camera.h"
#include <QMouseEvent>

#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <smartpointerhelp.h>
#include <set>
#include <scene/lsystem.h>
#include <texture.h>
#include <stdlib.h>

#include <QRgb>


class MyGL : public OpenGLContext
{
    Q_OBJECT
private:
    uPtr<Cube> mp_geomCube;// The instance of a unit cube we can use to render any cube. Should NOT be used in final version of your project.
    uPtr<WorldAxes> mp_worldAxes; // A wireframe representation of the world axes. It is hard-coded to sit centered at (32, 128, 32).
    uPtr<ShaderProgram> mp_progLambert;// A shader program that uses lambertian reflection
    uPtr<ShaderProgram> mp_progFlat;// A shader program that uses "flat" reflection (no shadowing at all)
    uPtr<ShaderProgram> mp_progShadow;
    GLuint vao; // A handle for our vertex array object. This will store the VBOs created in our geometry classes.
                // Don't worry too much about this. Just know it is necessary in order to render geometry.

    uPtr<Camera> mp_camera;
    uPtr<Terrain> mp_terrain;
    std::set<Chunk*> created;
    /// Timer linked to timerUpdate(). Fires approx. 60 times per second
    QTimer timer;


    uPtr<Texture> mp_texture_all;
    int m_time;

    // mm3 ： shadow mapping frame buffer, handle for texture, shadow map size
    GLuint m_depthBuffer;
    GLuint m_depthTexture;
    int shadow_width = 4096;
    int shadow_height = 4096;
    void calculate_depthMVP(glm::mat4& mat);
    void bind_buffer();
    void bind_shadow_buffer();
    BlockType rgb2type(glm::vec3 c);

    // mm2: L-system river
    Lsystem river_linear;
    Lsystem river_delta;

    float min = 0.f;
    float max = 0.f;
    int max_bound = 0;

    void MoveMouseToCenter(); // Forces the mouse position to the screen's center. You should call this
                              // from within a mouse move event after reading the mouse movement so that
                              // your mouse stays within the screen bounds and is always read.


public:
    explicit MyGL(QWidget *parent = 0);
    ~MyGL();

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void CreateTestScene();
    void GLDrawScene();
    void removeBlock();
    void addBlock(BlockType bt);
    void updateTerrainSize();
    void setBlockColumn(int x, int z);
    void createTextureAll();
    // mm3
    void heightMap();
    void deform(QRgb* color, glm::vec3 pos);
    void createShadowText();
    BlockType getBiomeBlockType(int x, int y, int z, int upperBound);

    void createCave(int steps,int start_x, int start_y,int start_z);
protected:
    void keyPressEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent *m);

private slots:
    /// Slot that gets called ~60 times per second
    void timerUpdate();
};


#endif // MYGL_H
