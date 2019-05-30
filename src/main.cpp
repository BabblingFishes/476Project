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
#include <chrono>
#include <ctime>
#include <ratio>

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
#include "VFC.h"

// value_ptr for glm
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#define DEBUG_MODE true
#define NUMOBJS 11
#define WORLD_SIZE 100
#define MAP_WIDTH 120
#define MAP_LENGTH 162

using namespace std;
using namespace glm;
using namespace std::chrono;

class Application : public EventCallbacks {
public:

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

	// Shape to be used (from obj file)
	Shape *cowShape;
	Shape *playerShape;
	Shape *cube;
	Shape *sphere;
  Shape *tree;

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
	vector<GameObject> mapObjs;
	vector<GOCow> gameObjs;



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

		//init GL programs
		initShadowMapping(resourceDirectory);
		initSkyBox(resourceDirectory);
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
		cowShape->loadMesh(resourceDirectory + "/bunny.obj");
		cowShape->resize();
		cowShape->init();

    // Initialize the obj mesh VBOs etc
    tree = new Shape();
    tree->loadMesh(resourceDirectory + "/tree.obj");
    tree->resize();
    tree->init();

		cube = new Shape();
		cube->loadMesh(resourceDirectory + "/cube.obj");
		cube->resize();
		cube->init();

		sphere = new Shape();
    sphere->loadMesh(resourceDirectory + "/sphere.obj");
    sphere->resize();
    sphere->init();

		skybox = new SkyBox();
		skybox->loadMesh(resourceDirectory + "/cube.obj");
		skybox->resize();
		skybox->init();

		playerShape = new Shape();
		playerShape->loadMesh(resourceDirectory + "/cylinder_shell.obj");
		playerShape->resize();
		playerShape->init();

