#include <Game.h>
#include <Cube.h>
#include <Easing.h>
#include "NPC.h"
#include "Player.h"

// Helper to convert Number to String for HUD
template <typename T>
string toString(T number)
{
	ostringstream oss;
	oss << number;
	return oss.str();
}

GLuint	vsid,		// Vertex Shader ID
fsid,		// Fragment Shader ID
progID,		// Program ID
vao = 0,	// Vertex Array ID
vbo,		// Vertex Buffer ID
vib,		// Vertex Index Buffer
to[1];		// Texture ID
GLint	positionID,	// Position ID
colorID,	// Color ID
textureID,	// Texture ID
uvID,		// UV ID
mvpID[4],		// Model View Projection ID
x_offsetID, // X offset ID
y_offsetID,	// Y offset ID
z_offsetID;	// Z offset ID

GLenum	error;		// OpenGL Error Code


					//Please see .//Assets//Textures// for more textures
const string filename = ".//Assets//Textures//grid_wip.tga";

int width;						// Width of texture
int height;						// Height of texture
int comp_count;					// Component of texture

unsigned char* img_data;		// image data


mat4 mvpLeft, mvpCenter, mvpRight, mvpPlayer, projection,
view, modelLeft, modelCenter, modelRight, modelPlayer;			// Model View Projection

Font font;						// Game font

float x_offset, y_offset, z_offset; // offset on screen (Vertex Shader)

Game::Game() :
	window(VideoMode(800, 600),
		"Introduction to OpenGL Texturing")
{
}

Game::Game(sf::ContextSettings settings) :
	window(VideoMode(800, 600),
		"Introduction to OpenGL Texturing",
		sf::Style::Default,
		settings)
{
}

Game::~Game() {}
std::string Game::loadShaderFromFile(const std::string & textFile)
{
	ifstream file;
	file.open(((textFile).c_str()));
	string output;
	string line;
	if (file.is_open())
	{
		while (!file.eof())
		{
			getline(file, line);
			output.append(line + "\n");
		}
	}
	return output;
}

void Game::run()
{

	initialize();

	Event event;

	float rotation = 0.01f;
	float start_value = 0.0f;
	float end_value = 1.0f;

	while (isRunning) {

#if (DEBUG >= 2)
		DEBUG_MSG("Game running...");
#endif

		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
			{
				isRunning = false;
			}

			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			{
				// Set Model Rotation
				modelLeft = rotate(modelLeft, -0.01f, glm::vec3(1, 0, 0)); // Rotate
			}

			else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			{
				// Set Model Rotation
				modelLeft = rotate(modelLeft, 0.01f, glm::vec3(1, 0, 0)); // Rotate
			}

			if (animate)
			{
				rotation += (1.0f * rotation) + 0.05f;
				modelLeft = rotate(modelLeft, 0.01f, animation); // Rotate
				rotation = 0.0f;
				animate = false;
			}
		}

		update();
		render();

	}

#if (DEBUG >= 2)
	DEBUG_MSG("Calling Cleanup...");
#endif
	unload();

}

