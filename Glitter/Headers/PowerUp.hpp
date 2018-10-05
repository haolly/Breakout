//
// Created by liuhao on 2018/10/4.
//

#ifndef GLITTER_POWERUP_HPP
#define GLITTER_POWERUP_HPP

#include <glm/vec2.hpp>
#include <GameObject.hpp>

const glm::vec2 SIZE(60, 20);
const glm::vec2 VELOCITY(0.0f, 150.0f);

class PowerUp : public GameObject
{
public:
	std::string Type;
	GLfloat Duration;
	GLboolean Activated;

	PowerUp(std::string type, glm::vec3 color, GLfloat duration, glm::vec2 position,
			Texture2D texture)
			: GameObject(position, SIZE, texture, color, VELOCITY),
			Type(type), Duration(duration), Activated()
	{
	}
};


#endif //GLITTER_POWERUP_HPP
