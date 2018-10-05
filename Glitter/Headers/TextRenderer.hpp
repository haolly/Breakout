//
// Created by liuhao on 2018/10/4.
//

#ifndef GLITTER_TEXTRENDERER_HPP
#define GLITTER_TEXTRENDERER_HPP

#include <unordered_map>
#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include "Shader.hpp"

struct Character {
	GLuint TextureID;
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	GLuint Advance;
};

class TextRenderer
{
public:
	std::unordered_map<GLchar, Character> Characters;
	Shader TextShader;
	TextRenderer(GLuint width, GLuint height);
	void RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color = glm::vec3(1.0f));
	void Load(std::string font, GLuint fontSize);
private:
	GLuint VAO, VBO;
};


#endif //GLITTER_TEXTRENDERER_HPP