void Game::initialize()
{
	isRunning = true;
	GLint isCompiled = 0;
	GLint isLinked = 0;

	if (!(!glewInit())) { DEBUG_MSG("glewInit() failed"); }

	// Copy UV's to all faces
	for (int i = 1; i < 6; i++)
		memcpy(&uvs[i * 4 * 2], &uvs[0], 2 * 4 * sizeof(GLfloat));

	DEBUG_MSG(glGetString(GL_VENDOR));
	DEBUG_MSG(glGetString(GL_RENDERER));
	DEBUG_MSG(glGetString(GL_VERSION));

	// Vertex Array Buffer
	glGenBuffers(1, &vbo);		// Generate Vertex Buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Vertices (3) x,y,z , Colors (4) RGBA, UV/ST (2)
	//glBufferData(GL_ARRAY_BUFFER, ((3 * VERTICES) + (4 * COLORS) + (2 * UVS)) * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, ((3 * VERTICES_NPC) + (4 * COLORS_NPC) + (2 * UVS_NPC)) * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, ((3 * VERTICES_NPC) + (4 * COLORS_NPC) + (2 * UVS_NPC)) * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, ((3 * VERTICES_NPC) + (4 * COLORS_NPC) + (2 * UVS_NPC)) * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, ((3 * VERTICES_PLAYER) + (4 * COLORS_PLAYER) + (2 * UVS_PLAYER)) * sizeof(GLfloat), NULL, GL_STATIC_DRAW);

	glGenBuffers(1, &vib); //Generate Vertex Index Buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vib);

	// Indices to be drawn
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * INDICES * sizeof(GLuint), indices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * VERTICES_NPC * sizeof(GLuint), Indices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * VERTICES_NPC * sizeof(GLuint), Indices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * VERTICES_NPC * sizeof(GLuint), Indices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * INDICES_PLAYER * sizeof(GLuint), indicesPlayer, GL_STATIC_DRAW);

	// NOTE: uniforms values must be used within Shader so that they 
	// can be retreived
	const char* vs_src =
		"#version 400\n\r"
		""
		"in vec3 sv_position;"
		"in vec4 sv_color;"
		"in vec2 sv_uv;"
		""
		"out vec4 color;"
		"out vec2 uv;"
		""
		"uniform mat4 sv_mvp;"
		"uniform float sv_x_offset;"
		"uniform float sv_y_offset;"
		"uniform float sv_z_offset;"
		""
		"void main() {"
		"	color = sv_color;"
		"	uv = sv_uv;"
		//"	gl_Position = vec4(sv_position, 1);"
		"	gl_Position = sv_mvp * vec4(sv_position.x + sv_x_offset, sv_position.y + sv_y_offset, sv_position.z + sv_z_offset, 1 );"
		"}"; //Vertex Shader Src

	DEBUG_MSG("Setting Up Vertex Shader");

	vsid = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vsid, 1, (const GLchar**)&vs_src, NULL);
	glCompileShader(vsid);

	// Check is Shader Compiled
	glGetShaderiv(vsid, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_TRUE) {
		DEBUG_MSG("Vertex Shader Compiled");
		isCompiled = GL_FALSE;
	}
	else
	{
		DEBUG_MSG("ERROR: Vertex Shader Compilation Error");
	}

	const char* fs_src =
		"#version 400\n\r"
		""
		"uniform sampler2D f_texture;"
		""
		"in vec4 color;"
		"in vec2 uv;"
		""
		"out vec4 fColor;"
		""
		"void main() {"
		"	fColor = color - texture2D(f_texture, uv);"
		""
		"}"; //Fragment Shader Src

	DEBUG_MSG("Setting Up Fragment Shader");

	fsid = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fsid, 1, (const GLchar**)&fs_src, NULL);
	glCompileShader(fsid);

	// Check is Shader Compiled
	glGetShaderiv(fsid, GL_COMPILE_STATUS, &isCompiled);

	if (isCompiled == GL_TRUE) {
		DEBUG_MSG("Fragment Shader Compiled");
		isCompiled = GL_FALSE;
	}
	else
	{
		DEBUG_MSG("ERROR: Fragment Shader Compilation Error");
	}

	DEBUG_MSG("Setting Up and Linking Shader");
	progID = glCreateProgram();
	glAttachShader(progID, vsid);
	glAttachShader(progID, fsid);
	glLinkProgram(progID);

	// Check is Shader Linked
	glGetProgramiv(progID, GL_LINK_STATUS, &isLinked);

	if (isLinked == 1) {
		DEBUG_MSG("Shader Linked");
	}
	else
	{
		DEBUG_MSG("ERROR: Shader Link Error");
	}

	// Set image data
	// https://github.com/nothings/stb/blob/master/stb_image.h
	img_data = stbi_load(filename.c_str(), &width, &height, &comp_count, 4);

	if (img_data == NULL)
	{
		DEBUG_MSG("ERROR: Texture not loaded");
	}

	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &to[0]);
	glBindTexture(GL_TEXTURE_2D, to[0]);

	// Wrap around
	// https://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexParameter.xml
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// Filtering
	// https://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexParameter.xml
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Bind to OpenGL
	// https://www.khronos.org/opengles/sdk/docs/man/xhtml/glTexImage2D.xml
	glTexImage2D(
		GL_TEXTURE_2D,			// 2D Texture Image
		0,						// Mipmapping Level 
		GL_RGBA,				// GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA, GL_RGB, GL_BGR, GL_RGBA 
		width,					// Width
		height,					// Height
		0,						// Border
		GL_RGBA,				// Bitmap
		GL_UNSIGNED_BYTE,		// Specifies Data type of image data
		img_data				// Image Data
	);

	// Projection Matrix 
	projection = perspective(
		45.0f,					// Field of View 45 degrees
		4.0f / 3.0f,			// Aspect ratio
		5.0f,					// Display Range Min : 0.1f unit
		100.0f					// Display Range Max : 100.0f unit
	);

	// Camera Matrix
	view = lookAt(
		vec3(0.0f, 4.0f, 10.0f),	// Camera (x,y,z), in World Space
		vec3(0.0f, 0.0f, 0.0f),		// Camera looking at origin
		vec3(0.0f, 1.0f, 0.0f)		// 0.0f, 1.0f, 0.0f Look Down and 0.0f, -1.0f, 0.0f Look Up
	);

	// Model matrix
	modelCenter = mat4(
		1.0f					// Identity Matrix
	);
	modelCenter = translate(modelCenter,npcCenterPos);

	modelLeft = mat4(
		1.0f					// Identity Matrix
	);
	modelLeft = translate(modelLeft, npcLeftPos);

	modelRight = mat4(
		1.0f					// Identity Matrix
	);
	modelRight = translate(modelRight, npcRightPos);

	modelPlayer = mat4(
		1.0f
	);
	modelPlayer = translate(modelPlayer,playerPos);


	// Enable Depth Test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	// Load Font
	font.loadFromFile(".//Assets//Fonts//BBrick.ttf");
}

