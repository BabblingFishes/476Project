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


#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "Shape.h"
#include "WindowManager.h"
#include "GLTextureWriter.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#define NUMOBJS 11
#define START_VELOCITY 0.0
#define PLAYER_VELOCITY .2
#define PLAYER_RADIUS 1.0
#define HEAD_RADIUS 2.0

using namespace std;
using namespace glm;
using namespace std::chrono;

class Application : public EventCallbacks
{
public:
	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> texProg;

	//timers for time based movement
	time_t startTime;
	time_t endTime;
	high_resolution_clock::time_point t1;
	high_resolution_clock::time_point t2;

	// Shape to be used (from obj file)
	shared_ptr<Shape> shape;
	shared_ptr<Shape> shape2;
	shared_ptr<Shape> playerShape;

	//random num generators for position and direction generation
	int randXPos; //= rand() % 20;
	int randZPos; //= rand() % 20;
	float randXDir;
	float randZDir;// = ((float)rand() / (RAND_MAX)) + 1;

	class GamePlayer {
	private:
		vec3 position;
		vec3 direction;
		float velocity;
		float minx = position.x - PLAYER_RADIUS;
		float minz = position.z - PLAYER_RADIUS;
		float maxx = position.x + PLAYER_RADIUS;
		float maxz = position.z + PLAYER_RADIUS;

	public:
		vec3 getPos() { return position; }
		vec3 getDir() { return direction; }
		float getVel() { return velocity; }
		float getMinx() { return minx; }
		float getMinz() { return minz; }
		float getMaxx() { return maxx; }
		float getMaxz() { return maxz; }

		GamePlayer(vec3 position, vec3 direction, float velocity) {
			this->position = position;
			this->direction = direction;
			this->velocity = velocity;
		}

		vec3 update(int key, float theta) {
			switch (key) {
			case(1):
				position.x -= (float)cos(theta) * velocity;
				position.z -= (float)sin(theta) * velocity;
				break;
			case(2):
				position.x += (float)cos(theta) * velocity;
				position.z += (float)sin(theta) * velocity;
				break;
			case(3):
				position.z -= (float)cos(theta) * velocity;
				position.x += (float)sin(theta) * velocity;
				break;
			case(4):
				position.z += (float)cos(theta) * velocity;
				position.x -= (float)sin(theta) * velocity;
				break;
			}
			minx = position.x - HEAD_RADIUS;
			minz = position.z - HEAD_RADIUS;
			maxx = position.x + HEAD_RADIUS;
			maxz = position.z + HEAD_RADIUS;

			//cout << "pminx: " << minx << " pminz: " << minz << "pmaxx: " << maxx << "pmaxz: " << maxz << endl;

			vec3 newPos = position;
			return newPos;
		}
	};

	class GameObject {
	private:
		vec3 position;
		vec3 direction;
		vec3 rotation;
		float velocity;
		shared_ptr<Shape> shape;
		bool toDraw = false;
		bool collected = false;
		shared_ptr<Program> prog;
		// bounding box
		float minx = position.x - PLAYER_RADIUS;
		float minz = position.z - PLAYER_RADIUS;
		float maxx = position.x + PLAYER_RADIUS;
		float maxz = position.z + PLAYER_RADIUS;

	public:
		vec3 getPos() { return position; }
		vec3 getDir() { return direction; }
		vec3 getRot() { return rotation; }
		float getVel() { return velocity; }
		bool getDraw() { return toDraw; }
		bool getCollected() { return collected; }
		float getMinx() { return minx; }
		float getMinz() { return minz; }
		float getMaxx() { return maxx; }
		float getMaxz() { return maxz; }

		void setPos(vec3 pos) { position = pos; }

