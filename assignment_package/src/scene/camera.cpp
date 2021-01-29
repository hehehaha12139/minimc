#include "scene/camera.h"
#include "utils.h"
#include <la.h>
#include <iostream>


Camera::Camera():
    Camera(400, 400)
{
    recomputeAttributes();
}

Camera::Camera(unsigned int w, unsigned int h):
    Camera(w, h, glm::vec3(0,0,10), glm::vec3(0,0,0), glm::vec3(0,1,0))
{}

Camera::Camera(unsigned int w, unsigned int h, const glm::vec3& e, const glm::vec3& r, const glm::vec3 &worldUp):
    fovy(45),
    width(w),
    height(h),
    near_clip(0.1f),
    far_clip(1000),
    pitch(0),
    yaw(-90),
    eye(e),
    ref(r),
    world_up(worldUp)
{
    recomputeAttributes();
}

Camera::Camera(const Camera &c):
    fovy(c.fovy),
    width(c.width),
    height(c.height),
    near_clip(c.near_clip),
    far_clip(c.far_clip),
    pitch(c.pitch),
    yaw(c.yaw),
    aspect(c.aspect),
    eye(c.eye),
    ref(c.ref),
    look(c.look),
    up(c.up),
    right(c.right),
    world_up(c.world_up),
    V(c.V),
    H(c.H)
{}

glm::mat4 Camera::getViewProj()
{
    return glm::perspective(glm::radians(fovy), (float)width / (float)height, near_clip, far_clip) * glm::lookAt(eye, ref, up);
}

void Camera::recomputeAttributes()
{
    glm::vec3 forward(cos(glm::radians(pitch)) * cos(glm::radians(yaw)),
                      sin(glm::radians(pitch)),
                      cos(glm::radians(pitch)) * sin(glm::radians(yaw)));

    // Normalizes the vectors to avoid the slower movement
    look = glm::normalize(forward);
    ref = eye + look;
    right = glm::normalize(glm::cross(look, world_up));
    up = glm::normalize(glm::cross(right, look));

    float tan_fovy = tan(glm::radians(fovy / 2.f));
    float len = glm::length(ref - eye);
    V = up * len * tan_fovy;
    H = right * len * aspect * tan_fovy;
}

void Camera::RotateAboutUp(float deg)
{
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(deg), up);
    ref = ref - eye;
    ref = glm::vec3(rotation * glm::vec4(ref, 1));
    ref = ref + eye;
    recomputeAttributes();
}

void Camera::RotateAboutRight(float deg)
{
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(deg), right);
    ref = ref - eye;
    ref = glm::vec3(rotation * glm::vec4(ref, 1));
    ref = ref + eye;
    recomputeAttributes();
}

void Camera::TranslateAlongLook(float amt)
{
    glm::vec3 translation = look * amt;
    eye += translation;
    ref += translation;
}

void Camera::TranslateAlongRight(float amt)
{
    glm::vec3 translation = right * amt;
    eye += translation;
    ref += translation;
}

void Camera::TranslateAlongUp(float amt)
{
    glm::vec3 translation = up * amt;
    eye += translation;
    ref += translation;
}