void Game::update()
{
#if (DEBUG >= 2)
	DEBUG_MSG("Updating...");
#endif
	// Update Model View Projection
	// For mutiple objects (cubes) create multiple models
	// To alter Camera modify view & projection



	//move player cube left and right
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
	{
		modelPlayer = translate(modelPlayer, glm::vec3(-0.005, 0, 0)); // Rotate
		playerPos.x -= 0.005;

	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
	{
		modelPlayer = translate(modelPlayer, glm::vec3(0.005, 0, 0)); // Rotate
		playerPos.x += 0.005;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
	{
		//m_cubeFired = true;
		shootCube();
		std::cout << playerPos.z << std::endl;
		
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
	{
		modelPlayer = mat4{
			1.0 };
		playerPos.z = 0.0f;
		modelPlayer = translate(modelPlayer, originalPlayerPos);
	}



	mvpLeft = projection * view * modelLeft;
	mvpCenter = projection * view * modelCenter;
	mvpRight = projection * view * modelRight;
	mvpPlayer = projection * view * modelPlayer;

	modelLeft = rotate(modelLeft, -0.001f, glm::vec3(0, -1, 0)); // Rotate
	modelCenter = rotate(modelCenter, -0.001f, glm::vec3(0, -1, 0)); // Rotate
	modelRight = rotate(modelRight, -0.001f, glm::vec3(0, -1, 0)); // Rotate
	//collision();
	//glm::vec3 npcLeftPos = glm::vec3(-8.0f, -3.0f, -10.0f);

	//glm::vec3 npcCenterPos = glm::vec3(0.0f, -3.0f, -10.0f);

	//glm::vec3 npcRightPos = glm::vec3(8.0f, -3.0f, -10.0f);

	//glm::vec3 playerPos = glm::vec3(0.0f, -3.0f, 0.0f);

	if (playerPos.z < npcCenterPos.z || playerPos.z < npcLeftPos.z || playerPos.z < npcRightPos.z)
	{
		m_cubeFired = false;
	
		modelPlayer = translate(modelPlayer, glm::vec3(0.0f, -3.0f, 0.0f));
		if (playerPos.x <= -7 && playerPos.x >= -9 && playerPos.z <= -9 && playerPos.z >= -11)
		{
			m_cubeHit = true;
			m_score += 10;
			std::cout << "collision";
			modelPlayer = translate(modelPlayer, glm::vec3(0.0f, -3.0f, 0.0f));
		}

		else if (playerPos.x <= 1 && playerPos.x >= -1 && playerPos.z <= -9 && playerPos.z >= -11)
		{
			m_cubeHit = true;
			m_score += 10;
			std::cout << "collision";
			modelPlayer = translate(modelPlayer, glm::vec3(0.0f, -3.0f, 0.0f));
		}

		else if (playerPos.x <= 9 && playerPos.x >= 7 && playerPos.z <= -9 && playerPos.z >= -11)
		{
			m_cubeHit = true;
			m_score += 10;
			std::cout << "collision";
			modelPlayer = translate(modelPlayer, glm::vec3(0.0f, -3.0f, 0.0f));
		}
			playerPos = glm::vec3(playerPos.x, playerPos.y, 0.0f);
	}



} 


void Game::render()
{

#if (DEBUG >= 2)
	DEBUG_MSG("Render Loop...");
#endif

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Save current OpenGL render states
	// https://www.sfml-dev.org/documentation/2.0/classsf_1_1RenderTarget.php#a8d1998464ccc54e789aaf990242b47f7
	window.pushGLStates();

	// Find mouse position using sf::Mouse
	int x = Mouse::getPosition(window).x;
	int y = Mouse::getPosition(window).y; 
	string hud = "Lives: ["
		+ string(toString(m_lives))
		+ "]  Score: ["
		+ string(toString(m_score))
		+ "]";
	string gameOverText = "GAMEOVER";

	Text text(hud, font);
	Text gameover(gameOverText, font);
	text.setFillColor(sf::Color(255, 255, 255, 170));
	text.setPosition(50.f, 50.f);

	gameover.setFillColor(sf::Color(255, 255, 255, 170));
	gameover.setPosition(50.f, 50.f);

	window.draw(text);

	// Restore OpenGL render states
	// https://www.sfml-dev.org/documentation/2.0/classsf_1_1RenderTarget.php#a8d1998464ccc54e789aaf990242b47f7

	window.popGLStates();

	// Rebind Buffers and then set SubData
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vib);

	// Use Progam on GPU
	glUseProgram(progID);

	// Find variables within the shader
	// https://www.khronos.org/opengles/sdk/docs/man/xhtml/glGetAttribLocation.xml
	positionID = glGetAttribLocation(progID, "sv_position");
	if (positionID < 0) { DEBUG_MSG("positionID not found"); }

	colorID = glGetAttribLocation(progID, "sv_color");
	if (colorID < 0) { DEBUG_MSG("colorID not found"); }

	uvID = glGetAttribLocation(progID, "sv_uv");
	if (uvID < 0) { DEBUG_MSG("uvID not found"); }

	textureID = glGetUniformLocation(progID, "f_texture");
	if (textureID < 0) { DEBUG_MSG("textureID not found"); }

	mvpID[0] = glGetUniformLocation(progID, "sv_mvp");
	if (mvpID[0] < 0) { DEBUG_MSG("mvpID not found"); }

	mvpID[1] = glGetUniformLocation(progID, "sv_mvp");
	if (mvpID[1] < 0) { DEBUG_MSG("mvpID not found"); }

	mvpID[2] = glGetUniformLocation(progID, "sv_mvp");
	if (mvpID[2] < 0) { DEBUG_MSG("mvpID not found"); }

	mvpID[3] = glGetUniformLocation(progID, "sv_mvp");
	if (mvpID[3] < 0) { DEBUG_MSG("mvpID not found"); }

	x_offsetID = glGetUniformLocation(progID, "sv_x_offset");
	if (x_offsetID < 0) { DEBUG_MSG("x_offsetID not found"); }

	y_offsetID = glGetUniformLocation(progID, "sv_y_offset");
	if (y_offsetID < 0) { DEBUG_MSG("y_offsetID not found"); }

	z_offsetID = glGetUniformLocation(progID, "sv_z_offset");
	if (z_offsetID < 0) { DEBUG_MSG("z_offsetID not found"); };


	// VBO Data....vertices, colors and UV's appended
	glBufferSubData(GL_ARRAY_BUFFER, 0 * VERTICES * sizeof(GLfloat), 3 * VERTICES * sizeof(GLfloat), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * VERTICES * sizeof(GLfloat), 4 * COLORS * sizeof(GLfloat), colors);
	glBufferSubData(GL_ARRAY_BUFFER, ((3 * VERTICES) + (4 * COLORS)) * sizeof(GLfloat), 2 * UVS * sizeof(GLfloat), uvs);

	// Send transformation to shader mvp uniform [0][0] is start of array
	glUniformMatrix4fv(mvpID[0], 1, GL_FALSE, &mvpLeft[0][0]);

	// Set Active Texture .... 32 GL_TEXTURE0 .... GL_TEXTURE31
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(textureID, 0); // 0 .... 31

							   // Set the X, Y and Z offset (this allows for multiple cubes via different shaders)
							   // Experiment with these values to change screen positions
	glUniform1f(x_offsetID, 0.00f);
	glUniform1f(y_offsetID, 0.00f);
	glUniform1f(z_offsetID, 0.00f);

	// Set pointers for each parameter (with appropriate starting positions)
	// https://www.khronos.org/opengles/sdk/docs/man/xhtml/glVertexAttribPointer.xml
	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, (VOID*)(3 * VERTICES * sizeof(GLfloat)));
	glVertexAttribPointer(uvID, 2, GL_FLOAT, GL_FALSE, 0, (VOID*)(((3 * VERTICES) + (4 * COLORS)) * sizeof(GLfloat)));

	// Enable Arrays
	glEnableVertexAttribArray(positionID);
	glEnableVertexAttribArray(colorID);
	glEnableVertexAttribArray(uvID);

	// Draw Element Arrays
	glDrawElements(GL_TRIANGLES, 3 * INDICES, GL_UNSIGNED_INT, NULL);


	glBufferSubData(GL_ARRAY_BUFFER, 0 * VERTICES_NPC * sizeof(GLfloat), 3 * VERTICES_NPC * sizeof(GLfloat), VerticesNPC);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * VERTICES_NPC * sizeof(GLfloat), 4 * COLORS_NPC * sizeof(GLfloat), ColorsNPC);
	glBufferSubData(GL_ARRAY_BUFFER, ((3 * VERTICES_NPC) + (4 * COLORS_NPC)) * sizeof(GLfloat), 2 * UVS_NPC * sizeof(GLfloat), uvs);
	glUniformMatrix4fv(mvpID[1], 1, GL_FALSE, &mvpCenter[0][0]);
	glDrawElements(GL_TRIANGLES, 3 * INDICES_NPC, GL_UNSIGNED_INT, NULL);



	glBufferSubData(GL_ARRAY_BUFFER, 0 * VERTICES_NPC * sizeof(GLfloat), 3 * VERTICES_NPC * sizeof(GLfloat), VerticesNPC2);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * VERTICES_NPC * sizeof(GLfloat), 4 * COLORS_NPC * sizeof(GLfloat), ColorsNPC2);
	glBufferSubData(GL_ARRAY_BUFFER, ((3 * VERTICES_NPC) + (4 * COLORS_NPC)) * sizeof(GLfloat), 2 * UVS_NPC * sizeof(GLfloat), uvs);
	glUniformMatrix4fv(mvpID[2], 1, GL_FALSE, &mvpRight[0][0]);
	glDrawElements(GL_TRIANGLES, 3 * INDICES_NPC, GL_UNSIGNED_INT, NULL);


	glBufferSubData(GL_ARRAY_BUFFER, 0 * VERTICES_PLAYER * sizeof(GLfloat), 3 * VERTICES_PLAYER * sizeof(GLfloat), verticesPlayer);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * VERTICES_PLAYER * sizeof(GLfloat), 4 * COLORS_PLAYER * sizeof(GLfloat), colorsPlayer);
	glBufferSubData(GL_ARRAY_BUFFER, ((3 * VERTICES_PLAYER) + (4 * COLORS_PLAYER)) * sizeof(GLfloat), 2 * UVS_PLAYER * sizeof(GLfloat), uvs);
	glUniformMatrix4fv(mvpID[3], 1, GL_FALSE, &mvpPlayer[0][0]);
	glDrawElements(GL_TRIANGLES, 3 * INDICES_PLAYER, GL_UNSIGNED_INT, NULL);


	window.display();



	// Disable Arrays
	glDisableVertexAttribArray(positionID);
	glDisableVertexAttribArray(colorID);
	glDisableVertexAttribArray(uvID);

	// Unbind Buffers with 0 (Resets OpenGL States...important step)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Reset the Shader Program to Use
	glUseProgram(0);

	// Check for OpenGL Error code
	error = glGetError();
	if (error != GL_NO_ERROR) {
		DEBUG_MSG(error);
	}
}

