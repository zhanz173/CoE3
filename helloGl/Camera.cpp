
#pragma region ignore
void Camera::SetPosition(const glm::vec3& _position)
{
	Position = _position;
}

void Camera::SetDirection(const glm::vec3& _direction)
{
	Direction = _direction;
}

void Camera::SetRotation(float Yaw, float Pitch)
{
	Direction.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Direction.y = sin(glm::radians(Pitch));
	Direction.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
}

glm::mat4 Camera::GetViewM()
{
	return glm::lookAt(Position, Direction + Position, up);
}

glm::mat4 Camera::GetProjM()
{
	glm::mat4 view = GetViewM();
	return glm::perspective(glm::radians(FOVdeg), (float)m_width / (float)m_height, nearPlane, farPlane)* view;
}

void Camera::ProcessMouseScroll(float yoffset)
{
	Zoom -= (float)yoffset;
	if (Zoom < 1.0f)
		Zoom = 1.0f;
	if (Zoom > 45.0f)
		Zoom = 45.0f;
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
	float velocity = MovementSpeed * deltaTime;
	if (direction == FORWARD)
		Position += Direction * velocity;
	if (direction == BACKWARD)
		Position -= Direction * velocity;
	if (direction == LEFT)
		Position -= Right * velocity;
	if (direction == RIGHT)
		Position += Right * velocity;
}
#pragma endregion This file is WIP and should be excluded from the build