#include "Camera.hpp"
#include "glm/glm.hpp" //core glm functionality
#include "glm/gtc/matrix_transform.hpp" //glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp" //glm extension for computing inverse matrices
#include "glm/gtc/type_ptr.hpp" //glm extension for accessing the internal data structure of glm types

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        this->cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction)
        {
            case gps::MOVE_FORWARD:
                this->cameraPosition += speed * cameraFrontDirection;
                break;
            case gps::MOVE_BACKWARD:
                this->cameraPosition -= speed * cameraFrontDirection;
                break;
            case gps::MOVE_RIGHT:
                this->cameraPosition += speed * cameraRightDirection;
                break;
            case gps::MOVE_LEFT:
                this->cameraPosition -= speed * cameraRightDirection;
                break;
            case gps::MOVE_UP:
                this->cameraPosition += speed * cameraUpDirection;
                break;
            case gps::MOVE_DOWN:
                this->cameraPosition -= speed * cameraUpDirection;
                break;
            default:
                break;
        }
        this->cameraTarget = cameraPosition + cameraFrontDirection;
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) 
    {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->cameraFrontDirection = glm::normalize(front);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        this->cameraTarget = cameraPosition + cameraFrontDirection;
    }

    float Camera::getDistance(glm::vec3 point)
    {
        glm::vec3 point_direction = point - cameraPosition;

        float dot_product = glm::dot(point_direction, cameraUpDirection);
        glm::vec3 projection = dot_product * cameraUpDirection;
        glm::vec3 distance_vector = point_direction - projection;

        return glm::length(distance_vector);
    }

    void Camera::set(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) 
    {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        this->cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }

    void Camera::print()
    {
        printf("%ff, %ff, %ff\n", cameraPosition.x, cameraPosition.y, cameraPosition.z);
        printf("%ff, %ff, %ff,\n", cameraTarget.x, cameraTarget.y, cameraTarget.z);
        printf("%f %f %f------\n", cameraUpDirection.x, cameraUpDirection.y, cameraUpDirection.z);
    }
}