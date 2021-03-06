#ifndef GAME_H
#define GAME_H

#include <string>
#include <sstream>

#include <iostream>
#include <GL/glew.h>
#include <GL/wglew.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <Debug.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>
using namespace std;
using namespace sf;
using namespace glm;

class Game
{
public:
	Game();
	Game(sf::ContextSettings settings);
	~Game();
	void run();
private:
	RenderWindow window;
	Clock clock;
	Time time;
	bool animate = false;
	vec3 animation = vec3(0.0f);
	float rotation = 0.0f;
	bool isRunning = false;
	void initialize();
	void update();
	void render();
	void unload();
	void shootCube();
	bool m_cubeFired = false;
	bool m_cubeHit = false;
	int m_lives = 5, m_score = 0;

	glm::vec3 npcLeftPos= glm::vec3(-8.0f, -3.0f, -10.0f);

	glm::vec3 npcCenterPos= glm::vec3(0.0f, -3.0f, -10.0f);

	glm::vec3 npcRightPos=glm::vec3(8.0f, -3.0f, -10.0f);

	glm::vec3 playerPos = glm::vec3(0.0f, -3.0f, 0.0f);

	glm::vec3 originalPlayerPos= glm::vec3(0.0f, -3.0f, 0.0f);

	std::string loadShaderFromFile(const std::string & textFile);

};

#endif  // ! GAME_H