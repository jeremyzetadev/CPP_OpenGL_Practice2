#include "camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <iostream>

Camera::Camera(){
    // assume we are placed at the origin
    myEye = glm::vec3(0.0f, 0.0f, 0.0f);
    // Assume we are looking out into the world
    mViewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    // Assume we start on a perfect plane
    mUpVector = glm::vec3(0.0, 1.0f, 0.0f);
}

glm::mat4 Camera::GetViewMatrix() const{
    return glm::lookAt(myEye, myEye+mViewDirection, mUpVector);
}

void Camera::MouseLook(int mouseX, int mouseY){
    static bool firstLook = true;
    glm::vec2 currentMouse = glm::vec2(mouseX, mouseY);
    if(firstLook){
        mOldMousePosition = currentMouse;
        firstLook=false;
    }

    mViewDirection = glm::rotate(mViewDirection, glm::radians((float)mouseX), mUpVector);
    glm::vec2 mouseDelta = mOldMousePosition - currentMouse;
}

void Camera::MoveForward(float speed){
    myEye += (mViewDirection*speed);
}

void Camera::MoveBackward(float speed){
    myEye -= (mViewDirection*speed);
}

void Camera::MoveLeft(float speed){
    // mViewDirection.x +=speed;
    glm::vec3 rightVector = glm::cross(mViewDirection, mUpVector);
    myEye += rightVector*speed;
}

void Camera::MoveRight(float speed){
    // mViewDirection.x -=speed;
    glm::vec3 rightVector = glm::cross(mViewDirection, mUpVector);
    myEye -= rightVector*speed;
}
