#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <il.h>

#include "Shader.h"
#include "Model.h"
#include "Camera.h"
#include "stb_image.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

const unsigned int MAP_WIDTH = 400;
const unsigned int MAP_HEIGHT = 600;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


int main()
{
	int scanned = 0;
	int width, height, bpp;
	unsigned char* rgb = stbi_load("../resources/Maps/Map2.png", &width, &height, &bpp, 3);

	cout << endl << "Map width: " << width << endl;
	cout << "Map height: " << height << endl;
	cout << "Area: " << height * width << endl;
	cout << "Bytes per pixel: " << bpp << endl;

	/*
	int*** arr = (int***)malloc(width*sizeof(int**));

	for (int i = 0; i < width-1; i++) {
		arr[i] = (int**)malloc(height*sizeof(int*));

		for (int j = 0; j < height; j++)
		{
			arr[i][j] = (int*)malloc(3*sizeof(int));
		}
	}
	*/
	int arrlen = width * height * bpp;
	int* arr = (int*)malloc(arrlen * sizeof(int));
	cout << "arr size: " << arrlen << endl;

	int x = 0;
	int y = 0;
	int counter = 0;
	for (int i = 0; i < arrlen; i += bpp) {
		int r = int(rgb[i]);
		int g = int(rgb[i + 1]);
		int b = int(rgb[i + 2]);
		if (counter > width - 1) {
			counter = 0;
			y = 0;
			x++;
		}
		int xInd = height * 5 * x + 5 * y + 0;
		int yInd = height * 5 * x + 5 * y + 1;
		int rInd = height * 5 * x + 5 * y + 2;
		int gInd = height * 5 * x + 5 * y + 3;
		int bInd = height * 5 * x + 5 * y + 4;

		arr[xInd] = x;
		arr[yInd] = y;
		arr[rInd] = r;
		arr[gInd] = g;
		arr[bInd] = b;

		/*cout << "\n\tx at: " << xInd;
		cout << ": " << arr[xInd];
		cout << "\ty at: " << yInd;
		cout << ": " << arr[yInd];
		cout << "\tr at: " << rInd;
		cout << ": " << arr[rInd];
		cout << "\tg at: " << gInd;
		cout << ": " << arr[gInd];
		cout << "\tb at: " << bInd;
		cout << ": " << arr[bInd];*/

		//arr[x][y][0] = r;
		//arr[x][y][1] = g;
		//arr[x][y][2] = b;
		//cout << endl << "x: " << x << "\ty: " << y << "\tr: " << r << "\tg:" << g << "\tb: " << b << endl;
		y++;
		counter++;
		scanned++;
	}

	//cout << endl << "Pixels scanned: " << scanned << endl;


	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("../resources/model_vert.glsl", "../resources/model_frag.glsl");

	// load models
	// -----------
	Model ourModel(("../resources/Models/pine_tree_free.fbx"));
	//Model ourModel(("../resources/Animated Cow/Models/cow.fbx"));


	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		ourShader.use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		// render the loaded model
		//glm::mat4 model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
		
		for (int i = 0; i < arrlen; i += 5) {
			int xCoord = arr[i], yCoord = arr[i + 1];
			int rVal = arr[i + 2], gVal = arr[i + 3], bVal = arr[i + 4];
			//cout << rVal;
			if (rVal == 255 && gVal == 255 && bVal == 255) {
				cout << xCoord << yCoord << endl;
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(float(xCoord), 0.0f, float(yCoord)));
				model = glm::scale(model, glm::vec3(0.001f, 0.001f, 0.001f));	// it's a bit too big for our scene, so scale it down
				ourShader.setMat4("model", model);
				ourModel.Draw(ourShader);
			}
			//cout << "x: " << arr[i] << "\ty: " << arr[i + 1] << "\tr: " << arr[i + 2] << "\tg: " << arr[i + 3] << "\tb: " << arr[i + 4] << endl;
		}
		//cout << arr[arrlen-5] << " " << (arr[arrlen - 4]) << endl;


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow * window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow * window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}