#include "Camera.hpp"

namespace gps {

	//Camera constructor
	Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
		//TODO
		this->cameraPosition = cameraPosition;
		this->cameraTarget = cameraTarget;
		this->cameraUpDirection = cameraUp;

		this->cameraUpDirection = glm::vec3(0.0f, 1.0f, 0.0f);
		this->cameraRightDirection = glm::vec3(1.0f, 0.0f, 0.0f);
		this->cameraFrontDirection = glm::vec3(0.0f, 0.0f, -1.0f);

	}

	//return the view matrix, using the glm::lookAt() function
	glm::mat4 Camera::getViewMatrix() {
		//TODO
		return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);;
	}

	//update the camera internal parameters following a camera move event
	void Camera::move(MOVE_DIRECTION direction, float speed) {
		//TODO
		if (direction == MOVE_FORWARD) {
			cameraPosition += cameraFrontDirection * speed;
		}
		if (direction == MOVE_BACKWARD) {
			cameraPosition -= cameraFrontDirection * speed;
		}
		if (direction == MOVE_LEFT) {
			cameraPosition -= cameraRightDirection * speed;
		}
		if (direction == MOVE_RIGHT) {
			cameraPosition += cameraRightDirection * speed;
		}
		cameraTarget = cameraPosition + cameraFrontDirection;
	}

	//update the camera internal parameters following a camera rotate event
	//yaw - camera rotation around the y axis
	//pitch - camera rotation around the x axis


	void Camera::rotate(float pitch, float yaw) {
		//TODO
		//glm::mat4 rotation4(1.0f);

		//rotation4 = glm::rotate(rotation4, glm::radians(pitch), cameraUpDirection);
		//glm::mat3 rotation = glm::rotate(rotation4, glm::radians(yaw), cameraRightDirection);

		//cameraRightDirection = glm::normalize(rotation * cameraRightDirection);
		//cameraFrontDirection = glm::normalize(rotation * cameraFrontDirection);

		//cameraTarget = cameraPosition + cameraFrontDirection;
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		this->cameraFrontDirection = glm::normalize(direction);
		this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
		this->cameraTarget = cameraPosition + cameraFrontDirection;
	}

}