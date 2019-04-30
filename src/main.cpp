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
#include "SkyBox.h"
#include "WindowManager.h"
#include "GLTextureWriter.h"
#include "GameObject.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#define NUMOBJS 11
#define START_VELOCITY 0.0
#define PLAYER_VELOCITY .2
#define PLAYER_RADIUS 1.0
#define HEAD_RADIUS 2.0
#define WORLD_SIZE 100

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
	shared_ptr<SkyBox> skybox;

	//random num generators for position and direction generation
	int randXPos; //= rand() % 20;
	int randZPos; //= rand() % 20;
	float randXDir;
	float randZDir;// = ((float)rand() / (RAND_MAX)) + 1;

	class GamePlayer {
	private:
		shared_ptr<Shape> shape;
		float radius;
		vec3 rotation;
		vec3 position;
		vec3 direction;
		vec3 velocity;
		float camPhi;
		float camTheta;
		float camZoom;

	public:
		vec3 getPos() { return position; }
		vec3 getDir() { return direction; }
		vec3 getVel() { return velocity; }

		GamePlayer(shared_ptr<Shape> shape, vec3 position, vec3 direction, vec3 velocity) {
			this->shape = shape;
			this->radius = 1; //TODO assign this according to shape
			this->rotation = vec3(0, 0, 0);
			this->position = position;
			this->direction = direction;
			this->velocity = velocity;
			this->camPhi = 0;
			this->camTheta = 0;
			this->camZoom = -10;
		}

		// check collisions with a single point
		// note that this is a sphere
		bool isColliding(glm::vec3 point) {
			return length(position - point) < radius;
		}

		void draw(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack> Model){
			//player model
			Model->pushMatrix();
			Model->translate(position);
			Model->rotate(rotation.x, vec3(1, 0, 0));
    	Model->rotate(rotation.z, vec3(0, 0, 1));
			Model->rotate(rotation.y, vec3(0, 1, 0));
			 // offset for wobble
			Model->rotate(0.2, vec3(0, 0, 1));
			Model->translate(vec3(0.3, 0, 0));
			//material
			glUniform3f(prog->getUniform("matAmb"), 0.3294, 0.2235, 0.02745);
			glUniform3f(prog->getUniform("matDif"), 0.7804, 0.5686, 0.11373);
			//draw
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
			shape->draw(prog);
			Model->popMatrix();
		}

		/* moves the player and camera */
		//TODO this needs to be done real-time
		//TODO the View logic can probably be abstracted out
		vec3 update(std::shared_ptr<MatrixStack> View, bool *wasdIsDown, bool *arrowIsDown) {
			//TODO after implementing real-time, make these values constants
			float moveMagn = 0.01f; //player force
			//float mass = 1; // player mass
			float friction = 0.98f;
			float rotSpeed = 0.1f; // rotationSpeed

			//TODO implement drifty camera too
			float cameraSpeed = 0.02f; //camera rotation acceleration

			rotation += vec3(0, rotSpeed, 0);

			// camera rotation
			if (arrowIsDown[0]) camPhi =  std::max(camPhi - cameraSpeed, 0.0f); // no clipping thru the floor
			if (arrowIsDown[1]) camTheta -= cameraSpeed;
			if (arrowIsDown[2]) camPhi = std::min(camPhi + cameraSpeed, 1.56f); // no flipping the camera
			if (arrowIsDown[3]) camTheta += cameraSpeed;

			//player and camera orientation
			vec3 cameraForward = vec3(cos(camPhi) * -sin(camTheta),
	                            	sin(camPhi),
	                            	cos(camPhi) * -cos(camTheta));
			//TODO normalize?
			vec3 playerForward = normalize(vec3(-sin(camTheta),
	                            	0,
	                            	-cos(camTheta)));
			vec3 playerLeft = normalize(cross(playerForward, vec3(0, 1, 0)));

			//NOTE generally, vec3 velocity += (vec3 acceleration * float timePassed)

			vec3 zAccel = playerForward * moveMagn;
			vec3 xAccel = playerLeft * moveMagn;

			//player velocity

			vec3 acceleration = vec3(0, 0, 0);


			//player controls
			if (wasdIsDown[0]) acceleration -= zAccel;
	  	if (wasdIsDown[1]) acceleration -= xAccel;
	  	if (wasdIsDown[2]) acceleration += zAccel;
	  	if (wasdIsDown[3]) acceleration += xAccel;

			//acceleration = normalize(acceleration) * moveMagn / mass;
			//TODO gravity
			//TODO more accurate friction
			velocity *= friction;// ""friction""
			velocity += acceleration;


			position += velocity;

			//NOTE this was for updating the collision box
			/*minx = position.x - HEAD_RADIUS;
			minz = position.z - HEAD_RADIUS;
			maxx = position.x + HEAD_RADIUS;
			maxz = position.z + HEAD_RADIUS;*/

			// place the camera, pointed at the player
			//TODO this can be abstracted out if needed
			vec3 cameraPos = position - (cameraForward * camZoom);
			View->lookAt(cameraPos, position, vec3(0, 1, 0));

			return position;
		}
	};


	vector<GameObject> generateObjs(std::shared_ptr<Shape> shape) {
		vector<GameObject> gameObjs;
		for (int i = 0; i < NUMOBJS; i++) {
			randXPos = (((float)rand() / (RAND_MAX)) * WORLD_SIZE * 2) - WORLD_SIZE;
			randZPos = (((float)rand() / (RAND_MAX)) * WORLD_SIZE * 2) - WORLD_SIZE;
			randXDir = (((float)rand() / (RAND_MAX)) * 2) - 1;
			randZDir = (((float)rand() / (RAND_MAX)) * 2) - 1;
			GameObject obj = GameObject(vec3(randXPos, 0.f, randZPos), vec3(randXDir, 0.f, randZDir), START_VELOCITY, shape, prog);
			gameObjs.push_back(obj);
		}

		return gameObjs;
	}

	vector<GameObject> gameObjs;

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

	vec3 playerPos; //player position, TODO replace with the one in player

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
			 "rt.tga",
			 "lf.tga",
			 "up.tga",
			 "sist_dn.tga",
			 "bk.tga",
			 "ft.tga"
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
		prog->setShaderNames(
			resourceDirectory + "/point_vert.glsl",
			resourceDirectory + "/point_frag.glsl");
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
		cowShape->loadMesh(resourceDirectory + "/Nefertiti-10K.obj");
		cowShape->resize();
		cowShape->init();

		//initialize skybox
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

		//TODO: stuff doesn't move, call checking collisions and behavior if there is one
		for (uint i = 0; i < gameObjs.size(); i++) {
			GameObject cur = gameObjs[i];
			//vec3 rot = cur.getRot();
			//cur.isColliding(gameObjs, player);
			if (player->isColliding(cur.getPos())) {
				cur.destroy();
			}

			cur.draw(prog, Model);
		}

		//ground
		Model->pushMatrix();
		Model->translate(vec3(0, -1, 0));
		Model->scale(vec3(WORLD_SIZE, 0, WORLD_SIZE));
		SetMaterial(4);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
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
