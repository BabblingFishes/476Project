/*
Base code
Currently will make 2 FBOs and textures (only uses one in base code)
and writes out frame as a .png (Texture_output.png)

Winter 2017 - ZJW (Piddington texture write)
2017 integration with pitch and yaw camera lab (set up for texture mapping lab)
*/

#include <iostream>
#include <glad/glad.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <chrono>
#include <ctime>
#include <ratio>
#include <irrKlang.h>

//#include "math.h"
//#define GLM_ENABLE_EXPERIMENTAL
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "Texture.h"
#include "Material.h"
#include "SkyBox.h"
#include "WindowManager.h"
#include "GLTextureWriter.h"
#include "Ground.h"
#include "GameObject.h"
#include "GamePlayer.h"
#include "GOCow.h"
#include "GOMothership.h"
#include "GOBarn.h"
#include "GOBorder.h"
#include "GOTree.h"
#include "GOHaybale.h"
#include "VFC.h"
#include "Particle.h"
#include "QuadTree.h"

//gui
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

//sounds
#include <irrKlang.h>

// value_ptr for glm
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <algorithm>

#define DEBUG_MODE true

using namespace std;
using namespace glm;
using namespace std::chrono;
using namespace irrklang;

class Application : public EventCallbacks {
public:
	//ui stuff
	bool show_UI = true;
	float timer = 0;

	WindowManager * windowManager = nullptr;

	// shader programs
	shared_ptr<Program> skyProg;
	shared_ptr<Program> depthProg;
	shared_ptr<Program> shadowProg;

	// shadowmapping
	mat4 lightP;
	mat4 lightV;
	bool shadowsEnabled = true; //TODO set this elsewhere
	GLuint depthMapFBO;
	const GLuint SHADOWMAP_WIDTH = 1024, SHADOWMAP_HEIGHT = 1024;
	GLuint depthMap;

	//VFCing
	bool CULL = true;
	bool CULL_DEBUG = false;

	//particle stuff
	shared_ptr<Program> partprog;
	vector<shared_ptr<Particle>> particles;
	int numP = 100;
	GLfloat points[900];
	GLfloat pointColors[1200];
	GLuint pointsbuffer;
	GLuint colorbuffer;
	shared_ptr<Texture> ptexture;
	float t = 0.5f; //reset in init
	float h = 0.005f;
	vec3 g = vec3(0.0f, -0.01f, 0.0f);
	bool sparking = false;

	//cow and haybale counter for the mothership
	int numCows = 0;
	int numHay = 0;

	//map width and height
	int Mwidth, Mheight;

	shared_ptr<QuadTree> quadTree;

	//default positions
	vec3 playerPos;// = vec3(40.0, 0.0, -60.0);
	vec3 MSPos;// = vec3(-20.0, 0.0, 20.0);

	// Shape to be used (from obj file)
	Shape *cowShape;
	Shape* cowWalk1;
	Shape* cowWalk2;
	Shape* cowWalk3;
	Shape* cowWalk4;
	Shape* cowWalk5;
	Shape* cowWalk6;
	Shape* cowWalk7;
	Shape* cowWalk8;
	Shape* cowWalk9;
	Shape* cowWalk10;
	Shape* cowWalk[10] = { cowWalk1, cowWalk2, cowWalk3, cowWalk4,
		cowWalk5, cowWalk6, cowWalk7, cowWalk8, cowWalk9, cowWalk10};

	Shape *playerShape;
	Shape *hayShape;
	Shape *cube;
	Shape *msShape;
	Shape* msiShape;
	Shape *groundShape;
	Shape *sphere;
  Shape *treeShape;
	Shape *barnShape;

	Texture *defaultTex;

	SkyBox *skybox;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//geometry for texture render
	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;

	//skybox texture ID
	unsigned int cubeMapTexture;

	//reference to texture FBO
	GLuint frameBuf[2];
	GLuint texBuf[2];
	GLuint depthBuf;

	bool wasdIsDown[4] = { false };
	bool arrowIsDown[4] = { false };
	int displayMode = 0;

	GamePlayer *player = nullptr;
	Ground *ground;
	GOMothership *mothership;
	GOBarn *barn;

	vector<GOTree> treeObjs;
	vector<GOTree> btreeObjs;
	vector<GOCow> cowObjs;
	vector<GOHaybale> hayObjs;



