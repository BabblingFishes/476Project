/*
Base code
Currently will make 2 FBOs and textures (only uses one in base code)
and writes out frame as a .png (Texture_output.png)

Winter 2017 - ZJW (Piddington texture write)
2017 integration with pitch and yaw camera lab (set up for texture mapping lab)
*/

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <time.h>
#include <stdlib.h>
#include <chrono>
#include <ctime>
#include <ratio>

#include "stb_image.h"

#include "Shader.h"
#include "Model.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "SkyBox.h"
#include "WindowManager.h"
#include "GLTextureWriter.h"
#include "GameObject.h"
#include "GamePlayer.h"
#include "GOCow.h"

// value_ptr for glm
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#define NUMOBJS 11
#define START_VELOCITY 0.0
#define PLAYER_VELOCITY .2
#define PLAYER_RADIUS 1.0
#define HEAD_RADIUS 2.0
#define WORLD_SIZE 100
#define MAP_WIDTH 120
#define MAP_LENGTH 162

using namespace std;
using namespace glm;
using namespace std::chrono;

class Application : public EventCallbacks {
public:
	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> texProg;
	std::shared_ptr<Program> skyProg;

	//timers for time based movement
	time_t startTime;
	time_t endTime;
	high_resolution_clock::time_point t1;
	high_resolution_clock::time_point t2;

	// Shape to be used (from obj file)
	shared_ptr<Shape> cowShape;
	shared_ptr<Shape> playerShape;
	shared_ptr<Shape> cube;
	//Model cube;
	shared_ptr<Shape> tree;
	shared_ptr<Shape> sphere;
  
	shared_ptr<SkyBox> skybox;

	//random num generators for position and direction generation
	int randXPos; //= rand() % 20;
	int randZPos; //= rand() % 20;
	float randXDir;
	float randZDir;// = ((float)rand() / (RAND_MAX)) + 1;




	vector<GOCow> generateObjs(std::shared_ptr<Shape> shape) {
		vector<GOCow> gameObjs;
		for (int i = 0; i < NUMOBJS; i++) {
			/*
			randXPos = (((float)rand() / (RAND_MAX)) * WORLD_SIZE * 2) - WORLD_SIZE;
			randZPos = (((float)rand() / (RAND_MAX)) * WORLD_SIZE * 2) - WORLD_SIZE;
			randRot = (((float)rand() / (RAND_MAX)) * pi);
			GOCow obj = GOCow(shape,
				2.0, //radius
				vec3(randXPos, 0, randZPos),
				vec3(0, randRot, 0),
				vec3(1, 1, 1),
				vec3(1, 0, 1));
				*/
			gameObjs.push_back(GOCow(shape, WORLD_SIZE));
		}

		return gameObjs;
	}

