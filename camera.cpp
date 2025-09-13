#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(){
    // assume we are placed at the origin
    myEye = glm::vec3(0.0f, 0.0f, 0.0f);
    // Assume we are looking out into the world
    mViewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    // Assume we start on a perfect plane
    mUpVector = glm::vec3(0.0, 1.0f, 0.0f);
}

glm::mat4 Camera::GetViewMatrix() const{
    return glm::lookAt(myEye, mViewDirection, mUpVector);
}

void Camera::MoveForward(float speed){
    myEye.z -= speed;
}

void Camera::MoveBackward(float speed){
    myEye.z += speed;
}

void Camera::MoveLeft(float speed){
    mViewDirection.x +=speed;
}

void Camera::MoveRight(float speed){
    mViewDirection.x -=speed;
}
