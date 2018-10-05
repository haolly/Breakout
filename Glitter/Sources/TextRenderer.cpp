//
// Created by liuhao on 2018/10/4.
//

//#include <freetype/freetype.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <TextRenderer.hpp>
#include <ResourceManager.hpp>

#include "TextRenderer.hpp"


TextRenderer::TextRenderer(GLuint width, GLuint height)
{
	this->TextShader = ResourceManager::LoadShader("Resource/text.vert", "Resource/text.frag", nullptr, "text");
	this->TextShader.SetMatrix4("projection",
			glm::ortho(0.0f,
					static_cast<GLfloat>(width),
					static_cast<GLfloat>(height),
					0.0f,
					-1.0f, 1.0f),
					GL_TRUE);
	this->TextShader.SetInteger("text", 0);

	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void TextRenderer::RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
	this->TextShader.Use();
	this->TextShader.SetVector3f("textColor", color);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	for(auto iter = text.cbegin(); iter != text.cend(); ++iter)
	{
		Character ch = Characters[*iter];

		auto normalY = this->Characters['H'].Bearing.y;
		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y + (normalY - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;

		GLfloat vertices[6][4] = {
				{xpos,  ypos + h, 0.0, 1.0},
				{xpos + w, ypos,  1.0, 0.0},
				{xpos,  ypos,     0.0, 0.0},

				{ xpos,     ypos + h,   0.0, 1.0 },
				{ xpos + w, ypos + h,   1.0, 1.0 },
				{ xpos + w, ypos,       1.0, 0.0 },
		};

		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		x+= (ch.Advance >> 6) * scale;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextRenderer::Load(std::string font, GLuint fontSize)
{
	this->Characters.clear();

	FT_Library ft;
	if(FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: could not init FreeType Library" << std::endl;
	}

	FT_Face face;
//	if(FT_New_Face(ft, "Resource/方正粗圆_GBK_0.ttf", 0, &face))
	if(FT_New_Face(ft, font.c_str(), 0, &face))
	{
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
	}
	FT_Set_Pixel_Sizes(face, 0, 48);
	if(FT_Load_Char(face, 'X', FT_LOAD_RENDER))
	{
		std::cout << "ERROR:FREETYPE: Failed to load Glyph" << std::endl;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for(uint c = 0; c<128; ++c)
	{
		if(FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR:FREETYPE: Failed to load Glyph" << std::endl;
			continue;
		}

		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
				);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
				static_cast<GLuint>(face->glyph->advance.x)
		};
		Characters.insert(std::pair<GLchar, Character>(c, character));
	}
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

//	glEnable(GL_BLEND);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