	/**** UI CALLBACKS ****/

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {

		if (action == GLFW_PRESS || action == GLFW_RELEASE) {
  		switch (key) {
			// WASD movement controls
  		case GLFW_KEY_W:
  			wasdIsDown[0] = action == GLFW_PRESS;
  			break;
  		case GLFW_KEY_D:
  			wasdIsDown[1] = action == GLFW_PRESS;
  			break;
  		case GLFW_KEY_S:
  			wasdIsDown[2] = action == GLFW_PRESS;
  			break;
  		case GLFW_KEY_A:
  			wasdIsDown[3] = action == GLFW_PRESS;
  			break;
			// Arrow camera controls
			case GLFW_KEY_UP:
  			arrowIsDown[0] = action == GLFW_PRESS;
  			break;
  		case GLFW_KEY_RIGHT:
  			arrowIsDown[1] = action == GLFW_PRESS;
  			break;
  		case GLFW_KEY_DOWN:
  			arrowIsDown[2] = action == GLFW_PRESS;
  			break;
  		case GLFW_KEY_LEFT:
  			arrowIsDown[3] = action == GLFW_PRESS;
  			break;
  		}
			//TODO: add pausing and zooming in/out
      if (action == GLFW_PRESS) {
  			switch (key) {
  			case GLFW_KEY_ESCAPE:
  				glfwSetWindowShouldClose(window, GL_TRUE);
  				break;
				//switches between drawing solids, wireframes, and points for debugging
        case GLFW_KEY_M:
					if(DEBUG_MODE){
						displayMode = (displayMode + 1) % 3;
						switch (displayMode) {
						case 0:
							glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
							break;
						case 1:
							glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
							break;
						case 2:
							glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
							break;
						}
					}
  				break;
  			}
      }
    }
	}

