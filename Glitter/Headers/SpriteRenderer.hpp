//
// Created by liuhao on 2018/9/28.
//

#ifndef GLITTER_SPRITERENDERER_HPP
#define GLITTER_SPRITERENDERER_HPP


#include <glm/vec2.hpp>
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include "Shader.hpp"
#include "Texture2D.hpp"

class SpriteRenderer
{
public:
	SpriteRenderer(const Shader& shader);
	~SpriteRenderer();

	void DrawSprite(const Texture2D& texture, glm::vec2 position, glm::vec2 size = glm::vec2(10, 10), GLfloat rotate = 0.0f,
			glm::vec3 color = glm::vec3(1.0f));

private:
	Shader shader;
	GLuint quadVAO;
	void initRenderData();

};


#endif //GLITTER_SPRITERENDERER_HPP
