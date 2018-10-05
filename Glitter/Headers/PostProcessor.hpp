//
// Created by liuhao on 2018/10/2.
//

#ifndef GLITTER_POSTPROCESSOR_HPP
#define GLITTER_POSTPROCESSOR_HPP


#include <Shader.hpp>
#include <Texture2D.hpp>

class PostProcessor
{
public:
	Shader PostProcessingShader;
	Texture2D Texture;
	GLuint Width, Height;
	GLboolean Confuse, Chaos, Shake;

	PostProcessor(Shader shader, GLuint width, GLuint height);
	void BeginRender();
	void EndRender();
	void Render(GLfloat time);

private:
	GLuint MSFBO, FBO;
	GLuint RBO;
	GLuint VAO;
	void initRenderData();

};


#endif //GLITTER_POSTPROCESSOR_HPP