	//mouse/touchpad scrolling callback
	//currently does nothing
	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
		return;
	}

	//mouse button callback
	//currently does nothing
	void mouseCallback(GLFWwindow *window, int button, int action, int mods) {
		return;
	}

	//mouse position callback
	//currently does nothing
	void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
		return;
	}

	//TODO ensure this doesn't fuck with the shadowmapping viewport size
	void resizeCallback(GLFWwindow *window, int width, int height) {
		glViewport(0, 0, width, height);
	}

	/*** INITIALIZATIONS ***/

	//load programs
	void init(const std::string& resourceDirectory) {
		//QUESTION what is this actually for? is it checking for graphics driver compatibility?
		GLSL::checkVersion();

		// Set background color (pink for debug, black otherwise)
		if (DEBUG_MODE) {
			glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
		}
		else {
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		}
		// Enable z-buffer test
		glEnable(GL_DEPTH_TEST);

		CHECKED_GL_CALL(glEnable(GL_BLEND));
		CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		CHECKED_GL_CALL(glPointSize(14.0f));

		//init GL programs
		initShadowMapping(resourceDirectory);
		initSkyBox(resourceDirectory);
		initParticleSystem(resourceDirectory);
	}

	//init map from editor
	void initMap(vector<GOCow> *cows, vector<GOTree> *btrees, vector<GOTree> *trees, vector<GOHaybale> *hay) {
		int scanned = 0;
		int bpp;
		unsigned char* rgb = stbi_load("../resources/Maps/Map2.png", &Mwidth, &Mheight, &bpp, 3);

		cout << endl << "Map width: " << Mwidth << endl;
		cout << "Map height: " << Mheight << endl;
		cout << "Area: " << Mheight * Mwidth << endl;
		cout << "Bytes per pixel: " << bpp << endl;

		int x = 0;
		int z = 0;
		int counter = 0;
		for (int i = 0; i < Mwidth * Mheight * bpp; i += bpp) {
			int r = int(rgb[i]);
			int g = int(rgb[i + 1]);
			int b = int(rgb[i + 2]);
			int a = int(rgb[i + 3]);
			if (counter > Mwidth - 1) {
				counter = 0;
				x = 0;
				z++;
			}

			//cout << r << " " << g << " " << b << " x:" << x << " z:" << z << endl;

			char* current = RGBtoOBJ(r, g, b);
			//cout << current << " x:" << x << " z:" << z << endl;

			//random offsets
			float xRand = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 1.5)) - 0.75;
			float zRand = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 1.5)) - 0.75;

			if (strcmp(current, "bordertree") == 0) {
				btrees->push_back(GOTree(treeShape, defaultTex, 1, vec3(-x + xRand, 0 , z + zRand), vec3(0, 180 * xRand, 0), vec3(5 + xRand + zRand, 5 + zRand, 5 + xRand)));
			}
			else if (strcmp(current, "innertree") == 0) {
				trees->push_back(GOTree(treeShape, defaultTex, 1, vec3(-x + xRand, 0, z + zRand), vec3(0, 180 * xRand, 0), vec3(5.f + zRand)));
			}
			else if (strcmp(current, "cow") == 0) {
				numCows++;
				cows->push_back(GOCow(cowShape, defaultTex, -x + xRand, z + zRand, cowWalk));
			}
			else if (strcmp(current, "haybale") == 0) {
				numHay++;
				hay->push_back(GOHaybale(hayShape, defaultTex, -x + xRand, z + zRand));
			}
			else if (strcmp(current, "player") == 0) {
				player->setPos(vec3(-x, 10, z)); //boing
			}
			else if (strcmp(current, "mothership") == 0) {
				mothership->setPos(vec3(-x, -0.5, z));
			}
			else if (strcmp(current, "barn") == 0) {
				barn->setPos(vec3(-x, 0, z));
			}

			/*if (strcmp(current, "empty") != 0) {
				cout << current << " at x:" << x << " y:" << z << endl;
			}*/

			//arr[x][z][0] = r;
			//arr[x][z][1] = g;
			//arr[x][z][2] = b;
			//cout << endl << "x: " << x << "\ty: " << z << "\tr: " << r << "\tg:" << g << "\tb: " << b << endl;
			x++;
			counter++;
			scanned++;
		}


		//quadtree buffer border
		float border = 20; //TODO 50

		cout << "Quadtree bounds: x: " << -Mwidth - border << " -> " << 0 + border << " y: " << 0 - border << " -> " << Mheight + border << endl;

		//quadTree = make_shared<QuadTree>(vec2(-2000, -2000), vec2(2000, 2000));
		quadTree = make_shared<QuadTree>(vec2(-Mwidth - border, 0 - border), vec2(0 + border, Mheight + border));
	}

  //Gives the obj based on the RGB value
  char* RGBtoOBJ(int R, int G, int B) {
    //border trees
    if (R == 0 && G == 150 && B == 0) { return "bordertree"; }
		//inner trees
		else if (R == 0 && G == 255 && B == 0) { return "innertree"; }
    //cow
    else if (R == 0 && G == 0 && B == 0) { return "cow"; }
		//hay bales
		else if (R == 255 && G == 150 && B == 0) { return "haybale"; }
		//barn
		else if (R == 150 && G == 50 && B == 0) { return "barn"; }
    //player start position
    else if (R == 0 && G == 0 && B == 255) { return "player"; }
    //mothership
    else if (R == 255 && G == 0 && B == 0) { return "mothership"; }
		//none
		else { return "empty"; }
  }

	// initializes skybox program
	void initSkyBox(const std::string& resourceDirectory) {
		//init the skybox program
		skyProg = make_shared<Program>();
		skyProg->setVerbose(true);
		skyProg->setShaderNames(
				resourceDirectory + "/skybox_vert.glsl",
				resourceDirectory + "/skybox_frag.glsl");
		if (! skyProg->init()) {
				std::cerr << "Skybox shaders failed to compile... exiting!" << std::endl;
				exit(1);
		}
		skyProg->addUniform("P");
		skyProg->addUniform("M");
		skyProg->addUniform("V");
		skyProg->addAttribute("vertPos");
	}

	// initializes shadowmapping program
	void initShadowMapping(const std::string& resourceDirectory) {
		depthProg = make_shared<Program>();
		depthProg->setVerbose(true);
		depthProg->setShaderNames(
			resourceDirectory + "/depth_vert.glsl",
			resourceDirectory + "/depth_frag.glsl");
		if (!depthProg->init()) {
			std::cerr << "Depth shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}

		depthProg->addUniform("LP");
		depthProg->addUniform("LV");
		depthProg->addUniform("M");
		depthProg->addAttribute("vertPos");
		depthProg->addAttribute("vertNor"); //req'd by shape
		depthProg->addAttribute("vertTex"); //req'd by shape
		depthProg->addUniform("matAmb"); //red'd by objects
		depthProg->addUniform("matDif"); //red'd by objects
		depthProg->addUniform("matSpec"); //red'd by objects
		depthProg->addUniform("shine"); //red'd by objects


		shadowProg = make_shared<Program>();
		shadowProg->setVerbose(true);
		shadowProg->setShaderNames(resourceDirectory + "/shadow_vert_BP.glsl", resourceDirectory + "/shadow_frag_BP.glsl");
		if (!shadowProg->init()) {
			std::cerr << "Shadow shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}

		shadowProg->addUniform("P");
		shadowProg->addUniform("M");
		shadowProg->addUniform("V");
		shadowProg->addUniform("matAmb");
		shadowProg->addUniform("matDif");
		shadowProg->addUniform("matSpec");
		shadowProg->addUniform("shine");
		shadowProg->addUniform("LS");
		shadowProg->addUniform("camPos");
		shadowProg->addUniform("lightPos");
		shadowProg->addUniform("lightClr");
		shadowProg->addAttribute("vertPos");
		shadowProg->addAttribute("vertNor");
		shadowProg->addAttribute("vertTex");
		shadowProg->addUniform("Texture0");
		shadowProg->addUniform("shadowDepth");

		initDepthBuffer();
	}

	// initializes FBO for shadowmapping depth map
	void initDepthBuffer() {
		//generate the FBO for the shadow depth
  	glGenFramebuffers(1, &depthMapFBO);

		//generate the texture
  	glGenTextures(1, &depthMap);
  	glBindTexture(GL_TEXTURE_2D, depthMap);
  	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT,
  		0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//bind with framebuffer's depth buffer
  	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
  	glDrawBuffer(GL_NONE);
  	glReadBuffer(GL_NONE);
  	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// loads & initializes skybox textures
	void initTex(const std::string& resourceDirectory) {
		vector<std::string> faces {
			 "SkyMidNight_Right.png",
			 "SkyMidNight_Left.png",
			 "SkyMidNight_Top.png",
			 "SkyMidNight_Bottom.png",
			 "SkyMidNight_Front.png",
			 "SkyMidNight_Back.png"
		};
		cubeMapTexture = createSky(resourceDirectory + "/",  faces);

		defaultTex = new Texture();
		defaultTex->setFilename(resourceDirectory + "/grass.jpg");
		defaultTex->init();
		defaultTex->setUnit(0);
		defaultTex->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}

	void initParticleSystem(const std::string& resourceDirectory)
	{
		partprog = make_shared<Program>();
		partprog->setVerbose(true);
		partprog->setShaderNames(
			resourceDirectory + "/part_vert.glsl",
			resourceDirectory + "/part_frag.glsl");
		if (!partprog->init())
		{
			std::cerr << "One or more particle shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		partprog->addUniform("P");
		partprog->addUniform("M");
		partprog->addUniform("V");
		partprog->addUniform("alphaTexture");
		partprog->addAttribute("vertPos");
	}

	void initParticles(const std::string& resourceDirectory)
	{
		ptexture = make_shared<Texture>();
		ptexture->setFilename(resourceDirectory + "/sparkalpha.bmp");
		ptexture->init();
		ptexture->setUnit(0);
		ptexture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		int n = numP;

		for (int i = 0; i < n; ++i)
		{
			auto particle = make_shared<Particle>();
			particles.push_back(particle);
			particle->load();
		}

		// generate the VAO
		CHECKED_GL_CALL(glGenVertexArrays(1, &VertexArrayID));
		CHECKED_GL_CALL(glBindVertexArray(VertexArrayID));

		// generate vertex buffer to hand off to OGL - using instancing
		CHECKED_GL_CALL(glGenBuffers(1, &pointsbuffer));
		// set the current state to focus on our vertex buffer
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer));
		// actually memcopy the data - only do this once
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW));

		CHECKED_GL_CALL(glGenBuffers(1, &colorbuffer));
		// set the current state to focus on our vertex buffer
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
		// actually memcopy the data - only do this once
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW));
	}

	unsigned int createSky(string dir, vector<string> faces) {
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(false);
		for(GLuint i = 0; i < faces.size(); i++) {
			 unsigned char *data =
			 stbi_load((dir+faces[i]).c_str(), &width, &height, &nrChannels, 0);
			 if (data) {
					 glTexImage2D(
							 GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
							 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			 } else {
					 std::cout << "failed to load: " << (dir+faces[i]).c_str() << std::endl;
			 }
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		return textureID;
	}

	// Load geometry
	void initGeom(const std::string& resourceDirectory) {

		// Initialize the obj mesh VBOs etc
		cowShape = new Shape();
		cowShape->loadMesh(resourceDirectory + "/Models/Cow.obj");
		cowShape->resize();
		cowShape->init();

		cowWalk1 = new Shape();
		cowWalk1->loadMesh(resourceDirectory + "/Models/CowWalk/step1.obj");
		cowWalk1->resize();
		cowWalk1->init();
		cowWalk[0] = cowWalk1;

		cowWalk2 = new Shape();
		cowWalk2->loadMesh(resourceDirectory + "/Models/CowWalk/step2.obj");
		cowWalk2->resize();
		cowWalk2->init();
		cowWalk[1] = cowWalk2;

		cowWalk3 = new Shape();
		cowWalk3->loadMesh(resourceDirectory + "/Models/CowWalk/step3.obj");
		cowWalk3->resize();
		cowWalk3->init();
		cowWalk[2] = cowWalk3;

		cowWalk4 = new Shape();
		cowWalk4->loadMesh(resourceDirectory + "/Models/CowWalk/step4.obj");
		cowWalk4->resize();
		cowWalk4->init();
		cowWalk[3] = cowWalk4;

		cowWalk5 = new Shape();
		cowWalk5->loadMesh(resourceDirectory + "/Models/CowWalk/step5.obj");
		cowWalk5->resize();
		cowWalk5->init();
		cowWalk[4] = cowWalk5;

		cowWalk6 = new Shape();
		cowWalk6->loadMesh(resourceDirectory + "/Models/CowWalk/step6.obj");
		cowWalk6->resize();
		cowWalk6->init();
		cowWalk[5] = cowWalk6;

		cowWalk7 = new Shape();
		cowWalk7->loadMesh(resourceDirectory + "/Models/CowWalk/step7.obj");
		cowWalk7->resize();
		cowWalk7->init();
		cowWalk[6] = cowWalk7;

		cowWalk8 = new Shape();
		cowWalk8->loadMesh(resourceDirectory + "/Models/CowWalk/step8.obj");
		cowWalk8->resize();
		cowWalk8->init();
		cowWalk[7] = cowWalk8;

		cowWalk9 = new Shape();
		cowWalk9->loadMesh(resourceDirectory + "/Models/CowWalk/step9.obj");
		cowWalk9->resize();
		cowWalk9->init();
		cowWalk[8] = cowWalk9;

		cowWalk10 = new Shape();
		cowWalk10->loadMesh(resourceDirectory + "/Models/CowWalk/step10.obj");
		cowWalk10->resize();
		cowWalk10->init();
		cowWalk[9] = cowWalk10;

		hayShape = new Shape();
		hayShape->loadMesh(resourceDirectory + "/Models/roundedCube.obj");
		hayShape->resize();
		hayShape->init();

		// Initialize the obj mesh VBOs etc
		treeShape = new Shape();
		treeShape->loadMesh(resourceDirectory + "/Models/pinetree.obj");
		treeShape->resize();
		treeShape->init();

		barnShape = new Shape();
		barnShape->loadMesh(resourceDirectory + "/Models/Barn.obj");
		barnShape->resize();
		barnShape->init();

		groundShape = new Shape();
		groundShape->loadMesh(resourceDirectory + "/Models/ground.obj");
		groundShape->resize();
		groundShape->init();

		cube = new Shape();
		cube->loadMesh(resourceDirectory + "/Models/cube.obj");
		cube->resize();
		cube->init();

		msShape = new Shape();
		msShape->loadMesh(resourceDirectory + "/Models/mothership.obj");
		msShape->resize();
		msShape->init();

		msiShape = new Shape();
		msiShape->loadMesh(resourceDirectory + "/Models/mothershipinterior.obj");
		msiShape->resize();
		msiShape->init();

		skybox = new SkyBox();
		skybox->loadMesh(resourceDirectory + "/Models/cube.obj");
		skybox->resize();
		skybox->init();

		playerShape = new Shape();
		playerShape->loadMesh(resourceDirectory + "/Models/UFO.obj");
		playerShape->resize();
		playerShape->init();

		//TODO replace below defaultTex with textures
		player = new GamePlayer(playerShape, defaultTex, vec3(0.0), vec3(0.0, -2.0, 0.0), vec3(1.0));
		mothership = new GOMothership(msShape, defaultTex, 10, vec3(0.0), vec3(0.0), vec3(10.0), numCows, numHay);
		barn = new GOBarn(barnShape, defaultTex, vec3(0.0), vec3(0.0), vec3(5.0));
		initMap(&cowObjs, &btreeObjs, &treeObjs, &hayObjs);
		ground = new Ground(groundShape, defaultTex, (float)Mwidth, (float)Mheight);
		initQuad(); //quad for VBO

		//cout << "MS x: " << mothership->getPos().x << " y: " << mothership->getPos().y << " z: " << mothership->getPos().z << endl; //DEBUG
		//cout << "BARN x: " << barn->getPos().x << " y: " << barn->getPos().y << " z: " << barn->getPos().z << endl; //DEBUG

		quadTree->addObject(player);
		//quadTree->addObject(mothership); TODO
		//quadTree->addObject(barn); TODO
		for(vector<GOCow>::iterator cur = cowObjs.begin(); cur != cowObjs.end(); cur++) {
			quadTree->addObject(&*cur);
	  }
		for(vector<GOTree>::iterator cur = btreeObjs.begin(); cur != btreeObjs.end(); cur++) {
			quadTree->addObject(&*cur);
	  }
		for(vector<GOTree>::iterator cur = treeObjs.begin(); cur != treeObjs.end(); cur++) {
			quadTree->addObject(&*cur);
	  }
		for(vector<GOHaybale>::iterator cur = hayObjs.begin(); cur != hayObjs.end(); cur++) {
			quadTree->addObject(&*cur);
	  }
	}


	// geometry set up for a quad
	void initQuad() {
		//now set up a simple quad for rendering FBO
		glGenVertexArrays(1, &quad_VertexArrayID);
		glBindVertexArray(quad_VertexArrayID);

		static const GLfloat g_quad_vertex_buffer_data[] = {
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f,  1.0f, 0.0f,
		};

		glGenBuffers(1, &quad_vertexbuffer);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
	}

	/* Helper function to create the framebuffer object and
	associated texture to write to */
	void createFBO(GLuint& fb, GLuint& tex) {
		//initialize FBO
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

		//set up framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, fb);
		//set up texture
		glBindTexture(GL_TEXTURE_2D, tex);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			cout << "Error setting up frame buffer - exiting" << endl;
			exit(0);
		}
	}


	/***** Updating Game State *****/

	//main update loop, called once per frame
	//TODO maybe pass a world state and handle collisions inside objs?
	void update(double timeScale) {
		player->doControls(wasdIsDown, arrowIsDown);

		quadTree->update(timeScale);

		player->positionCamera();

		//TODO mothership->collide(cow/bale) should collect
		//TODO cows->collide(mothership) should stop them moving
		// or delete them
		//TODO see above but for haybales
	}

	void updateParticles()
	{
		// update the particles
		for (auto particle : particles)
		{
			particle->update(t, h, g, player->getSparking());
		}
		t += h;

		// Sort the particles by Z
		auto temp = make_shared<MatrixStack>();
		temp->rotate(player->getCamTheta(), vec3(0, 1, 0));

		ParticleSorter sorter;
		sorter.C = temp->topMatrix();
		std::sort(particles.begin(), particles.end(), sorter);


		vec3 pos;
		vec4 col;
		// go through all the particles and update the CPU buffer
		for (int i = 0; i < numP; i++)
		{
			pos = particles[i]->getPosition();
			col = particles[i]->getColor();
			points[i * 3 + 0] = pos.x;
			points[i * 3 + 1] = pos.y;
			points[i * 3 + 2] = pos.z;
			pointColors[i * 4 + 0] = col.r + col.a / 10.f;
			pointColors[i * 4 + 1] = col.g + col.g / 10.f;
			pointColors[i * 4 + 2] = col.b + col.b / 10.f;
			pointColors[i * 4 + 3] = col.a;
		}
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW));
		CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 3, points));

		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW));
		CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 4, pointColors));

		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}


	/***** Rendering *****/

