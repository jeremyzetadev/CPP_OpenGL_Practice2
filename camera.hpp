#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

class Camera{
    public:
        Camera();
        // The ultimate view matrix we will produce
        glm::mat4 GetViewMatrix() const;

        void MoveForward(float speed);
        void MoveBackward(float speed);
        void MoveLeft(float speed);
        void MoveRight(float speed);

    private:
        glm::vec3 myEye;
        glm::vec3 mViewDirection;
        glm::vec3 mUpVector;
};

#endif
