#include "Pillar.h"
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <algorithm>
#include <cmath>
#include <vector>

namespace {
GLuint cylinder_vao = 0;
GLuint cylinder_vbo = 0;
GLsizei cylinder_vertex_count = 0;

void append_vertex(std::vector<GLfloat>& vertices, const glm::vec3& position, const glm::vec3& normal)
{
	vertices.insert(vertices.end(), {
		position.x, position.y, position.z,
		normal.x, normal.y, normal.z
	});
}

void initialize_cylinder_mesh()
{
	if (cylinder_vao != 0)
		return;

	constexpr int slices = 24;
	constexpr float pi = 3.14159265358979323846f;
	std::vector<GLfloat> vertices;
	vertices.reserve(slices * 12 * 6);

	for (int i = 0; i < slices; ++i) {
		const float angle0 = 2.f * pi * i / slices;
		const float angle1 = 2.f * pi * (i + 1) / slices;
		const glm::vec3 n0(std::cos(angle0), 0.f, std::sin(angle0));
		const glm::vec3 n1(std::cos(angle1), 0.f, std::sin(angle1));
		const glm::vec3 b0(n0.x, 0.f, n0.z);
		const glm::vec3 b1(n1.x, 0.f, n1.z);
		const glm::vec3 t0(n0.x, 1.f, n0.z);
		const glm::vec3 t1(n1.x, 1.f, n1.z);

		append_vertex(vertices, b0, n0);
		append_vertex(vertices, t0, n0);
		append_vertex(vertices, t1, n1);
		append_vertex(vertices, b0, n0);
		append_vertex(vertices, t1, n1);
		append_vertex(vertices, b1, n1);

		const glm::vec3 up(0.f, 1.f, 0.f);
		append_vertex(vertices, glm::vec3(0.f, 1.f, 0.f), up);
		append_vertex(vertices, t1, up);
		append_vertex(vertices, t0, up);

		const glm::vec3 down(0.f, -1.f, 0.f);
		append_vertex(vertices, glm::vec3(0.f), down);
		append_vertex(vertices, b0, down);
		append_vertex(vertices, b1, down);
	}

	cylinder_vertex_count = static_cast<GLsizei>(vertices.size() / 6);
	glGenVertexArrays(1, &cylinder_vao);
	glGenBuffers(1, &cylinder_vbo);
	glBindVertexArray(cylinder_vao);
	glBindBuffer(GL_ARRAY_BUFFER, cylinder_vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}
}


Pillar::Pillar() : position{}, radius{}, height{}
{
}

Pillar::Pillar(const glm::vec3 pos, const float r, const float h) : position{ pos }, radius{ r }, height{ h }
{
}

Pillar::~Pillar()
{
}

Pillar::Pillar(const Pillar& other) : position{ other.position }, radius{ other.radius }, height{ other.height }, select{ other.select }
{
}

Pillar& Pillar::operator=(const Pillar& other)
{
	if (this != &other) {
		position = other.position;
		height = other.height;
		radius = other.radius;
		select = other.select;
	}

	return *this;
}

glm::vec3 Pillar::get_position() const
{
	return glm::vec3(position.x, height, position.z);
}

float Pillar::get_radius() const
{
	return radius;
}

float Pillar::get_height() const
{
	return height;
}

void Pillar::selected()
{
	select = true;
}

void Pillar::unselected()
{
	select = false;
}

bool Pillar::is_selected() const
{
	return select;
}

void Pillar::change_height_by_a(float a)
{
	height = std::max(0.1f, height + a);
}

void Pillar::draw(GLuint shader_program, glm::vec3 color)
{
	initialize_cylinder_mesh();

	glm::mat4 transform(1.f);
	transform = glm::translate(transform, position);
	transform = glm::scale(transform, glm::vec3(radius, height, radius));


	GLint Location = glGetUniformLocation(shader_program, "modelTransform");
	glUniformMatrix4fv(Location, 1, GL_FALSE, glm::value_ptr(transform));


	if (select)
		color = { 1.f, 0.f, 0.f };

	int objColorLocation = glGetUniformLocation(shader_program, "objectColor");
	glUniform3fv(objColorLocation, 1, glm::value_ptr(color));

	glBindVertexArray(cylinder_vao);
	glDrawArrays(GL_TRIANGLES, 0, cylinder_vertex_count);
}

void Pillar::draw_edge(GLuint shader_program)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glm::vec3 Color = {0.f, 0.f, 0.f};

	glVertexAttrib3fv(1, glm::value_ptr(Color));

	int objColorLocation = glGetUniformLocation(shader_program, "objectColor");
	glUniform3fv(objColorLocation, 1, glm::value_ptr(Color));

	glLineWidth(0.5f);

	//glBegin(GL_QUADS);
	//glVertex3f(position.x - radius, position.y + height, position.z); //사각형을 그리기 위해 4개의 정점을 설정한다.
	//glVertex3f(position.x + radius, position.y + height, position.z);
	//glVertex3f(position.x + radius, position.y, position.z);
	//glVertex3f(position.x - radius, position.y, position.z);
	//glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(position.x, position.y + height, position.z - radius); //사각형을 그리기 위해 4개의 정점을 설정한다.
	glVertex3f(position.x, position.y + height, position.z + radius);
	glVertex3f(position.x, position.y, position.z + radius);
	glVertex3f(position.x, position.y, position.z - radius);
	glEnd();
}

void Pillar::draw_line(GLuint shader_program)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glm::vec3 Color = { 0.f, 0.f, 0.f };

	glVertexAttrib3fv(1, glm::value_ptr(Color));

	int objColorLocation = glGetUniformLocation(shader_program, "objectColor");
	glUniform3fv(objColorLocation, 1, glm::value_ptr(Color));

	glLineWidth(0.5f);

	//glBegin(GL_QUADS);
	//glVertex3f(position.x - radius, position.y + height, position.z); //사각형을 그리기 위해 4개의 정점을 설정한다.
	//glVertex3f(position.x + radius, position.y + height, position.z);
	//glVertex3f(position.x + radius, position.y, position.z);
	//glVertex3f(position.x - radius, position.y, position.z);
	//glEnd();

	glBegin(GL_LINE_LOOP);
	glVertex3f(position.x, position.y + height, position.z - radius); //사각형을 그리기 위해 4개의 정점을 설정한다.
	glVertex3f(position.x, position.y + height, position.z + radius);
	glVertex3f(position.x, position.y, position.z + radius);
	glVertex3f(position.x, position.y, position.z - radius);
	glEnd();
}

bool Pillar::is_conflict(glm::vec3 pos, float& length, bool side_view) const
{
	if (!side_view) {
		if (position.x - radius <= pos.x && pos.x <= position.x + radius
			&& position.y <= pos.y && pos.y <= position.y + height) {
			length = -position.z;
			return true;
		}
		return false;
	}
	else {
		if (position.z - radius <= pos.z && pos.z <= position.z + radius
			&& position.y <= pos.y && pos.y <= position.y + height) {
			length = -position.x;
			return true;
		}
	}
	return false;
}