		GameObject(vec3 position, vec3 direction, float velocity, shared_ptr<Shape> shape, shared_ptr<Program> prog) {
			this->position = position;
			this->direction = direction;
			this->velocity = velocity;
			this->shape = shape;
			this->prog = prog;

			toDraw = false;
			rotation = vec3(asin(direction.x), 0.f, acos(direction.z));
			minx = position.x - HEAD_RADIUS;
			minz = position.z - HEAD_RADIUS;
			maxx = position.x + HEAD_RADIUS;
			maxz = position.z + HEAD_RADIUS;
		}

		//timer for spawning
		clock_t timer = clock();

		vec3 update(double dt) {
			vec3 move = direction * velocity * (float)dt;
			vec3 newPos = position + move;
			position = newPos;
			minx = position.x - HEAD_RADIUS;
			minz = position.z - HEAD_RADIUS;
			maxx = position.x + HEAD_RADIUS;
			maxz = position.z + HEAD_RADIUS;

			return newPos;
		}

		void draw() { shape->draw(prog); }
		void destroy() { toDraw = false; }
		//TODO: check collisions function(s)

		bool isColliding(vector<GameObject> gameObjs, GamePlayer p)
		{
			if ((p.getMaxx() > minx && p.getMaxz() > minz) || (p.getMinx() < maxx && p.getMinz() < maxz)){
				cout << "PLAYER COLLISION\n";
				return true;
			}
			for (uint i = 0; i < gameObjs.size(); i++) {
				GameObject cur = gameObjs[i];
				if ((cur.maxx < minx && cur.maxz < minz) || (cur.minx < maxx && cur.minz < maxz)) {
					cout << "Collision! between objects!";
					return true;
				}
			}
			return false;
		}
	};

	vector<GameObject> generateObjs() {
		vector<GameObject> gameObjs;
		for (int i = 0; i < NUMOBJS; i++) {
			randXPos = (((float)rand() / (RAND_MAX)) * 40) - 20;
			randZPos = (((float)rand() / (RAND_MAX)) * 40) - 20;
			randXDir = (((float)rand() / (RAND_MAX)) * 2) - 1;
			randZDir = (((float)rand() / (RAND_MAX)) * 2) - 1;
			GameObject obj = GameObject(vec3(randXPos, 0.f, randZPos), vec3(randXDir, 0.f, randZDir), START_VELOCITY, shape, prog);
			gameObjs.push_back(obj);
		}

		return gameObjs;
	}

	vector<GameObject> gameObjs = generateObjs();

	WindowManager * windowManager = nullptr;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our triangle to OpenGL
	GLuint VertexBufferID;

