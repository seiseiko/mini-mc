#include <mygl.h>
// shadow mapping
void MyGL::createShadowText(){

    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
     glGenFramebuffers(1, &m_depthBuffer);

     // Depth texture. Slower than a depth buffer, but you can sample it later in your shader

     glGenTextures(1, &m_depthTexture);
     glBindTexture(GL_TEXTURE_2D, m_depthTexture);
     glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, shadow_width, shadow_height, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

     glBindFramebuffer(GL_FRAMEBUFFER, m_depthTexture);
     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
     glDrawBuffer(GL_NONE); // No color buffer is drawn to.
     glReadBuffer(GL_NONE);
     glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

void MyGL::calculate_depthMVP(glm::mat4& mat)
{
    glm::vec3 lightInvDir = glm::vec3(0.2, 0.5, - 0.7);

    // Compute the MVP matrix from the light's point of view
    float near_plane = 1.0f, far_plane = 100.f;
    glm::mat4 depthProjectionMatrix = glm::ortho<float>(-100.f, 100.f, -100.f, 100.f, near_plane, far_plane);
    glm::mat4 depthViewMatrix = glm::lookAt( 20.f * lightInvDir + mp_camera->eye, mp_camera->eye, glm::vec3(0, 1, 0));
    glm::mat4 depthModelMatrix = glm::mat4(1.0);

    mat = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;

}


void MyGL::bind_buffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    glViewport(0, 0, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
}
void MyGL::bind_shadow_buffer()
{
    glViewport(0, 0, shadow_width, shadow_height);
    glBindFramebuffer(GL_FRAMEBUFFER, m_depthBuffer);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
}