    vector<GameObject> generateMap(std::shared_ptr<Shape> shape) {
        vector<GameObject> mapObjs;
        int xPos, zPos;

        for (int i = -MAP_LENGTH / 2; i <= MAP_LENGTH / 2; i += 4) {
            if (i < -MAP_WIDTH / 2 || i > MAP_WIDTH / 2) {
                zPos = i;
                xPos = (-MAP_WIDTH / 2);
                GameObject obj1 = GameObject(shape, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
                mapObjs.push_back(obj1);
                xPos = (MAP_WIDTH / 2);
                GameObject obj2 = GameObject(shape, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
                mapObjs.push_back(obj2);
            }
            else {
                zPos = i;
                xPos = -MAP_WIDTH / 2;
                GameObject obj1 = GameObject(shape, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
                mapObjs.push_back(obj1);
                xPos = MAP_WIDTH / 2;
                GameObject obj2 = GameObject(shape, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
                mapObjs.push_back(obj2);

                xPos = i;
                zPos = -MAP_LENGTH / 2;
                GameObject obj3 = GameObject(shape, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
                mapObjs.push_back(obj3);
                zPos = MAP_LENGTH / 2;
                GameObject obj4 = GameObject(shape, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
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
                GameObject obj5 = GameObject(shape, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
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
            GameObject obj6 = GameObject(shape, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
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
            GameObject obj7 = GameObject(shape, 1, vec3(xPos, 0.f, zPos), vec3(0), vec3(5.f), vec3(0));
            mapObjs.push_back(obj7);
        }

        return mapObjs;
    }

  vector<GameObject> mapObjs;
	vector<GOCow> gameObjs;

	WindowManager * windowManager = nullptr;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//geometry for texture render
	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;

	unsigned int cubeMapTexture;

	//reference to texture FBO
	GLuint frameBuf[2];
	GLuint texBuf[2];
	GLuint depthBuf;

	bool FirstTime = true;
	bool Moving = false;
	int gMat = 0;

	//vector for x, z positions for robot
	vector<float> xPositionsRobot = { 14, 13, -19, 17, 5, 9, 17, -7, -10, -15 };
	vector<float> zPositionsRobot = { 4, 19, -18, -8, -5, 11, -6, 13, 8, -22 };

	float cTheta = 0;
	bool mouseDown = false;
	bool wasdIsDown[4] = { false };
	bool arrowIsDown[4] = { false };

	float theta = 0;
	float phi = 0;

	float zoom = -10;

	double mouseXPrev = 0;
	double mouseYPrev = 0;

    vec3 playerPos;// = vec3(45, 0, -60); //player position, TODO replace with the one in player

	const float PI = 3.14159;
	GamePlayer *player = nullptr;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {

		if (action == GLFW_PRESS || action == GLFW_RELEASE) {
			// movement
  		switch (key) {
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
        case GLFW_KEY_B: //display normally
  				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  				break;
        case GLFW_KEY_N: //display wireframe
  				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  				break;
  			}
      }
    }
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
		cTheta += (float)deltaX;
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods) {
		double posX = 0, posY = 0;

		if (action == GLFW_PRESS)
		{
			mouseDown = true;
			glfwGetCursorPos(window, &posX, &posY);
			glfwGetCursorPos(window, &mouseXPrev, &mouseYPrev);
		}

		if (action == GLFW_RELEASE)
		{
			//Moving = false;
			mouseDown = false;
		}
	}

	void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
		/*
		if (mouseDown) {
			deltaX = xpos - mouseXPrev;
			deltaY = ypos - mouseYPrev;

			phi += float((deltaY * 0.001));
			if (phi > 0.8) {
				phi = 0.8;
			}

			if (phi < -0.8) {
				phi = -0.8;
			}

			theta += float((deltaX * 0.001));

			mouseXPrev = xpos;
			mouseYPrev = ypos;
		}
		*/
	}

	void resizeCallback(GLFWwindow *window, int width, int height) {
		glViewport(0, 0, width, height);
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

// Code to load in the three textures
void initTex(const std::string& resourceDirectory)
{
	 vector<std::string> faces {
			 "SkyEarlyDusk_Right.png",
			 "SkyEarlyDusk_Left.png",
			 "SkyEarlyDusk_Top.png",
			 "SkyEarlyDusk_Bottom.png",
			 "SkyEarlyDusk_Front.png",
			 "SkyEarlyDusk_Back.png"
	 };
	 cubeMapTexture = createSky(resourceDirectory + "/",  faces);
}

	/*** INITIALIZATIONS ***/

	void init(const std::string& resourceDirectory) {
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		GLSL::checkVersion();

		cTheta = 0;
		// Set background color.
		glClearColor(.12f, .34f, .56f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program.
		prog = make_shared<Program>();
		prog->setVerbose(true);

		//TODO this may have a negatives error
		prog->setShaderNames(
			resourceDirectory + "/point_vert_BP.glsl",
			resourceDirectory + "/point_frag_BP.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("matAmb");
		prog->addUniform("matDif");
		prog->addUniform("matSpec");
		prog->addUniform("shine");
		prog->addUniform("lightPos");
		prog->addUniform("lightClr");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");

		//create two frame buffer objects to toggle between
		glGenFramebuffers(2, frameBuf);
		glGenTextures(2, texBuf);
		glGenRenderbuffers(1, &depthBuf);
		createFBO(frameBuf[0], texBuf[0]);

		//set up depth necessary as rendering a mesh that needs depth test
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);

		//more FBO set up
		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers);

		//create another FBO so we can swap back and forth
		createFBO(frameBuf[1], texBuf[1]);
		//this one doesn't need depth

		//initialize textures
		initTex(resourceDirectory);

		//set up the shaders to blur the FBO just a placeholder pass thru now
		//next lab modify and possibly add other shaders to complete blur
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(
			resourceDirectory + "/pass_vert.glsl",
			resourceDirectory + "/tex_fragH.glsl");
		if (!texProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		texProg->addUniform("texBuf");
		texProg->addAttribute("vertPos");
		texProg->addUniform("dir");
		texProg->addUniform("mode");
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
		texProg->addUniform("randNum");

		// SkyBox
		skyProg = make_shared<Program>();
    skyProg->setVerbose(true);
    skyProg->setShaderNames(
        resourceDirectory + "/skybox_vert.glsl",
        resourceDirectory + "/skybox_frag.glsl");
    if (! skyProg->init()) {
        std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
        exit(1);
    }
    skyProg->addUniform("P");
    skyProg->addUniform("M");
    skyProg->addUniform("V");
    skyProg->addAttribute("vertPos");
	}

	void initGeom(const std::string& resourceDirectory) {

		// Initialize the obj mesh VBOs etc
		cowShape = make_shared<Shape>();
		cowShape->loadMesh(resourceDirectory + "/bunny.obj");
		cowShape->resize();
		cowShape->init();


    // Initialize the obj mesh VBOs etc
    tree = make_shared<Shape>();
    tree->loadMesh(resourceDirectory + "/tree.obj");
    tree->resize();
    tree->init();

    sphere = make_shared<Shape>();
    sphere->loadMesh(resourceDirectory + "/sphere.obj");
    sphere->resize();
    sphere->init();

		//initialize skybox
		//Model cube(resourceDirectory + "/cube.obj");
		cube = make_shared<Shape>();
		cube->loadMesh(resourceDirectory + "/cube.obj");
		cube->resize();
		cube->init();

		skybox = make_shared<SkyBox>();
		skybox->loadMesh(resourceDirectory + "/cube.obj");
		skybox->resize();
		skybox->init();

		playerShape = make_shared<Shape>();
		playerShape->loadMesh(resourceDirectory + "/cylinder_shell.obj");
		playerShape->resize();
		playerShape->init();

		player = new GamePlayer(playerShape, vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0));
		gameObjs = generateObjs(cowShape);
        mapObjs = generateMap(tree);
		//Initialize the geometry to render a quad to the screen
		initQuad();
	}

	/**** geometry set up for a quad *****/
	void initQuad()
	{
		//now set up a simple quad for rendering FBO
		glGenVertexArrays(1, &quad_VertexArrayID);
		glBindVertexArray(quad_VertexArrayID);

		static const GLfloat g_quad_vertex_buffer_data[] =
		{
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
	void createFBO(GLuint& fb, GLuint& tex)
	{
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

	// To complete image processing on the specificed texture
	// Right now just draws large quad to the screen that is texture mapped
	// with the prior scene image - next lab we will process
	void ProcessImage(GLuint inTex, int i)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, inTex);

		// example applying of 'drawing' the FBO texture - change shaders
		texProg->bind();
		glUniform1i(texProg->getUniform("texBuf"), 0);
		glUniform2f(texProg->getUniform("dir"), -1, 0);
		glUniform1f(texProg->getUniform("mode"), i % 2);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(0);
		texProg->unbind();
	}

	void renderScene(shared_ptr<MatrixStack> View, shared_ptr<MatrixStack> Model) {
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));

		Model->loadIdentity();
		//Model->rotate(radians(cTheta), vec3(0, 1, 0));

		//float dt = difftime(startTime, endTime);
		duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
		//double dt = time_span.count();

        Model->pushMatrix();
        for (uint i = 0; i < mapObjs.size(); i++) {
            GameObject cur = mapObjs[i];

            cur.draw(prog, Model, 1);
        }
        Model->popMatrix();

		//TODO: stuff doesn't move, call checking collisions and behavior if there is one
		for (uint i = 0; i < gameObjs.size(); i++) {
			GOCow *cur = &(gameObjs[i]);
			//cur.isColliding(gameObjs, player);
			if (cur->isColliding(player)) {
				cur->collide(player);
				player->collide(cur);
			}
			cur->update();
			cur->draw(prog, Model);
		}
        
        //UFO (Sphere for now, will change later)
        Model->pushMatrix();
            Model->translate(vec3(-20, 0, 20));
            Model->scale(vec3(15, 15, 15));
            glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
            sphere->draw(prog);
        Model->popMatrix();

		//ground
		Model->pushMatrix();
		Model->translate(vec3(0, -1, 0));
		Model->scale(vec3(WORLD_SIZE, 0, WORLD_SIZE));
		SetMaterial(4);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		Shader ourShader("../resources/model_vert.glsl", "../resources/modelfrag.glsl");
		//cube.Draw(ourShader)
		cube->draw(prog);
		Model->popMatrix();

		//player model
		player->draw(prog, Model);
	}

	void render()
	{
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* Leave this code to just draw the meshes alone */
		float aspect = width / (float)height;

		// Create the matrix stacks
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();
		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(45.0f, aspect, 0.01f, WORLD_SIZE * 5);
		//View for fps camera
		View->pushMatrix();
		player->update(View, wasdIsDown, arrowIsDown);

		//Draw our scene - two meshes - right now to a texture
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));

		//light
		glUniform3f(prog->getUniform("lightPos"), player->getPos().x, player->getPos().y, player->getPos().z);
		glUniform3f(prog->getUniform("lightClr"), 0.3, 0.3, 0.3);

		//time(&startTime);

		t1 = high_resolution_clock::now();

		glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[0]);
		Model->pushMatrix();
			renderScene(View, Model); //note: this was previously FBOView
		Model->popMatrix();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		Model->pushMatrix();
			renderScene(View, Model);
		Model->popMatrix();

		//time(&endTime);
		t2 = high_resolution_clock::now();
		prog->unbind();

		//draw the sky box
		skyProg->bind();
		glUniformMatrix4fv(skyProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(skyProg->getUniform("V"), 1, GL_FALSE,value_ptr(View->topMatrix()));
		mat4 ident(1.0);
		glDepthFunc(GL_LEQUAL);
		Model->pushMatrix();
				Model->loadIdentity();
				Model->rotate(radians(cTheta), vec3(0, 1, 0));
				Model->translate(vec3(0, 6.0, 0));
				Model->scale(WORLD_SIZE*2);
				glUniformMatrix4fv(skyProg->getUniform("M"), 1, GL_FALSE,value_ptr(Model->topMatrix()) );
				glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
				skybox->draw(texProg);
		glDepthFunc(GL_LESS);
		Model->popMatrix();
		skyProg->unbind();

		texProg->bind();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texBuf[0]);

		Projection->popMatrix();
		View->popMatrix();
	}

	// helper function to set materials for shading
	void SetMaterial(int i)
	{
		switch (i)
		{
			//shiny blue plastic
			case 0:
				glUniform3f(prog->getUniform("matAmb"), 0.02, 0.04, 0.2);
				glUniform3f(prog->getUniform("matDif"), 0.0, 0.16, 0.9);
				glUniform3f(prog->getUniform("matSpec"), 0.14, 0.2, 0.8);
				glUniform1f(prog->getUniform("shine"), 120.0);
				break;

				//flat grey
			case 1:
				glUniform3f(prog->getUniform("matAmb"), 0.13, 0.13, 0.14);
				glUniform3f(prog->getUniform("matDif"), 0.3, 0.3, 0.4);
				glUniform3f(prog->getUniform("matSpec"), 0.1, 0.1, 0.1);
				glUniform1f(prog->getUniform("shine"), 1.0);
				break;

				//brass
			case 2:
				glUniform3f(prog->getUniform("matAmb"), 0.3294, 0.2235, 0.02745);
				glUniform3f(prog->getUniform("matDif"), 0.7804, 0.5686, 0.11373);
				glUniform3f(prog->getUniform("matSpec"), 0.9922, 0.9412, 0.8078);
				glUniform1f(prog->getUniform("shine"), 27.90);
				break;

				//ruby
			case 3:
				glUniform3f(prog->getUniform("matAmb"), 0.1745f, 0.01175f, 0.01175f);
				glUniform3f(prog->getUniform("matDif"), 0.61424f, 0.04136f, 0.04136f);
				glUniform3f(prog->getUniform("matSpec"), 0.727811f, 0.626959f, 0.626959f);
				glUniform1f(prog->getUniform("shine"), 0.6);
				break;

			case 4: //ground color
				glUniform3f(prog->getUniform("matAmb"), 0.1913f, 0.0735f, 0.0225f);
				glUniform3f(prog->getUniform("matDif"), 0.7038f, 0.27048f, 0.0828f);
				glUniform3f(prog->getUniform("matSpec"), 0.256777f, 0.137622f, 0.086014f);
				glUniform1f(prog->getUniform("shine"), 12.8);
				break;
		}
	}
};

int main(int argc, char **argv)
{
	//set seed for rand operations
	srand(time(0));

	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(512, 512);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