		//TODO replace below defaultTex with textures
		ground = new Ground(cube, defaultTex, (float) WORLD_SIZE, (float) WORLD_SIZE);
		player = new GamePlayer(playerShape, defaultTex, vec3(10.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0));
		mothership = new GOMothership(sphere, defaultTex, 13, vec3(-20, 0, 20), vec3(0, 0, 0), vec3(15, 1, 15), NUMOBJS);
		gameObjs = generateCows(cowShape, defaultTex);
    mapObjs = generateMap(tree, defaultTex);
		initQuad(); //quad for VBO
	}

	// makes cows and places them randomly in the world
	vector<GOCow> generateCows(Shape *shape, Texture *texture) {
		vector<GOCow> cows;
		for (int i = 0; i < NUMOBJS; i++) {
			cows.push_back(GOCow(shape, texture, WORLD_SIZE - 40));
		}
		return cows;
	}


  vector<GameObject> generateMap(Shape *shape, Texture *texture) {
    vector<GameObject> mapObjs;
    int xPos, zPos;

    for (int i = -MAP_LENGTH / 2; i <= MAP_LENGTH / 2; i += 4) {
      if (i < -MAP_WIDTH / 2 || i > MAP_WIDTH / 2) {
        zPos = i;
        xPos = -MAP_WIDTH / 2;
        GameObject obj1 = GameObject(shape, texture, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
        mapObjs.push_back(obj1);
        xPos = MAP_WIDTH / 2;
        GameObject obj2 = GameObject(shape, texture, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
        mapObjs.push_back(obj2);
      }
      else {
        zPos = i;
        xPos = -MAP_WIDTH / 2;
        GameObject obj1 = GameObject(shape, texture, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
        mapObjs.push_back(obj1);
        xPos = MAP_WIDTH / 2;
        GameObject obj2 = GameObject(shape, texture, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
        mapObjs.push_back(obj2);

        xPos = i;
        zPos = -MAP_LENGTH / 2;
        GameObject obj3 = GameObject(shape, texture, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
        mapObjs.push_back(obj3);

        zPos = MAP_LENGTH / 2;
        GameObject obj4 = GameObject(shape, texture, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
        mapObjs.push_back(obj4);
      }
    }
		//Tree lines within border. Each for loop is a line
        for (int i = ((-MAP_WIDTH / 2) + 5); i < MAP_WIDTH / 2; i += 3) {
          if (i > -20 || i < -40) {
            xPos = i;
            if (i % 2 == 0) {
                zPos = -30 + 2;
            }
            else {
                zPos = -30 - 2;
            }
            GameObject obj5 = GameObject(shape, texture, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
            mapObjs.push_back(obj5);
          }
        }
        for (int i = ((-MAP_WIDTH / 2) + 5); i <= 0; i += 3) {
          xPos = i;
          if (i % 2 == 0) {
              zPos = 0 + 2;
          }
          else {
              zPos = 0 - 2;
          }
          GameObject obj6 = GameObject(shape, texture, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
          mapObjs.push_back(obj6);
        }
        for (int i = ((MAP_LENGTH / 2) - 25); i >= -5; i -= 3) {
          zPos = i;
          if (i % 2 == 0) {
              xPos = 0 + 2;
          }
          else {
              xPos = 0 - 2;
          }
          GameObject obj7 = GameObject(shape, texture, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
          mapObjs.push_back(obj7);
      }

    return mapObjs;
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
		player->update(wasdIsDown, arrowIsDown, timeScale);

		vector<GOCow>::iterator cur;
		for (cur = gameObjs.begin(); cur != gameObjs.end(); cur++) {
			//TODO mothership collision
			if (!cur->isCollected()) {
				if (cur->isColliding(mothership)) {
					mothership->collect(&*cur);
				}
				else if (cur->isColliding(player)) {
					cur->collide(player);
					player->collide(&*cur);
				}
				cur->update(timeScale);
			}
		}
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

/* V - camera view */
  void setView_OLD(shared_ptr<Program> curProg) {
		vec3 camPos = player->getCamPos();
		glUniform3f(shadowProg->getUniform("camPos"), camPos.x, camPos.y, camPos.z);
  	mat4 View = glm::lookAt(camPos, player->getPos(), vec3(0, 1, 0));
  	glUniformMatrix4fv(curProg->getUniform("V"), 1, GL_FALSE, value_ptr(View));
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

		vector<GameObject>::iterator objI;
		vector<GOCow>::iterator cowI;

		if (shadowTexture) {
			//mothership
			if(!ViewFrustCull(mothership->getPos(), mothership->getRadius(), CULL)) {
				mothership->getTexture()->bind(shadowTexture);
				mothership->draw(curProg, Model);
			}
			//ground
			ground->getTexture()->bind(shadowTexture);
			ground->draw(curProg, Model);
			//trees and such
			for(objI = mapObjs.begin(); objI != mapObjs.end(); objI++) {
				if(!ViewFrustCull(objI->getPos(), objI->getRadius(), CULL)) {
					objI->getTexture()->bind(shadowTexture);
			  	objI->draw(curProg, Model);
				}
			}
			//cows
			for(cowI = gameObjs.begin(); cowI != gameObjs.end(); cowI++) {
				if(!ViewFrustCull(cowI->getPos(), cowI->getRadius(), CULL)) {
					cowI->getTexture()->bind(shadowTexture);
			  	cowI->draw(curProg, Model);
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
			//ground
			ground->draw(curProg, Model);
			//trees and such
			for(objI = mapObjs.begin(); objI != mapObjs.end(); objI++) {
				if(!ViewFrustCull(objI->getPos(), objI->getRadius(), CULL)) {
			  	objI->draw(curProg, Model);
				}
			}
			//cows
			for(cowI = gameObjs.begin(); cowI != gameObjs.end(); cowI++) {
				if(!ViewFrustCull(cowI->getPos(), cowI->getRadius(), CULL)) {
			  	cowI->draw(curProg, Model);
				}
			}
			//player
			player->draw(curProg, Model);
		}
	}

	//TODO you deleted this code, dumbass. Put it back.
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

	void render() {
		if (shadowsEnabled) renderShadowDepth();
		renderSkyBox();
		renderScene();
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
	windowManager->init(512, 512);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	application->init(resourceDir);
	application->initTex(resourceDir);
	application->initGeom(resourceDir);

	float timeScale;
	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle())) {

		/*high_resolution_clock::time_point t1;
		high_resolution_clock::time_point t2;
		t1 = high_resolution_clock::now();
		printf(t1);*/

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

	// Quit program.
	windowManager->shutdown();
	return 0;
}