/* P - projection */
	void setProjectionMatrix_OLD(shared_ptr<Program> curProg) {
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		//glViewport(0, 0, width, height); ??
		float aspect = width/(float)height;
		mat4 Projection = perspective(radians(45.0f), aspect, 0.01f, 100.0f);
		glUniformMatrix4fv(curProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection));
	}

/* lightP - orthogonal projection for (directional?) light */
//TODO maybe use non-orthogonal as cone light?
  mat4 setOrthoMatrix(shared_ptr<Program> curProg) {
  	mat4 ortho = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 0.1f, 30.0f); //Z
		glUniformMatrix4fv(curProg->getUniform("LP"), 1, GL_FALSE, value_ptr(ortho)); //Z
  	return ortho;
  }

/* V - camera view without translation */
	void setSkyBoxView(shared_ptr<Program> curProg) {
		mat4 View = mat4(1.f);
		View *= glm::rotate(mat4(1.0), player->getCamPhi(), vec3(1, 0, 0));
		View *= glm::rotate(mat4(1.0), player->getCamTheta(), vec3(0, -1, 0));
		glUniformMatrix4fv(curProg->getUniform("V"), 1, GL_FALSE, value_ptr(View));
	}

/* lightV - view for light */
  mat4 setLightView(shared_ptr<Program> curProg, vec3 lightPos, vec3 lightAim, vec3 up) {
  	mat4 Cam = lookAt(lightPos, lightAim, up);
		glUniformMatrix4fv(curProg->getUniform("LV"), 1, GL_FALSE, value_ptr(Cam));
		return Cam;
  }

	void drawScene(shared_ptr<Program> curProg, GLint shadowTexture) {
		shared_ptr<MatrixStack> Model = make_shared<MatrixStack>(); //TODO the sharedptr is probably unnecessary

		if (shadowTexture) {
			//mothership
			//if(!ViewFrustCull(mothership->getPos(), mothership->getRadius(), CULL)) {
				mothership->getTexture()->bind(shadowTexture);
				mothership->draw(curProg, Model);
			//}
			//barn
			if (!ViewFrustCull(barn->getPos(), barn->getRadius(), CULL)) {
				barn->getTexture()->bind(shadowTexture);
				barn->draw(curProg, Model);
			}
			//ground
			ground->getTexture()->bind(shadowTexture);
			ground->draw(curProg, Model);

			//border trees
			for(vector<GOTree>::iterator btreeI = btreeObjs.begin(); btreeI != btreeObjs.end(); btreeI++) {
				if(!ViewFrustCull(btreeI->getPos(), btreeI->getRadius(), CULL)) {
					btreeI->getTexture()->bind(shadowTexture);
			  	btreeI->draw(curProg, Model);
				}
			}
			//inner trees
			for (vector<GOTree>::iterator treeI = treeObjs.begin(); treeI != treeObjs.end(); treeI++) {
				if (!ViewFrustCull(treeI->getPos(), treeI->getRadius(), CULL)) {
					treeI->getTexture()->bind(shadowTexture);
					treeI->draw(curProg, Model);
				}
			}
			//cows
			for(vector<GOCow>::iterator cowI = cowObjs.begin(); cowI != cowObjs.end(); cowI++) {
				if(!ViewFrustCull(cowI->getPos(), cowI->getRadius(), CULL)) {
					cowI->getTexture()->bind(shadowTexture);
					cowI->draw(curProg, Model);
				}
			}
			//hay
			for (vector<GOHaybale>::iterator hayI = hayObjs.begin(); hayI != hayObjs.end(); hayI++) {
				if (!ViewFrustCull(hayI->getPos(), hayI->getRadius(), CULL)) {
					hayI->getTexture()->bind(shadowTexture);
					hayI->draw(curProg, Model);
				}
			}
			//player
			player->getTexture()->bind(shadowTexture);
			player->draw(curProg, Model);
		}
		else {
			//mothership
			if(!ViewFrustCull(mothership->getPos(), mothership->getRadius(), CULL)) {
				mothership->draw(curProg, Model);
			}
			//barn
			if (!ViewFrustCull(barn->getPos(), barn->getRadius(), CULL)) {
				barn->draw(curProg, Model);
			}
			//ground
			ground->draw(curProg, Model);

			//bordertrees
			for(vector<GOTree>::iterator btreeI = btreeObjs.begin(); btreeI != btreeObjs.end(); btreeI++) {
				if(!ViewFrustCull(btreeI->getPos(), btreeI->getRadius(), CULL)) {
					btreeI->draw(curProg, Model);
				}
			}
			//inner trees
			for (vector<GOTree>::iterator treeI = treeObjs.begin(); treeI != treeObjs.end(); treeI++) {
				if (!ViewFrustCull(treeI->getPos(), treeI->getRadius(), CULL)) {
					treeI->draw(curProg, Model);
				}
			}
			//cows
			for(vector<GOCow>::iterator cowI = cowObjs.begin(); cowI != cowObjs.end(); cowI++) {
				if(!ViewFrustCull(cowI->getPos(), cowI->getRadius(), CULL)) {
			  	cowI->draw(curProg, Model);
				}
			}
			//hay
			for (vector<GOHaybale>::iterator hayI = hayObjs.begin(); hayI != hayObjs.end(); hayI++) {
				if (!ViewFrustCull(hayI->getPos(), hayI->getRadius(), CULL)) {
					hayI->draw(curProg, Model);
				}
			}
			//player
			player->draw(curProg, Model);
		}
	}

	void renderSkyBox() {
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height); //view to window size
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear framebuffer

		skyProg->bind();
		setProjectionMatrix_OLD(skyProg);
		setSkyBoxView(skyProg);
		//mat4 ident(1.0); // TODO ????
		glDepthFunc(GL_LEQUAL);
		mat4 Model = mat4(1.f);
		//mat4 Model = glm::scale(mat4(1.f), vec3(WORLD_SIZE)); //TODO scale necessary?
		glUniformMatrix4fv(skyProg->getUniform("M"), 1, GL_FALSE,value_ptr(Model));
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
		skybox->draw(skyProg);
		glDepthFunc(GL_LESS);
		skyProg->unbind();
	}

	void renderShadowDepth() {
		//depth map setup
		glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

		//set up shadow shader
		//render scene
		depthProg->bind();
		lightP = setOrthoMatrix(depthProg);
		lightV = setLightView(depthProg, player->getPos() + vec3(0, 10, 0), player->getPos() - vec3(0, 1, 0), vec3(0, 1, 0)); //TODO we could even point this at the nearest cow for funsies
		drawScene(depthProg, 0);
		depthProg->unbind();

		//reset culling and unbind frame buffer for normal rendering
		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void renderScene() {
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height); //view to window size
		glClear(GL_DEPTH_BUFFER_BIT); //clear framebuffer

		//set up shadow shader
		shadowProg->bind();
		/* also set up light depth map */
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glUniform1i(shadowProg->getUniform("shadowDepth"), 1);
		//pass in light info
		glUniform3f(shadowProg->getUniform("lightPos"), player->getPos().x, player->getPos().y, player->getPos().z);
		glUniform3f(shadowProg->getUniform("lightClr"), 0.3f, 0.3f, 0.3f);
		//render scene
		mat4 P = SetProjectionMatrix(shadowProg, windowManager, width, height);
		mat4 V = SetView(shadowProg, player->getCamPos(), player->getPos());
		ExtractVFPlanes(P, V);

		mat4 lightS = lightP * lightV;
		glUniformMatrix4fv(shadowProg->getUniform("LS"), 1, GL_FALSE, value_ptr(lightS));
		//TODO: is there other uniform data that must be sent?

		drawScene(shadowProg, shadowProg->getUniform("Texture0"));
		shadowProg->unbind();
	}

	void renderParticles() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		auto M = make_shared<MatrixStack>();
		auto V = make_shared<MatrixStack>();

		M->pushMatrix();
		M->loadIdentity();

		V->pushMatrix();
		V->loadIdentity();

		partprog->bind();
		updateParticles();

		ptexture->bind(partprog->getUniform("alphaTexture"));
		setProjectionMatrix_OLD(partprog);
		CHECKED_GL_CALL(glUniformMatrix4fv(partprog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix())));
		CHECKED_GL_CALL(glUniformMatrix4fv(partprog->getUniform("V"), 1, GL_FALSE, value_ptr(V->topMatrix())));

		CHECKED_GL_CALL(glEnableVertexAttribArray(0));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer));
		CHECKED_GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0));

		CHECKED_GL_CALL(glEnableVertexAttribArray(1));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
		CHECKED_GL_CALL(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0));

		CHECKED_GL_CALL(glVertexAttribDivisor(0, 1));
		CHECKED_GL_CALL(glVertexAttribDivisor(1, 1));
		// Draw the points !
		CHECKED_GL_CALL(glDrawArraysInstanced(GL_POINTS, 0, 1, numP));

		CHECKED_GL_CALL(glVertexAttribDivisor(0, 0));
		CHECKED_GL_CALL(glVertexAttribDivisor(1, 0));
		CHECKED_GL_CALL(glDisableVertexAttribArray(0));
		CHECKED_GL_CALL(glDisableVertexAttribArray(1));
		partprog->unbind();
	}

	void renderGUI() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
		window_flags |= ImGuiWindowFlags_NoBackground;

		if (show_UI)
		{
			ImGui::Begin("Holy Cow", NULL, window_flags);
			ImGui::SetWindowFontScale(2.0f);
			int collCows = mothership->getCollectedCows();
			int collHay = mothership->getCollectedHay();

			ImGui::Text("Cows Collected: %d / %d", collCows, numCows);
			(ImGui::GetFontSize() * 100.0f);
			ImGui::Text("Hay Collected: %d", collHay);

			int cowpoints = collCows * 10;
			int haypoints = collHay * 7;
			int totalpoints = cowpoints - haypoints;
			ImGui::TextColored(ImVec4(1.0, 0.5, 0.5, 1.0), "Points earned: %d", totalpoints);
			//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(windowManager->getHandle(), &display_w, &display_h);
		glViewport(0, 0, display_w	, display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void render() {
		if (shadowsEnabled) renderShadowDepth();
		renderSkyBox();
		renderScene();
		renderGUI();
		renderParticles();
	}
};

int main(int argc, char **argv) {
	//set seed for rand operations
	srand(time(0));

	// Locate the resource directory
	// (By default, assume we are in the build directory)
	std::string resourceDir = "../resources";
	if (argc >= 2) {
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Establish window
	WindowManager *windowManager = new WindowManager();
	windowManager->init(1050, 1050);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	application->init(resourceDir);
	application->initTex(resourceDir);
	application->initParticles(resourceDir);
	application->initGeom(resourceDir);

	//gui stuff
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui_ImplGlfw_InitForOpenGL(windowManager->getHandle(), true);
	ImGui_ImplOpenGL3_Init("#version 330");
	ImGui::StyleColorsDark();

	//background music
	ISoundEngine* engine;

	float timeScale = 0;
	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle())) {
		auto start = std::chrono::steady_clock::now();

		// update game state
		application->update(timeScale);
		// render game
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();

		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start);
		timeScale = elapsed.count() / (10e+9);
		timeScale *= 500; //TODO this is adjusting to fish's crap speed, but we can just adjust other variables instead

		//cout << timeScale << endl; //DEBUG
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();

	// Quit program.
	windowManager->shutdown();
	return 0;
}