	//geometry for texture render
	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;

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
	GamePlayer player = GamePlayer(vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), PLAYER_VELOCITY);

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{

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

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{
		cTheta += (float)deltaX;
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
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

	void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
	{
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

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
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
			resourceDirectory + "/simple_vert.glsl",
			resourceDirectory + "/simple_frag.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("matAMB");
		prog->addUniform("matDIF");
		prog->addUniform("matSPEC");
		prog->addUniform("shine");
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
	}

	void initGeom(const std::string& resourceDirectory)
	{
		// Initialize the obj mesh VBOs etc
		shape = make_shared<Shape>();
		shape->loadMesh(resourceDirectory + "/Nefertiti-10K.obj");
		shape->resize();
		shape->init();

		//initialize
		shape2 = make_shared<Shape>();
		shape2->loadMesh(resourceDirectory + "/cube.obj");
		shape2->resize();
		shape2->init();

		playerShape = make_shared<Shape>();
		playerShape->loadMesh(resourceDirectory + "/cylinder_shell.obj");
		playerShape->resize();
		playerShape->init();

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
		double dt = time_span.count();

		//TODO: stuff doesn't move, call checking collisions and behavior if there is one
		for (uint i = 0; i < gameObjs.size(); i++) {
			GameObject cur = gameObjs[i];
			vec3 rot = cur.getRot();
			//cur.isColliding(gameObjs, player);
			Model->pushMatrix();
			//cout << i << "\n";
			//cout << "prev pos" << glm::to_string(cur.getPos());
			vec3 newPos = gameObjs[i].update(dt);
			Model->translate(newPos);
			//cout << "translated pos " << glm::to_string(newPos) << endl;
			Model->rotate((rot.z + rot.x) / 2.f, vec3(0, 1, 0));
			Model->rotate(radians(-90.f), vec3(1, 0, 0));
			SetMaterial(i % 4);
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
			shape->draw(prog);
			Model->popMatrix();
		}

		Model->pushMatrix();
		Model->translate(vec3(0, -1, 0));
		Model->scale(vec3(20, 0, 20));
		SetMaterial(4);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		shape2->draw(prog);
		Model->popMatrix();

		//player model
		Model->pushMatrix();
		Model->translate(playerPos);
		SetMaterial(2);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		playerShape->draw(prog);
		Model->popMatrix();
	}

	// handles player & camera movement
	//TODO this needs to be done real-time
	//TODO this needs acceleration for drift
	void playerCamMovement(std::shared_ptr<MatrixStack> View)
	{
		//TODO after implementing real-time, make these two values global constants
		float playerSpeed = 0.1f; //player speed
		float cameraSpeed = 0.02f; //camera rotation speed
		float almostUp = 1.56f; //this is nearly 90 degrees (90 flips the camera)

		// camera rotation
		if (arrowIsDown[0]) phi = std::min(phi + cameraSpeed, almostUp);
		if (arrowIsDown[1]) theta -= cameraSpeed;
		if (arrowIsDown[2]) phi = std::max(phi - cameraSpeed, 0.0f);
		if (arrowIsDown[3]) theta += cameraSpeed;

		//player and camera orientation
		vec3 cameraForward = vec3(cos(phi) * -sin(theta),
                            	sin(phi),
                            	cos(phi) * -cos(theta));
		vec3 playerForward = vec3(-sin(theta),
                            	0,
                            	-cos(theta));
		vec3 playerLeft = normalize(cross(playerForward, vec3(0, 1, 0)));

		// player movement
		if (wasdIsDown[0]) playerPos -= playerForward * playerSpeed;
  	if (wasdIsDown[1]) playerPos -= playerLeft * playerSpeed;
  	if (wasdIsDown[2]) playerPos += playerForward * playerSpeed;
  	if (wasdIsDown[3]) playerPos += playerLeft * playerSpeed;

		// place the camera, pointed at the player
		vec3 cameraPos = playerPos - (cameraForward * zoom);
		View->lookAt(cameraPos, playerPos, vec3(0, 1, 0));
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
		Projection->perspective(45.0f, aspect, 0.01f, 100.0f);
		//View for fps camera
		View->pushMatrix();
		playerCamMovement(View);

		//Draw our scene - two meshes - right now to a texture
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));

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
				glUniform3f(prog->getUniform("matAMB"), 0.02, 0.04, 0.2);
				glUniform3f(prog->getUniform("matDIF"), 0.0, 0.16, 0.9);
				break;

				//flat grey
			case 1:
				glUniform3f(prog->getUniform("matAMB"), 0.13, 0.13, 0.14);
				glUniform3f(prog->getUniform("matDIF"), 0.3, 0.3, 0.4);
				break;

				//brass
			case 2:
				glUniform3f(prog->getUniform("matAMB"), 0.3294, 0.2235, 0.02745);
				glUniform3f(prog->getUniform("matDIF"), 0.7804, 0.5686, 0.11373);
				break;

				//ruby
			case 3:
				glUniform3f(prog->getUniform("matAMB"), 0.1745f, 0.01175f, 0.01175f);
				glUniform3f(prog->getUniform("matDIF"), 0.61424f, 0.04136f, 0.04136f);
				break;

			case 4: //ground color
				glUniform3f(prog->getUniform("matAMB"), 0.1913f, 0.0735f, 0.0225f);
				glUniform3f(prog->getUniform("matDIF"), 0.1, 0.27048f, 0.0828f);
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
