//#include "pch.h"
//

#include <iostream>
#include <string>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <stdio.h>
// Nagłówki
#include <GL/glew.h>
#include <SFML/Window.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <fstream>
#include <sstream>>

// Kody shaderów
const GLchar* vertexSource = R"glsl(
#version 150 core
in vec3 position;
in vec3 color;
out vec3 Color;
in vec2 aTexCoord;
out vec2 TexCoord;
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 proj;

void main(){
TexCoord = aTexCoord;
Color = color;

gl_Position = proj * view * model * vec4(position, 1.0);

}
)glsl";

const GLchar* fragmentSource = R"glsl(
#version 150 core
in vec3 Color;
out vec4 outColor;
in vec2 TexCoord;
uniform sampler2D texture1;
uniform sampler2D texture2;
void main()
{
//outColor = vec4(Color, 1.0);
//outColor = mix(texture(texture2, TexCoord), texture(texture1, TexCoord), 0.4);
outColor=texture(texture1, TexCoord);
//outColor=texture(texture2, TexCoord)*texture(texture1,TexCoord);
//outColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)glsl";

//Globalne zmienne do obsługi kamery
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float obrot = 0.0f;
bool firstMouse = false;
int lastX, lastY;
double yaw = -90;
double pitch = 0;

void StereoProjection(GLuint shaderProgram_, float _left, float _right, float _bottom, float _top, float _near, float _far, float _zero_plane, float _dist, float _eye)
{
	//    Perform the perspective projection for one eye's subfield.
	//    The projection is in the direction of the negative z-axis.
	//            _left=-6.0;
	//            _right=6.0;
	//            _bottom=-4.8;
	   //             _top=4.8;
	//    [default: -6.0, 6.0, -4.8, 4.8]
	//    left, right, bottom, top = the coordinate range, in the plane of zero parallax setting,
	//         which will be displayed on the screen.
	//         The ratio between (right-left) and (top-bottom) should equal the aspect
	//    ratio of the display.


	   //                  _near=6.0;
	   //                  _far=-20.0;
	//    [default: 6.0, -6.0]
	//    near, far = the z-coordinate values of the clipping planes.

	   //                  _zero_plane=0.0;
	//    [default: 0.0]
	//    zero_plane = the z-coordinate of the plane of zero parallax setting.

	//    [default: 14.5]
	  //                     _dist=10.5;
	//   dist = the distance from the center of projection to the plane of zero parallax.

	//    [default: -0.3]
	  //                 _eye=-0.3;
	//    eye = half the eye separation; positive for the right eye subfield,
	//    negative for the left eye subfield.

	float   _dx = _right - _left;
	float   _dy = _top - _bottom;

	float   _xmid = (_right + _left) / 2.0;
	float   _ymid = (_top + _bottom) / 2.0;

	float   _clip_near = _dist + _zero_plane - _near;
	float   _clip_far = _dist + _zero_plane - _far;

	float  _n_over_d = _clip_near / _dist;

	float   _topw = _n_over_d * _dy / 2.0;
	float   _bottomw = -_topw;
	float   _rightw = _n_over_d * (_dx / 2.0 - _eye);
	float   _leftw = _n_over_d * (-_dx / 2.0 - _eye);

	// Create a fustrum, and shift it off axis
	glm::mat4 proj = glm::frustum(_leftw, _rightw, _bottomw, _topw, _clip_near, _clip_far);

	proj = glm::translate(proj, glm::vec3(-_xmid - _eye, -_ymid, 0));

	GLint uniProj = glGetUniformLocation(shaderProgram_, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
}

void set_camera_angle_keyboard(GLint _uniView, float _time) {

	float cameraSpeed = 0.000005f * _time; //ustawić odpowiednio w zależności od szybkości komputera

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
	{
		cameraPos += cameraSpeed * cameraFront;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
	{
		cameraPos -= cameraSpeed * cameraFront;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}

	glm::mat4 view;
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glUniformMatrix4fv(_uniView, 1, GL_FALSE, glm::value_ptr(view));
}
void set_camera_angle_mouse(GLint _uniView, float _elapsedTime, const sf::Window& _window) {
	sf::Vector2i localPosition = sf::Mouse::getPosition(_window);

	sf::Vector2i position;
	bool relocation = false;

	if (localPosition.x <= 0) {
		position.x = _window.getSize().x;
		position.y = localPosition.y;
		relocation = true;

	}
	if (localPosition.x >= _window.getSize().x-1) {
		position.x = 0;
		position.y = localPosition.y;
		relocation = true;
	}
	if (localPosition.y <= 0) {
		position.y = _window.getSize().y;
		position.x = localPosition.x;
		relocation = true;
	}
	if (localPosition.y >= _window.getSize().y-1) {
		position.y = 0;
		position.x = localPosition.x;
		relocation = true;
	}

	if (relocation) {
		sf::Mouse::setPosition(position, _window);
		firstMouse = true;
	}

	localPosition = sf::Mouse::getPosition(_window);

	if (firstMouse) {
		lastX = localPosition.x;
		lastY = localPosition.y;
		firstMouse = false;
	}

	double xoffset = localPosition.x - lastX;
	double yoffset = localPosition.y - lastY;
	lastX = localPosition.x;
	lastY = localPosition.y;

	double sensitivity = 0.001f;
	double cameraSpeed = 0.005f * _elapsedTime;

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset * cameraSpeed;
	pitch -= yoffset * cameraSpeed;

	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

	glm::mat4 view;
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glUniformMatrix4fv(_uniView, 1, GL_FALSE, glm::value_ptr(view));
 }
void make_cube(int buffer) {
	int punkty_ = 36;

	float vertices[] = {
-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,

-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,	1.0f, 0.0f,
 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,	1.0f, 1.0f,
 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,	1.0f, 1.0f,
-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,	0.0f, 1.0f,
-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,	0.0f, 0.0f,

-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,	1.0f, 0.0f,
-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,	1.0f, 1.0f,
-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,	1.0f, 1.0f,
-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,	0.0f, 1.0f,
-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,	0.0f, 0.0f,

 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 0.0f,

-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,	1.0f, 0.0f,
 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,	0.0f, 1.0f,
-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,	0.0f, 0.0f,

-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,	1.0f, 0.0f,
 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,	1.0f, 1.0f,
-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,	0.0f, 1.0f,
-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,	0.0f, 0.0f


	};
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * punkty_ * 8, vertices, GL_STATIC_DRAW);
}

void ilosc_punktow(int punkty, int buffer, float dzielnik) {
	if (punkty < 3) {
		return;
	}

	GLfloat* vertices = new GLfloat[punkty * 6];

	float dalfa = 2 * 3.1415 / punkty; //kat o ktory inkrementujemy
	float alfa = 0; //kat poczatkowy
	float R = 1;  //promien 
	for (int i = 0; i < punkty * 6; i += 6) {
		//ustawianie pozycji wierzcholkow
		vertices[i] = R * cos(alfa);
		vertices[i + 1] = R * sin(alfa);
		vertices[i + 2] = 0;

		// zmiana kolorow RGB
		vertices[i + 3] = ((sin(alfa) + 1) / dzielnik);
		vertices[i + 4] = ((cos(alfa) + 1) / dzielnik);
		vertices[i + 5] = ((sin(alfa) + 1) / dzielnik);
		alfa += dalfa; // zwiekszenie kata
	}

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * punkty * 6, vertices, GL_STATIC_DRAW);
	delete vertices;
	
}

int obj_vert_1 = 0;
int obj_vert_2 = 0;
int obj_vert_3 = 0;
int obj_vert_4 = 0;
bool LoadModelOBJ(int& punkty_, const char* filename, int buffer)
{
	int vert_num = 0;
	int triangles = 0;
	int normals = 0;
	int coord_num = 0;

	std::ifstream myReadFile;
	myReadFile.open(filename);
	std::string output;
	if (myReadFile.is_open()) {
		while (!myReadFile.eof()) {
			myReadFile >> output;
			if (output == "v") vert_num++;
			if (output == "f") triangles++;
			if (output == "vn") normals++;
			if (output == "vt") coord_num++;
		}
	}

	myReadFile.close();
	myReadFile.open(filename);

	if (myReadFile.is_open()) {
		while (!myReadFile.eof()) {
			myReadFile >> output;
			if (output == "o") {
				myReadFile >> output;
				while (output != "o") {
					if (output == "f") obj_vert_1 += 3;
					myReadFile >> output;
				}
			}
			if (output == "o") {
				myReadFile >> output;
				while (output != "o") {
					if (output == "f") obj_vert_2 += 3;
					myReadFile >> output;
				}
			}
			if (output == "o") {
				myReadFile >> output;
				while (output != "o") {
					if (output == "f") obj_vert_3 += 3;
					myReadFile >> output;
				}
			}
			if (output == "o") {
				myReadFile >> output;
				while (!myReadFile.eof()) {
					if (output == "f") obj_vert_4 += 3;
					myReadFile >> output;
				}
			}
		}
	}


	myReadFile.close();
	myReadFile.open(filename);


	float** vert;
	vert = new float* [vert_num]; //przydzielenie pamięci na w wierszy

	for (int i = 0; i < vert_num; i++)
		vert[i] = new float[3];


	int** trian;
	trian = new int* [triangles]; //przydzielenie pamięci na w wierszy

	for (int i = 0; i < triangles; i++)
		trian[i] = new int[9];

	float** norm;
	norm = new float* [normals]; //przydzielenie pamięci na w wierszy

	for (int i = 0; i < normals; i++)
		norm[i] = new float[3];

	float** coord;
	coord = new float* [coord_num]; //przydzielenie pamięci na w wierszy

	for (int i = 0; i < coord_num; i++)
		coord[i] = new float[2];

	int licz_vert = 0;
	int licz_triang = 0;
	int licz_normals = 0;
	int licz_coord = 0;


	while (!myReadFile.eof()) {
		output = "";
		myReadFile >> output;
		if (output == "vn") { myReadFile >> norm[licz_normals][0]; myReadFile >> norm[licz_normals][1]; myReadFile >> norm[licz_normals][2]; licz_normals++; }
		if (output == "v") { myReadFile >> vert[licz_vert][0]; myReadFile >> vert[licz_vert][1]; myReadFile >> vert[licz_vert][2]; licz_vert++; }
		if (output == "vt") { myReadFile >> coord[licz_coord][0]; myReadFile >> coord[licz_coord][1]; licz_coord++; }

		if (output == "f") {

			for (int i = 0; i < 9; i += 3)
			{
				std::string s;
				myReadFile >> s;
				std::stringstream ss(s);

				std::vector <std::string> el;
				std::string item;


				while (getline(ss, item, '/')) {
					el.push_back(item);
				}
				trian[licz_triang][i] = std::stoi(el[0]);
				trian[licz_triang][i + 1] = std::stoi(el[1]);
				trian[licz_triang][i + 2] = std::stoi(el[2]);


			}
			licz_triang++;
		}
	}
	GLfloat* vertices = new GLfloat[triangles * 24];

	int vert_current = 0;

	for (int i = 0; i < triangles; i++)
	{
		vertices[vert_current] = vert[trian[i][0] - 1][0];
		vertices[vert_current + 1] = vert[trian[i][0] - 1][1];
		vertices[vert_current + 2] = vert[trian[i][0] - 1][2];
		vertices[vert_current + 3] = norm[trian[i][2] - 1][0];
		vertices[vert_current + 4] = norm[trian[i][2] - 1][1];
		vertices[vert_current + 5] = norm[trian[i][2] - 1][2];
		vertices[vert_current + 6] = coord[trian[i][1] - 1][0];
		vertices[vert_current + 7] = coord[trian[i][1] - 1][1];

		vertices[vert_current + 8] = vert[trian[i][3] - 1][0];
		vertices[vert_current + 9] = vert[trian[i][3] - 1][1];
		vertices[vert_current + 10] = vert[trian[i][3] - 1][2];
		vertices[vert_current + 11] = norm[trian[i][5] - 1][0];
		vertices[vert_current + 12] = norm[trian[i][5] - 1][1];
		vertices[vert_current + 13] = norm[trian[i][5] - 1][2];
		vertices[vert_current + 14] = coord[trian[i][4] - 1][0];
		vertices[vert_current + 15] = coord[trian[i][4] - 1][1];

		vertices[vert_current + 16] = vert[trian[i][6] - 1][0];
		vertices[vert_current + 17] = vert[trian[i][6] - 1][1];
		vertices[vert_current + 18] = vert[trian[i][6] - 1][2];
		vertices[vert_current + 19] = norm[trian[i][8] - 1][0];
		vertices[vert_current + 20] = norm[trian[i][8] - 1][1];
		vertices[vert_current + 21] = norm[trian[i][8] - 1][2];
		vertices[vert_current + 22] = coord[trian[i][7] - 1][0];
		vertices[vert_current + 23] = coord[trian[i][7] - 1][1];

		vert_current += 24;
	}

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * triangles * 24, vertices, GL_STATIC_DRAW);

	punkty_ = triangles * 3;

	delete vertices;




	for (int i = 0; i < vert_num; i++)
		delete[] vert[i];
	delete[] vert;

	for (int i = 0; i < triangles; i++)
		delete[] trian[i];
	delete[] trian;

	for (int i = 0; i < normals; i++)
		delete[] norm[i];
	delete[] norm;

	for (int i = 0; i < coord_num; i++)
		delete[] coord[i];
	delete[] coord;

	return 0;
}
void LoadModelOBJ_EBO(int& punkty_, const char* filename, int buffer_vbo, int buffer_ebo)
{
	int vert_num = 0;
	int trian_num = 0;

	std::ifstream myReadFile;
	myReadFile.open(filename);

	std::string output;

	if (myReadFile.is_open()) {
		while (!myReadFile.eof()) {
			myReadFile >> output;
			if (output == "v") vert_num++;
			if (output == "f") trian_num++;
		}
	}

	myReadFile.close();
	myReadFile.open(filename);

	float *vert;
	vert = new float[vert_num * 3];
	int* element;
	element = new int[trian_num * 3];

	int licz_vert = 0;
	int licz_element = 0;
	int tmp = 0;

	while (!myReadFile.eof()) {
		myReadFile >> output;
		if (output == "v") {
			myReadFile >> vert[licz_vert];
			myReadFile >> vert[licz_vert + 1];
			myReadFile >> vert[licz_vert + 2];
			licz_vert += 3;
		}
		if (output == "f") {
			myReadFile >> tmp; tmp--; element[licz_element] = tmp;
			myReadFile >> tmp; tmp--; element[licz_element + 1] = tmp;
			myReadFile >> tmp; tmp--; element[licz_element + 2] = tmp;
			licz_element += 3;
		}
		output.clear();
	}

	glBindBuffer(GL_ARRAY_BUFFER, buffer_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * vert_num * 3, vert, GL_STATIC_DRAW);


	glBindBuffer(GL_ARRAY_BUFFER, buffer_ebo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * trian_num * 3, element, GL_STATIC_DRAW);

	punkty_ = trian_num * 3;

	delete[] vert;
	delete[] element;
}
int main()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;

	// Okno renderingu
	sf::Window window(sf::VideoMode(1280, 720, 32), "OpenGL", sf::Style::Titlebar | sf::Style::Close, settings);
	//sf::Window window(sf::VideoMode(1920, 1080, 32), "OpenGL", sf::Style::Fullscreen | sf::Style::Close, settings);

	// Inicjalizacja GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	window.setMouseCursorGrabbed(true);
	window.setMouseCursorVisible(false);

	// Utworzenie VAO (Vertex Array Object)
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//Element buffer object
	GLuint ebo;
	glGenBuffers(1, &ebo);

	// Utworzenie VBO (Vertex Buffer Object)
	// i skopiowanie do niego danych wierzchołkowych
	GLuint vbo;
	glGenBuffers(1, &vbo);

	//make_cube(vbo);
	int punkty = 0;
	LoadModelOBJ(punkty, "room.obj", vbo);
	//LoadModelOBJ_EBO(punkty, "test.obj", vbo, ebo);

	/*
	GLfloat vertices[] = {
	0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
	0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
	-0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	*/

	/*
	int punkty = 20;
	ilosc_punktow(punkty, vbo, 1.0);
	*/

	// Utworzenie i skompilowanie shadera wierzchołków
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	GLint status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	//if sprawdzi czy shadery skompilowaly sie poprawnie
	if (status == GL_FALSE)
	{
		GLint maxLength = 0;
		//zapisujemy dlugosc loga do "maxlength"
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

		//tworzymy vector ktory przechowa nam nasz errorlog
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &errorLog[0]);

		//wypisujemy errorlog, usuwamy niedzialajacy shader i wychodzimy z kodem -1
		std::cout << "Blad podczas kompilowania shadera \n Kod bledu: ";
		for (int i = 0; i < maxLength; i++) {
			std::cout << errorLog[i];
		}
		glDeleteShader(vertexShader);
		return -1;
	}

	std::cout << "Kompilacja shaderow powiodla sie" << std::endl;
	// Utworzenie i skompilowanie shadera fragmentów
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	//if sprawdzi czy shadery skompilowaly sie poprawnie
	if (status == GL_FALSE)
	{
		GLint maxLength = 0;
		//zapisujemy dlugosc loga do "maxlength"
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

		//tworzymy vector ktory przechowa nam nasz errorlog
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &errorLog[0]);

		//wypisujemy errorlog, usuwamy niedzialajacy shader i wychodzimy z kodem -1
		std::cout << "Blad podczas kompilowania shadera \n Kod bledu: ";
		for (int i = 0; i < maxLength; i++) {
			std::cout << errorLog[i];
		}
		glDeleteShader(fragmentShader);
		return -1;
	}

	// Zlinkowanie obu shaderów w jeden wspólny program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	//glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	// Specifikacja formatu danych wierzchołkowych
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	
	//GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
	//glEnableVertexAttribArray(colAttrib);
	//glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	
	GLint TexCoord = glGetAttribLocation(shaderProgram, "aTexCoord");
	glEnableVertexAttribArray(TexCoord);
	glVertexAttribPointer(TexCoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
	
	//tworzenie macierzy modelu, widoku, projekcji
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 1.0f));

	GLint uniTrans = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(model));

	GLint uniView = glGetUniformLocation(shaderProgram, "view");
	glm::mat4 proj = glm::perspective(glm::radians(80.0f), 800.0f / 800.0f, 0.06f, 100.0f);
	GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	/////////////////////////////////////////////////////////

	int mouse_prev_y = 0;
	int mouse_prev_x = 0;
	float dzielnik = 2.0f;
	int primitive = GL_TRIANGLES;

	unsigned int tryb_wyswietlania = 2;
	float dist = 13;

	// Rozpoczęcie pętli zdarzeń
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);


	//TEXTURY
	unsigned int texture1;
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	//wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//filtry
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("floor.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "blad wczytywania tekstur" << std::endl;
	}
	stbi_image_free(data);

	unsigned int texture2;
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	//wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//filtry
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//
	//int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("wood.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "blad wczytywania tekstur" << std::endl;
	}
	stbi_image_free(data);

	unsigned int texture3;
	glGenTextures(1, &texture3);
	glBindTexture(GL_TEXTURE_2D, texture3);
	//wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//filtry
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//
	//int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("stone.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "blad wczytywania tekstur" << std::endl;
	}
	stbi_image_free(data);

	unsigned int texture4;
	glGenTextures(1, &texture4);
	glBindTexture(GL_TEXTURE_2D, texture4);
	//wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//filtry
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//
	//int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("wood_2.jpeg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "blad wczytywania tekstur" << std::endl;
	}
	stbi_image_free(data);

	glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture3"), 2);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture4"), 3);
	bool running = true;

	sf::Clock clock;
	window.setFramerateLimit(60);
	sf::Time time;
	int licznik = 0;
	while (running) {

		time = clock.restart();
		licznik++;

		float licznik_fps = 1000000 / time.asMicroseconds();
		//float licznik_fps = time.asMicroseconds()/ 1000000;
		if (licznik > licznik_fps) {
			window.setTitle(std::to_string(licznik_fps));
			licznik = 0;
		}
		sf::Event windowEvent;
		while (window.pollEvent(windowEvent)) {
			switch (windowEvent.type) {
			case sf::Event::Closed:
				running = false;
				break;

			case sf::Event::KeyPressed:
				switch (windowEvent.key.code)
				{
				case sf::Keyboard::Escape:
					running = false;
					break;

				case sf::Keyboard::Num0:
					primitive = GL_POINTS;
					break;
				case sf::Keyboard::Num1:
					primitive = GL_LINES;
					break;
				case sf::Keyboard::Num2:
					primitive = GL_LINE_STRIP;
					break;
				case sf::Keyboard::Num3:
					primitive = GL_LINE_LOOP;
					break;
				case sf::Keyboard::Num4:
					primitive = GL_POLYGON;
					break;
				case sf::Keyboard::Num5:
					primitive = GL_QUADS;
					break;
				case sf::Keyboard::Num6:
					primitive = GL_QUAD_STRIP;
					break;
				case sf::Keyboard::Num7:
					primitive = GL_TRIANGLES;
					break;
				case sf::Keyboard::Num8:
					primitive = GL_TRIANGLE_STRIP;
					break;
				case sf::Keyboard::Num9:
					primitive = GL_TRIANGLE_FAN;
					break;
				case sf::Keyboard::Q:
					tryb_wyswietlania = 0;
					break;
				case sf::Keyboard::W:
					tryb_wyswietlania = 1;
					break;
				case sf::Keyboard::E:
					tryb_wyswietlania = 2;
					break;
				case sf::Keyboard::R:
					dist += 0.01;
					break;
				case sf::Keyboard::T:
					dist -= 0.01;
					break;

				default:
					break;
				}

			case sf::Event::MouseMoved:

				set_camera_angle_mouse(uniView, time.asMicroseconds(), window);
				break;
				/*
			case sf::Event::MouseMoved:
				if (windowEvent.mouseMove.y > mouse_prev_y)
				{
					static int counter = 0;
					counter++;
					if (counter > 3) {
						punkty++;
						std::cout << "Vertices: " << punkty << " | RGB_factor: " << dzielnik << std::endl;
						mouse_prev_y = windowEvent.mouseMove.y;
						ilosc_punktow(punkty, vbo, dzielnik);
						counter = 0;

					}
				}
				else if (windowEvent.mouseMove.y < mouse_prev_y){
					static int counter = 0;
					counter++;
					if (counter > 3) {
						punkty--;
						std::cout << "Vertices: " << punkty << " | RGB_factor: " << dzielnik << std::endl;
						mouse_prev_y = windowEvent.mouseMove.y;
						ilosc_punktow(punkty, vbo, dzielnik);
						counter = 0;
					}
				}
				*/

				/*
				else if (windowEvent.mouseMove.x > mouse_prev_x) {
						if (dzielnik < 5) dzielnik += 0.05;
						std::cout << "Vertices: " << punkty << " | RGB_factor: " << dzielnik << std::endl;
						mouse_prev_x = windowEvent.mouseMove.x;
						ilosc_punktow(punkty, vbo, dzielnik);
				}
				else {
						if (dzielnik > 0.4) dzielnik -= 0.05;
						std::cout << "Vertices: " << punkty << " | RGB_factor: " << dzielnik << std::endl;
						mouse_prev_x = windowEvent.mouseMove.x;
						ilosc_punktow(punkty, vbo, dzielnik);
				}
				*/
			}
		}

		set_camera_angle_keyboard(uniView, time.asMicroseconds());
		//std::cin >> punkty;

		// Nadanie scenie koloru czarnego
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// wczytanie tekstur
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texture3);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, texture4);

		switch (tryb_wyswietlania) {
		case 0:
			glViewport(0, 0, window.getSize().x, window.getSize().y);
			glDrawBuffer(GL_BACK_LEFT);
			StereoProjection(shaderProgram, -6, 6, -4.8, 4.8, 12.99, -100, 0, dist, -0.05);
			glColorMask(true, false, false, false);
			glDrawArrays(primitive, 0, 36);
			
			glClear(GL_DEPTH_BUFFER_BIT);
			glDrawBuffer(GL_BACK_RIGHT);
			StereoProjection(shaderProgram, -6, 6, -4.8, 4.8, 12.99, -100, 0, dist, 0.05);
			glColorMask(false, false, true, false);
			glDrawArrays(primitive, 0, 36);
			glColorMask(true, true, true, true);
			break;
		case 1:
			glViewport(0,0, window.getSize().x/2, window.getSize().y);
			StereoProjection(shaderProgram, -6, 6, -4.8, 4.8, 12.99, -100, 0, 13, -0.05);
			glDrawArrays(primitive, 0, 36);

			glViewport(window.getSize().x / 2, 0, window.getSize().x / 2, window.getSize().y);
			StereoProjection(shaderProgram, -6, 6, -4.8, 4.8, 12.99, -100, 0, 13, 0.05);
			glDrawArrays(primitive, 0, 36);

			break;
		case 2:
			glActiveTexture(GL_TEXTURE0);
			glViewport(0, 0, window.getSize().x, window.getSize().y);

			glBindTexture(GL_TEXTURE_2D, texture1);
			glDrawArrays(primitive, 0, obj_vert_1);
			
			glBindTexture(GL_TEXTURE_2D, texture2);
			glDrawArrays(primitive, obj_vert_1, obj_vert_2);
			
			glBindTexture(GL_TEXTURE_2D, texture3);
			glDrawArrays(primitive, (obj_vert_1+obj_vert_2), obj_vert_3);
			
			glBindTexture(GL_TEXTURE_2D, texture4);
			glDrawArrays(primitive, (obj_vert_1+obj_vert_2+obj_vert_3), obj_vert_4);
			//glDrawElements(primitive, punkty, GL_UNSIGNED_INT, 0);
		default:
			break;
		}

		// Narysowanie trójkąta na podstawie 3 wierzchołków
		//glDrawArrays(primitive, 0, punkty);
		//glDrawArrays(primitive, 0, 36);
		// Wymiana buforów tylni/przedni
		window.display();
	}
	// Kasowanie programu i czyszczenie buforów
	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	// Zamknięcie okna renderingu
	window.close();
	return 0;

}