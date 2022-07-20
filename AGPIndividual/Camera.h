/*

Name: Martin Harvey

Student ID: B00329330

I declare that the following code was produced by Martin Harvey as a individual assignment for the AGP module and that this is my own work.

I am aware of the penalties incurred by submitting in full or in part work that is not our own and that was developed by third parties that are not appropriately acknowledged.

*/
#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

using namespace glm;

// Abstraction for possible directions of camera movement
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	SPRINT
};

struct Boundary {
	vec3 Max;
	vec3 Min;
};

// Default values for major variables
const float YAW			= -90.0f;
const float PITCH		= 0.0f;
const float SPEED		= 3.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM		= 45.0f;
const vec3 MAX = vec3(25.0f, 0.0f, 25.0f);
const vec3 MIN = vec3(-25.0f, 0.0f, -25.0f);

class Camera
{
public:
	vec3 Position;
	vec3 Front;
	vec3 Up;
	vec3 Right;
	vec3 WorldUp;
	vec3 Forward;
	Boundary boundary;

	float Yaw;
	float Pitch;

	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;
	float Height;

	// Constructor
	Camera(vec3 position = vec3(0.0f, 0.0f, 0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM), Height(position.y)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		boundary.Max = MAX;
		boundary.Min = MIN;
		updateCameraVectors();
	}

	// Returns the direction the player is looking to translate objects into view space
	mat4 GetViewMatrix()
	{
		return lookAt(Position, Position + Front, Up);
	}

	// Processes the keyboard input to control camera movement
	void ProcessKeyboard(Camera_Movement direction, float deltaTime, bool sprinting)
	{
		float velocity = MovementSpeed * deltaTime;
		if (sprinting)
			velocity *= 2;
		if (direction == FORWARD)
			Position += Forward * velocity;
		if (direction == BACKWARD)
			Position -= Forward * velocity;
		if (direction == LEFT)
			Position -= Right * velocity;
		if (direction == RIGHT)
			Position += Right * velocity;

		// keep player in room
		float offset = 0.5f;
		if (Position.x >= boundary.Max.x - offset)
			Position.x = boundary.Max.x - offset;
		if (Position.z >= boundary.Max.z - offset)
			Position.z = boundary.Max.z - offset;
		if (Position.x <= boundary.Min.x + offset)
			Position.x = boundary.Min.x + offset;
		if (Position.z <= boundary.Min.z + offset)
			Position.z = boundary.Min.z + offset;
	}

	// Processes the mouse movement to control where the camera is facing
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		updateCameraVectors();
	}

	// Processes the scroll wheel to control the zoom
	void ProcessMouseScroll(float yoffset)
	{
		if (Zoom >= 1.0f && Zoom <= ZOOM)
			Zoom -= yoffset;
		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > 45.0f)
			Zoom = 45.0f;
	}

private:
	// Reacalculates the cameras direction vectors
	void updateCameraVectors()
	{
		vec3 front;
		front.x = cos(radians(Yaw)) * cos(radians(Pitch));
		front.y = sin(radians(Pitch));
		front.z = sin(radians(Yaw)) * cos(radians(Pitch));
		Front = normalize(front);

		Right = normalize(cross(Front, WorldUp));
		Forward = normalize(cross(WorldUp, Right));
		Up = normalize(cross(Right, Front));
	}
};
#endif
