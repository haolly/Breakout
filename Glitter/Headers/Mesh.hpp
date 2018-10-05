//
// Created by liuhao on 2018/9/6.
//

#ifndef GLITTER_MESH_HPP
#define GLITTER_MESH_HPP

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <string>

using namespace std;

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct Texture {
	unsigned int id;
	std::string type;

};


class Mesh
{
public:
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);

private:
	unsigned int VAO, VBO, EBO;
	void setupMesh();

};


#endif //GLITTER_MESH_HPP