void Game::unload()
{
#if (DEBUG >= 2)
	DEBUG_MSG("Cleaning up...");
#endif
	glDetachShader(progID, vsid);	// Shader could be used with more than one progID
	glDetachShader(progID, fsid);	// ..
	glDeleteShader(vsid);			// Delete Vertex Shader
	glDeleteShader(fsid);			// Delete Fragment Shader
	glDeleteProgram(progID);		// Delete Shader
	glDeleteBuffers(1, &vbo);		// Delete Vertex Buffer
	glDeleteBuffers(1, &vib);		// Delete Vertex Index Buffer
	stbi_image_free(img_data);		// Free image stbi_image_free(..)
}

//void Game::collision()
//{
////	glm::vec3 npcCenterPos = glm::vec3(0.0f, 3.0f, -4.0f);
//	if(playerPos.x <= -2 && playerPos.x >= -4 && playerPos.z <= -3 && playerPos.z >= -5)
//	{
//		window.close();
//		std::cout << "collision";
//	}
//}

void Game::shootCube()
{
	m_cubeFired = true;
	if (m_cubeFired)
	{
		modelPlayer = translate(modelPlayer, glm::vec3(0.0, 0.0, -0.2));
		playerPos.z += -0.2;
		m_cubeFired = false;
	}

}

