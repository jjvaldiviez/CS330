#include <iostream>
#include <cstdlib>          // EXIT_FAILURE
//#include <glad/glad.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

namespace {

	const char* const WINDOW_TITLE = "Assignment 5 - Jacob Valdiviez"; // Macro for window title
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbo;         // Handle for the vertex buffer object
		GLuint nVertices;    // Number of indices of the mesh
	};
	/*
		Stores texture ids for the rubiks cube
	*/
	struct RubiksTexture
	{

		GLuint whiteFaceId;
		GLuint yellowFaceId;
		GLuint redFaceId;
		GLuint orangeFaceId;
		GLuint blueFaceId;
		GLuint greenFaceId;

	};
	/*
		Stores meshes for the rubiks cube
	*/
	struct RubiksMesh
	{

		GLMesh topFace;
		GLMesh bottomFace;
		GLMesh rightFace;
		GLMesh leftFace;
		GLMesh frontFace;
		GLMesh backFace;

		int topFaceId = 4;
		int bottomFaceId = 5;
		int rightFaceId = 6;
		int leftFaceId = 7;
		int frontFaceId = 8;
		int backFaceId = 9;


	};
	/*
		Stores textures for the Raspberry pi
	*/
	struct PiTexture {

		GLuint body;
		GLuint top;
		GLuint bottom;

	};

	struct BalmTexture {

		GLuint body;
		GLuint top;
		GLuint bottom;

	};

	/*
		Stores meshes for the Raspberry pi
	*/
	struct PiMesh
	{

		GLMesh body;
		GLMesh top;
		GLMesh bottom;

		int bodyId = 10;
		int topId = 11;
		int bottomId = 12;

	};

	struct BalmMesh
	{

		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;

		GLMesh body;
		GLMesh top;
		GLMesh bottom;

		int bodyId = 13;
		int topId = 14;
		int bottomId = 15;

	};

	// Main GLFW window
	GLFWwindow* gWindow = nullptr;
	// Switch body
	GLMesh gMesh1;
	//Switch screen
	GLMesh gMesh2;
	//wooden plane
	GLMesh gMesh3;
	//rubiks cube
	RubiksMesh rubiksMesh;
	//Raspberry pi
	PiMesh piMesh;
	//Lip balm
	BalmMesh balmMesh;

	// Texture for switch body
	GLuint gTextureId1;
	// Texture2 for switch screen
	GLuint gTextureId2;
	// Texture3 for wooden plane
	GLuint gTextureId3;
	// Textures for rubiks cube
	RubiksTexture rubiksIds;
	// Textures for Raspberry pi
	PiTexture piIds;

	BalmTexture balmIds;

	glm::vec2 gUVScale(1.0f, 1.0f);
	GLint gTexWrapMode = GL_REPEAT;

	// Shader program
	GLuint gProgramId;
	GLuint gLampProgramId;

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	bool firstMouse = true;
	float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
	float pitch = 0.0f;
	float lastX = 800.0f / 2.0;
	float lastY = 600.0 / 2.0;
	float fov = 45.0f;

	// timing
	float deltaTime = 0.0f;	// time between current frame and last frame
	float lastFrame = 0.0f;

	float movementSpeed = 2.5;

	// Light position and scale
	glm::vec3 gLightPosition(0.0f, 3.0f, 4.0f);
	glm::vec3 gLightScale(0.3f);

	glm::vec3 gLightPosition2(0.0, 0.0f, -4.0f);
	glm::vec3 gLightScale2(0.3f);

}


bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void UCreateMesh(GLMesh& mesh, int id);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
	layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Global variables for the transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates

	vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

	vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties

	vertexTextureCoordinate = textureCoordinate;
});

/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
	in vec3 vertexNormal; // For incoming normals
	in vec3 vertexFragmentPos; // For incoming fragment position
	in vec2 vertexTextureCoordinate;

out vec4 fragmentColor;

uniform vec3 objectColor;
/*
	Stores data for the directional lights
*/
struct DirLight
{
	vec3 position;
	float strength;
	vec3 lightColor;

};
//Normal light 1
DirLight light1 = DirLight(
	vec3(0.0f, 3.0f, 4.0f),
	0.5f,
	vec3(1.0f, 1.0f, 1.0f)
);
//Normal light 2
DirLight light2 = DirLight(

	vec3(0.0f, 0.0f, -4.0f),
	0.5f,
	vec3(1.0f, 1.0f, 1.0f)

);

uniform vec3 viewPosition;

uniform sampler2D uTexture;
uniform vec2 uvScale;

void main()
{
	/*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

//Calculate Ambient lighting*/
/*Light 1*/
//calculate diffuse lighting
	vec3 ambient = light1.strength * light1.lightColor;

	vec3 norm = normalize(vertexNormal);
	vec3 direction = normalize(light1.position - vertexFragmentPos);

	float impact = max(dot(norm, direction), 0.0);
	vec3 diffuse = impact * light1.lightColor;

	//Calculate specular intensity
	float specularIntensity = 0.8f;
	float highlightSize = 16.0f;

	vec3 viewDir = normalize(viewPosition - vertexFragmentPos);
	vec3 reflectDir = reflect(-direction, norm);
	//Calculate specular component
	float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
	vec3 specular = specularIntensity * specularComponent * light1.lightColor;
	//Texture
	vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);
	//Phong
	vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;


	/*light 2*/

	//calculate diffuse lighting
	ambient = light2.strength * light2.lightColor;

	norm = normalize(vertexNormal);
	direction = normalize(light2.position - vertexFragmentPos);

	impact = max(dot(norm, direction), 0.0);
	diffuse = impact * light2.lightColor;
	//Calculate specular intensity
	specularIntensity = 1.0f;
	highlightSize = 16.0f;

	viewDir = normalize(viewPosition - vertexFragmentPos);
	reflectDir = reflect(-direction, norm);
	//Calculate specular component
	specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
	specular = specularIntensity * specularComponent * light2.lightColor;
	//Texture
	textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);
	//Phong
	phong += (ambient + diffuse + specular) * textureColor.xyz;


	//Output lighting and color
	fragmentColor = vec4(phong, 1.0f);
}
);

/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

	layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

		//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);

/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

	out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
	fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);

// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
	for (int j = 0; j < height / 2; ++j)
	{
		int index1 = j * width * channels;
		int index2 = (height - 1 - j) * width * channels;

		for (int i = width * channels; i > 0; --i)
		{
			unsigned char tmp = image[index1];
			image[index1] = image[index2];
			image[index2] = tmp;
			++index1;
			++index2;
		}
	}
}

int main(int argc, char* argv[])
{

	if (!UInitialize(argc, argv, &gWindow))
		return EXIT_FAILURE;
	//mesh1 first box
	UCreateMesh(gMesh1, 1);
	//mesh2 second box
	UCreateMesh(gMesh2, 2);
	//mesh3 plane
	UCreateMesh(gMesh3, 3);
	//rubiks cube mesh
	UCreateMesh(rubiksMesh.topFace, rubiksMesh.topFaceId);
	UCreateMesh(rubiksMesh.bottomFace, rubiksMesh.bottomFaceId);
	UCreateMesh(rubiksMesh.leftFace, rubiksMesh.leftFaceId);
	UCreateMesh(rubiksMesh.rightFace, rubiksMesh.rightFaceId);
	UCreateMesh(rubiksMesh.frontFace, rubiksMesh.frontFaceId);
	UCreateMesh(rubiksMesh.backFace, rubiksMesh.backFaceId);

	//Raspberry Pi mesh
	UCreateMesh(piMesh.body, piMesh.bodyId);
	UCreateMesh(piMesh.top, piMesh.topId);
	UCreateMesh(piMesh.bottom, piMesh.bottomId);

	UCreateMesh(balmMesh.body, balmMesh.bodyId);
	UCreateMesh(balmMesh.top, balmMesh.topId);
	UCreateMesh(balmMesh.bottom, balmMesh.bottomId);

	if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
		return EXIT_FAILURE;

	if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
		return EXIT_FAILURE;

	// Load texture for box1
	const char* texFilename1 = "images/switch_screen_without.png";
	if (!UCreateTexture(texFilename1, gTextureId1))
	{
		cout << "Failed to load texture " << texFilename1 << endl;
		return EXIT_FAILURE;
	}
	//Load texture for box2
	const char* texFilename2 = "images/switch_screen_with.png";
	if (!UCreateTexture(texFilename2, gTextureId2))
	{

		cout << "Failed to load texture " << texFilename2 << endl;
		return EXIT_FAILURE;
	}
	//Load texture for box3
	const char* texFilename3 = "images/wood3.png";
	if (!UCreateTexture(texFilename3, gTextureId3))
	{

		cout << "Failed to load texture " << texFilename3 << endl;
		return EXIT_FAILURE;
	}

	/*
		
		Rubiks Cube Textures
	
	*/
	//White face of cube
	const char* whiteFilename = "images/white-face.png";
	if (!UCreateTexture(whiteFilename, rubiksIds.whiteFaceId))
	{
		cout << "Failed to load texture " << whiteFilename << endl;
		return EXIT_FAILURE;
	}
	//Yellow face of cube
	const char* yellowFilename = "images/yellow-face.png";
	if (!UCreateTexture(yellowFilename, rubiksIds.yellowFaceId))
	{
		cout << "Failed to load texture " << yellowFilename << endl;
		return EXIT_FAILURE;
	}
	//Red face of cube
	const char* redFilename = "images/red-face.png";
	if (!UCreateTexture(redFilename, rubiksIds.redFaceId))
	{
		cout << "Failed to load texture " << redFilename << endl;
		return EXIT_FAILURE;
	}
	//Orange face of cube
	const char* orangeFilename = "images/orange-face.png";
	if (!UCreateTexture(orangeFilename, rubiksIds.orangeFaceId))
	{
		cout << "Failed to load texture " << orangeFilename << endl;
		return EXIT_FAILURE;
	}
	//Blue face of cube
	const char* blueFilename = "images/blue-face.png";
	if (!UCreateTexture(blueFilename, rubiksIds.blueFaceId))
	{
		cout << "Failed to load texture " << blueFilename << endl;
		return EXIT_FAILURE;
	}
	//Green face of cube
	const char* greenFilename = "images/green-face.png";
	if (!UCreateTexture(greenFilename, rubiksIds.greenFaceId))
	{
		cout << "Failed to load texture " << greenFilename << endl;
		return EXIT_FAILURE;
	}

	/*
	
		Raspberry Pi Textures
	
	*/
	//Pi body texture
	const char* bodyFilename = "images/pi-body.png";
	if (!UCreateTexture(bodyFilename, piIds.body))
	{
		cout << "Failed to load texture " << bodyFilename << endl;
		return EXIT_FAILURE;
	}
	//Pi top texture
	const char* topFilename = "images/pi-top.png";
	if (!UCreateTexture(topFilename, piIds.top))
	{
		cout << "Failed to load texture " << topFilename << endl;
		return EXIT_FAILURE;
	}
	//Pi bottom texture
	const char* bottomFilename = "images/pi-bottom.png";
	if (!UCreateTexture(bottomFilename, piIds.bottom))
	{
		cout << "Failed to load texture " << bottomFilename << endl;
		return EXIT_FAILURE;
	}
	//Lip balm body texture
	const char* bBodFilename = "images/balm-body.png";
	if (!UCreateTexture(bBodFilename, balmIds.body))
	{
		cout << "Failed to load texture " << bBodFilename << endl;
		return EXIT_FAILURE;
	}
	//Lip balm top texture
	const char* bTFilename = "images/balm-top-bot.png";
	if (!UCreateTexture(bTFilename, balmIds.top))
	{
		cout << "Failed to load texture " << bTFilename << endl;
		return EXIT_FAILURE;
	}
	//Lip balm bottom texture
	const char* bBFilename = "images/balm-top-bot.png";
	if (!UCreateTexture(bBFilename, balmIds.bottom))
	{
		cout << "Failed to load texture " << bBFilename << endl;
		return EXIT_FAILURE;
	}

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	glUseProgram(gProgramId);
	// We set the texture as texture unit 0
	glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);
	// Sets the background color of the window to black (it will be implicitely used by glClear)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(gWindow))
	{
		// per-frame timing
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(gWindow);

		// Render this frame
		URender();

		glfwPollEvents();
	}


	// Release mesh data
	UDestroyMesh(gMesh1);

	UDestroyMesh(gMesh2);

	UDestroyMesh(gMesh3);

	UDestroyMesh(rubiksMesh.topFace);
	UDestroyMesh(rubiksMesh.bottomFace);
	UDestroyMesh(rubiksMesh.leftFace);
	UDestroyMesh(rubiksMesh.rightFace);
	UDestroyMesh(rubiksMesh.frontFace);
	UDestroyMesh(rubiksMesh.backFace);


	UDestroyMesh(piMesh.top);
	UDestroyMesh(piMesh.bottom);
	UDestroyMesh(piMesh.body);

	UDestroyMesh(balmMesh.top);
	UDestroyMesh(balmMesh.bottom);
	UDestroyMesh(balmMesh.body);

	// Release texture
	UDestroyTexture(gTextureId1);

	UDestroyTexture(gTextureId2);

	UDestroyTexture(gTextureId2);

	UDestroyTexture(rubiksIds.whiteFaceId);

	UDestroyTexture(rubiksIds.yellowFaceId);

	UDestroyTexture(rubiksIds.redFaceId);

	UDestroyTexture(rubiksIds.orangeFaceId);

	UDestroyTexture(rubiksIds.blueFaceId);

	UDestroyTexture(rubiksIds.greenFaceId);


	UDestroyTexture(piIds.top);

	UDestroyTexture(piIds.bottom);

	UDestroyTexture(piIds.body);

	UDestroyTexture(balmIds.top);

	UDestroyTexture(balmIds.bottom);

	UDestroyTexture(balmIds.body);

	// Release shader program
	UDestroyShaderProgram(gProgramId);
	UDestroyShaderProgram(gLampProgramId);

	exit(EXIT_SUCCESS); // Terminates the program successfully
}

// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
	// GLFW: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// GLFW: window creation
	// ---------------------
	* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, UResizeWindow);
	glfwSetCursorPosCallback(*window, mouse_callback);
	glfwSetScrollCallback(*window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLEW: initialize
	// ----------------
	// Note: if using GLEW version 1.13 or earlier
	glewExperimental = GL_TRUE;
	GLenum GlewInitResult = glewInit();

	if (GLEW_OK != GlewInitResult)
	{
		std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
		return false;
	}

	// Displays GPU OpenGL version
	cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

	return true;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = movementSpeed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		cameraPos += cameraUp * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		cameraPos -= cameraUp * cameraSpeed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
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

	float sensitivity = 0.1f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//Change speed
	if (yoffset < 0) {

		movementSpeed = (movementSpeed - 1) < 1 ? 1 : movementSpeed - 1;

	}
	else {

		movementSpeed++;

	}

}

// Functioned called to render a frame
void URender()
{
	// Enable z-depth
	glEnable(GL_DEPTH_TEST);

	// Clear the frame and z buffers
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 1. Scales the object by 2
	glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
	// 2. Rotates shape by 15 degrees in the x axis
	glm::mat4 rotation = glm::rotate(45.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Place object at the origin
	glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
	// Model matrix: transformations are applied right-to-left order
	glm::mat4 model = translation * rotation * scale;

	// Creates a perspective projection
	glm::mat4 projection = glm::perspective(glm::radians(fov), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

	// Set the shader to be used
	glUseProgram(gProgramId);

	// Retrieves and passes transform matrices to the Shader program
	GLint modelLoc = glGetUniformLocation(gProgramId, "model");
	GLint viewLoc = glGetUniformLocation(gProgramId, "view");
	GLint projLoc = glGetUniformLocation(gProgramId, "projection");

	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(gMesh1.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId1);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, gMesh1.nVertices);



	/*
		Draw Switch screen
	*/
	glUseProgram(gProgramId);

	// Deactivate the Vertex Array Object
	//glBindVertexArray(0);

	glBindVertexArray(gMesh2.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId2);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, gMesh2.nVertices);

	// Deactivate the Vertex Array Object
	//glBindVertexArray(0);

	/*
		Draw wooden plane
	*/

	glUseProgram(gProgramId);

	glBindVertexArray(gMesh3.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId3);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, gMesh3.nVertices);

	/*
		Draw Rubiks cube (left face)
	*/

	glUseProgram(gProgramId);

	glBindVertexArray(rubiksMesh.leftFace.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rubiksIds.orangeFaceId);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, rubiksMesh.leftFace.nVertices);

	/*
		Draw Rubiks cube (back face)
	*/

	glUseProgram(gProgramId);

	glBindVertexArray(rubiksMesh.backFace.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rubiksIds.greenFaceId);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, rubiksMesh.backFace.nVertices);

	/*
		Draw Rubiks cube (front face)
	*/

	glUseProgram(gProgramId);

	glBindVertexArray(rubiksMesh.frontFace.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rubiksIds.blueFaceId);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, rubiksMesh.frontFace.nVertices);

	/*
		Draw Rubiks cube (top Face)
	*/

	glUseProgram(gProgramId);

	glBindVertexArray(rubiksMesh.topFace.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rubiksIds.whiteFaceId);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, rubiksMesh.topFace.nVertices);

	/*
		Draw Rubiks cube (right face)
	*/

	glUseProgram(gProgramId);

	glBindVertexArray(rubiksMesh.rightFace.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rubiksIds.redFaceId);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, rubiksMesh.rightFace.nVertices);

	/*
		Draw Rubiks cube (bottom face)
	*/

	glUseProgram(gProgramId);

	glBindVertexArray(rubiksMesh.bottomFace.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rubiksIds.yellowFaceId);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, rubiksMesh.bottomFace.nVertices);

	/*
		Draw Raspberry pi (top)
	*/

	glUseProgram(gProgramId);

	glBindVertexArray(piMesh.top.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, piIds.top);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, piMesh.top.nVertices);

	/*
		Draw Raspberry pi (body)
	*/

	glUseProgram(gProgramId);

	glBindVertexArray(piMesh.body.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, piIds.body);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, piMesh.body.nVertices);

	/*
		Draw Raspberry pi (bottom)
	*/

	glUseProgram(gProgramId);

	glBindVertexArray(piMesh.bottom.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, piIds.bottom);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, piMesh.bottom.nVertices);

	/*
		Draw Lip balm body
	*/

	glUseProgram(gProgramId);

	glBindVertexArray(balmMesh.body.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, balmIds.body);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, balmMesh.body.nVertices);

	/*
		Draw Lip balm Top
	*/

	glUseProgram(gProgramId);

	glBindVertexArray(balmMesh.top.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, balmIds.top);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, balmMesh.top.nVertices);
	/*
		Draw Lip balm bottom
	*/
	glUseProgram(gProgramId);

	glBindVertexArray(balmMesh.bottom.vao);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, balmIds.bottom);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, balmMesh.bottom.nVertices);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}

// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh, int id)
{
	/*
		Vertices for body of switch
	
	*/
	GLfloat vertices1[] = {
	//Positions           //Texture Coordinates

	-0.7f, -0.1f, -0.5f,  0.0f, 0.0f,    //Front Face
	 0.7f, -0.1f, -0.5f,  1.0f, 0.0f,
	 0.7f,  0.0f, -0.5f,  1.0f, 1.0f,
	 0.7f,  0.0f, -0.5f,  1.0f, 1.0f,
	-0.7f,  0.0f, -0.5f,  0.0f, 1.0f,
	-0.7f, -0.1f, -0.5f,  0.0f, 0.0f,

	-0.7f, -0.1f,  0.5f,  0.0f, 0.0f,    //Back Face
	 0.7f, -0.1f,  0.5f,  1.0f, 0.0f,
	 0.7f,  0.0f,  0.5f,  1.0f, 1.0f,
	 0.7f,  0.0f,  0.5f,  1.0f, 1.0f,
	-0.7f,  0.0f,  0.5f,  0.0f, 1.0f,
	-0.7f, -0.1f,  0.5f,  0.0f, 0.0f,

	-0.7f,  0.0f,  0.5f,  0.0f, 0.0f,    //Left Face
	-0.7f,  0.0f, -0.5f,  1.0f, 0.0f,
	-0.7f, -0.1f, -0.5f,  1.0f, 1.0f,
	-0.7f, -0.1f, -0.5f,  1.0f, 1.0f,
	-0.7f, -0.1f,  0.5f,  0.0f, 1.0f,
	-0.7f,  0.0f,  0.5f,  0.0f, 0.0f,

	 0.7f,  0.0f,  0.5f,  0.0f, 0.0f,    //Right Face
	 0.7f,  0.0f, -0.5f,  1.0f, 0.0f,
	 0.7f, -0.1f, -0.5f,  1.0f, 1.0f,
	 0.7f, -0.1f, -0.5f,  1.0f, 1.0f,
	 0.7f, -0.1f,  0.5f,  0.0f, 1.0f,
	 0.7f,  0.0f,  0.5f,  0.0f, 0.0f,

	-0.7f, -0.1f, -0.5f,  0.0f, 0.0f,    //Bottom Face
	 0.7f, -0.1f, -0.5f,  1.0f, 0.0f,
	 0.7f, -0.1f,  0.5f,  1.0f, 1.0f,
	 0.7f, -0.1f,  0.5f,  1.0f, 1.0f,
	-0.7f, -0.1f,  0.5f,  0.0f, 1.0f,
	-0.7f, -0.1f, -0.5f,  0.0f, 0.0f,

	-0.7f,  0.0f, -0.5f,  0.0f, 0.0f,    //Top Face
	 0.7f,  0.0f, -0.5f,  1.0f, 0.0f,
	 0.7f,  0.0f,  0.5f,  1.0f, 1.0f,
	 0.7f,  0.0f,  0.5f,  1.0f, 1.0f,
	-0.7f,  0.0f,  0.5f,  0.0f, 1.0f,
	-0.7f,  0.0f, -0.5f,  0.0f, 0.0f,

	};
	/*
		Vertices for screen of switch
	
	*/
	GLfloat vertices2[] = {

	-0.65f, -0.1f, -0.466f,  0.0f, 0.0f,    //Front Face
	 0.65f, -0.1f, -0.466f,  1.0f, 0.0f,
	 0.65f,  0.001f, -0.466f,  1.0f, 1.0f,
	 0.65f,  0.001f, -0.466f,  1.0f, 1.0f,
	-0.65f,  0.001f, -0.466f,  0.0f, 1.0f,
	-0.65f, -0.1f, -0.466f,  0.0f, 0.0f,

	-0.65f, -0.1f,  0.466f,  0.0f, 0.0f,    //Back Face
	 0.65f, -0.1f,  0.466f,  1.0f, 0.0f,
	 0.65f,  0.001f,  0.466f,  1.0f, 1.0f,
	 0.65f,  0.001f,  0.466f,  1.0f, 1.0f,
	-0.65f,  0.001f,  0.466f,  0.0f, 1.0f,
	-0.65f, -0.1f,  0.466f,  0.0f, 0.0f,

	-0.65f, -0.1f, -0.466f,  0.0f, 0.0f,
	-0.65f,  0.001f,  0.466f,  1.0f, 1.0f,    //Left Face
	-0.65f,  0.001f, -0.466f,  0.0f, 1.0f,
	-0.65f, -0.1f, -0.466f,  0.0f, 1.0f,
	-0.65f, -0.1f,  0.466f,  1.0f, 0.0f,
	-0.65f,  0.001f,  0.466f,  1.0f, 1.0f,

	 0.65f,  0.001f,  0.466f,  1.0f, 1.0f,    //Right Face
	 0.65f,  0.001f, -0.466f,  0.0f, 1.0f,
	 0.65f, -0.1f, -0.466f,  0.0f, 0.0f,
	 0.65f, -0.1f, -0.466f,  0.0f, 0.0f,
	 0.65f, -0.1f,  0.466f,  1.0f, 0.0f,
	 0.65f,  0.001f,  0.466f,  1.0f, 1.0f,

	-0.65f, -0.1f, -0.466f,  0.0f, 0.0f,    //Bottom Face
	 0.65f, -0.1f, -0.466f,  1.0f, 0.0f,
	 0.65f, -0.1f,  0.466f,  1.0f, 1.0f,
	 0.65f, -0.1f,  0.466f,  1.0f, 1.0f,
	-0.65f, -0.1f,  0.466f,  0.0f, 1.0f,
	-0.65f, -0.1f, -0.466f,  0.0f, 0.0f,

	-0.65f,  0.001f, -0.466f,  0.0f, 0.0f,    //Top Face
	 0.65f,  0.001f, -0.466f,  1.0f, 0.0f,
	 0.65f,  0.001f,  0.466f,  1.0f, 1.0f,
	 0.65f,  0.001f,  0.466f,  1.0f, 1.0f,
	-0.65f,  0.001f,  0.466f,  0.0f, 1.0f,
	-0.65f,  0.001f, -0.466f,  0.0f, 0.0f

	};
	/*
		Vertices for wooden plane
	*/
	GLfloat vertices3[] = {

	-5.0f,  -0.1f, -5.0f,  0.0f, 0.0f,    //Plane
	 5.0f,  -0.1f, -5.0f,  1.0f, 0.0f,
	 5.0f,  -0.1f,  5.0f,  1.0f, 1.0f,
	 5.0f,  -0.1f,  5.0f,  1.0f, 1.0f,
	-5.0f,  -0.1f,  5.0f,  0.0f, 1.0f,
	-5.0f,  -0.1f, -5.0f,  0.0f, 0.0f

	};
	/*
		Left rubiks cube
	*/
	GLfloat leftVerts[] = {

		1.5f,  0.0f,  1.5f,  0.0f, 0.0f,    //left face
		1.5f,  1.0f,  1.5f,  1.0f, 0.0f,
		1.5f,  1.0f,  2.5f,  1.0f, 1.0f,
		1.5f,  1.0f,  2.5f,  1.0f, 1.0f,
		1.5f,  0.0f,  2.5f,  0.0f, 1.0f,
		1.5f,  0.0f,  1.5f,  0.0f, 0.0f

	};
	/*
		Back rubiks cube
	*/
	GLfloat backVerts[] = {

	    1.5f,  0.0f,  2.5f,  0.0f, 0.0f, //Back
		2.5f,  0.0f,  2.5f,  1.0f, 0.0f,
		2.5f,  1.0f,  2.5f,  1.0f, 1.0f,
		2.5f,  1.0f,  2.5f,  1.0f, 1.0f,
	    1.5f,  1.0f,  2.5f,  0.0f, 1.0f,
	    1.5f,  0.0f,  2.5f,  0.0f, 0.0f

	};
	/*
		front rubiks cube
	*/
	GLfloat frontVerts[] = {

	    1.5f,  0.0f,  1.5f,  0.0f, 0.0f, //Front
		2.5f,  0.0f,  1.5f,  1.0f, 0.0f,
		2.5f,  1.0f,  1.5f,  1.0f, 1.0f,
		2.5f,  1.0f,  1.5f,  1.0f, 1.0f,
	    1.5f,  1.0f,  1.5f,  0.0f, 1.0f,
	    1.5f,  0.0f,  1.5f,  0.0f, 0.0f

	};
	/*
		top rubiks cube
	*/
	GLfloat topVerts[] = {

	    1.5f,  1.0f,  1.5f,  0.0f, 1.0f, //top
		2.5f,  1.0f,  1.5f,  1.0f, 1.0f,
		2.5f,  1.0f,  2.5f,  1.0f, 0.0f,
		2.5f,  1.0f,  2.5f,  1.0f, 0.0f,
	    1.5f,  1.0f,  2.5f,  0.0f, 0.0f,
	    1.5f,  1.0f,  1.5f,  0.0f, 1.0f

	};
	/*
		right rubiks cube
	*/
	GLfloat rightVerts[] = {

		2.5f,  1.0f,  2.5f,  1.0f, 0.0f, //Right
		2.5f,  1.0f,  1.5f,  1.0f, 1.0f,
		2.5f,  0.0f,  1.5f,  0.0f, 1.0f,
		2.5f,  0.0f,  1.5f,  0.0f, 1.0f,
		2.5f,  0.0f,  2.5f,  0.0f, 0.0f,
		2.5f,  1.0f,  2.5f,  1.0f, 0.0f,

	};
	/*
		bottom rubiks cube
	*/
	GLfloat bottomVerts[] = {

	    1.5f,  0.0f,  1.5f,  0.0f, 1.0f, //bottom
		2.5f,  0.0f,  1.5f,  1.0f, 1.0f,
		2.5f,  0.0f,  2.5f,  1.0f, 0.0f,
		2.5f,  0.0f,  2.5f,  1.0f, 0.0f,
	    1.5f,  0.0f,  2.5f,  0.0f, 0.0f,
	    1.5f,  0.0f,  1.5f,  0.0f, 1.0f

	};
	/*
		raspberry pi body
	*/
	GLfloat piBody[] = {

		-2.0f,  0.0f, 1.5f,  0.0f, 0.0f,  //Front
		-1.0f,  0.0f, 1.5f,  1.0f, 0.0f,
		-1.0f,  0.25f, 1.5f,  1.0f, 1.0f,
		-1.0f,  0.25f, 1.5f,  1.0f, 1.0f,
		-2.0f,  0.25f, 1.5f,  0.0f, 1.0f,
		-2.0f,  0.0f, 1.5f,  0.0f, 0.0f,

		-2.0f,  0.0f,  3.0f,  0.0f, 0.0f,   //Back
		-1.0f,  0.0f,  3.0f,  1.0f, 0.0f,
		-1.0f,  0.25f,  3.0f,  1.0f, 1.0f,
		-1.0f,  0.25f,  3.0f,  1.0f, 1.0f,
		-2.0f,  0.25f,  3.0f,  0.0f, 1.0f,
		-2.0f,  0.0f,  3.0f,  0.0f, 0.0f,

		-2.0f,  0.25f,  3.0f,  1.0f, 0.0f,
		-2.0f,  0.25f, 1.5f,  1.0f, 1.0f,
		-2.0f,  0.0f, 1.5f,  0.0f, 1.0f,
		-2.0f,  0.0f, 1.5f,  0.0f, 1.0f,
		-2.0f,  0.0f,  3.0f,  0.0f, 0.0f,
		-2.0f,  0.25f,  3.0f,  1.0f, 0.0f,

		-1.0f,  0.25f,  3.0f,  1.0f, 0.0f,
		-1.0f,  0.25f, 1.5f,  1.0f, 1.0f,
		-1.0f,  0.0f, 1.5f,  0.0f, 1.0f,
		-1.0f,  0.0f, 1.5f,  0.0f, 1.0f,
		-1.0f,  0.0f,  3.0f,  0.0f, 0.0f,
		-1.0f,  0.25f,  3.0f,  1.0f, 0.0f,

	};
	/*
		Raspberry pi bottom side
	*/
	GLfloat piBottom[] = {

		-2.0f, 0.0f,  1.5f,  0.0f, 1.0f,  //Bottom
		-1.0f, 0.0f,  1.5f,  1.0f, 1.0f,
		-1.0f, 0.0f,  3.0f,  1.0f, 0.0f,
		-1.0f, 0.0f,  3.0f,  1.0f, 0.0f,
		-2.0f, 0.0f,  3.0f,  0.0f, 0.0f,
		-2.0f, 0.0f,  1.5f,  0.0f, 1.0f,

	};
	/*
		Rsapberry pi top side
	*/
	GLfloat piTop[] = {

		-2.0f,  0.25f,  1.5f,  0.0f, 1.0f,   //Top
		-1.0f,  0.25f,  1.5f,  1.0f, 1.0f,
		-1.0f,  0.25f,  3.0f,  1.0f, 0.0f,
		-1.0f,  0.25f,  3.0f,  1.0f, 0.0f,
		-2.0f,  0.25f,  3.0f,  0.0f, 0.0f,
		-2.0f,  0.25f,  1.5f,  0.0f, 1.0f

	};
	//Translate 2 to the left
	float xMove = -2.0f;
	/*
		Lip balm body vertices
	*/
	GLfloat balmBody[] = {

		-0.1f + xMove, 0.0f, -0.1f,  0.0f, 0.0f,
		 0.1f + xMove, 0.0f, -0.1f,  1.0f, 0.0f,
		 0.1f + xMove,  0.5f, -0.1f,  1.0f, 1.0f,
		 0.1f + xMove,  0.5f, -0.1f,  1.0f, 1.0f,
		-0.1f + xMove,  0.5f, -0.1f,  0.0f, 1.0f,
		-0.1f + xMove, 0.0f, -0.1f,  0.0f, 0.0f,

		-0.1f + xMove, 0.0f,  0.1f,  0.0f, 0.0f,
		 0.1f + xMove, 0.0f,  0.1f,  1.0f, 0.0f,
		 0.1f + xMove,  0.5f,  0.1f,  1.0f, 1.0f,
		 0.1f + xMove,  0.5f,  0.1f,  1.0f, 1.0f,
		-0.1f + xMove,  0.5f,  0.1f,  0.0f, 1.0f,
		-0.1f + xMove, 0.0f,  0.1f,  0.0f, 0.0f,

		-0.1f + xMove,  0.5f,  0.1f,  1.0f, 0.0f,
		-0.1f + xMove,  0.5f, -0.1f,  1.0f, 1.0f,
		-0.1f + xMove, 0.0f, -0.1f,  0.0f, 1.0f,
		-0.1f + xMove, 0.0f, -0.1f,  0.0f, 1.0f,
		-0.1f + xMove, 0.0f,  0.1f,  0.0f, 0.0f,
		-0.1f + xMove,  0.5f,  0.1f,  1.0f, 0.0f,

		 0.1f + xMove,  0.5f,  0.1f,  1.0f, 0.0f,
		 0.1f + xMove,  0.5f, -0.1f,  1.0f, 1.0f,
		 0.1f + xMove, 0.0f, -0.1f,  0.0f, 1.0f,
		 0.1f + xMove, 0.0f, -0.1f,  0.0f, 1.0f,
		 0.1f + xMove, 0.0f,  0.1f,  0.0f, 0.0f,
		 0.1f + xMove,  0.5f,  0.1f,  1.0f, 0.0f,

	};
	/*
		Lip balm bottom vertices
	*/
	GLfloat balmBot[]{

		-0.1f + xMove, 0.0f, -0.1f,  0.0f, 1.0f,
		 0.1f + xMove, 0.0f, -0.1f,  1.0f, 1.0f,
		 0.1f + xMove, 0.0f,  0.1f,  1.0f, 0.0f,
		 0.1f + xMove, 0.0f,  0.1f,  1.0f, 0.0f,
		-0.1f + xMove, 0.0f,  0.1f,  0.0f, 0.0f,
		-0.1f + xMove, 0.0f, -0.1f,  0.0f, 1.0f
	};
	/*
		Lip balm top vertices	
	*/
	GLfloat balmTop[]{
		-0.1f + xMove,  0.5f, -0.1f,  0.0f, 1.0f,
		 0.1f + xMove,  0.5f, -0.1f,  1.0f, 1.0f,
		 0.1f + xMove,  0.5f,  0.1f,  1.0f, 0.0f,
		 0.1f + xMove,  0.5f,  0.1f,  1.0f, 0.0f,
		-0.1f + xMove,  0.5f,  0.1f,  0.0f, 0.0f,
		-0.1f + xMove,  0.5f, -0.1f,  0.0f, 1.0f

	};

	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerUV = 2;
	/*
		Switch body
	*/
	if (id == 1) {
		mesh.nVertices = sizeof(vertices1) / (sizeof(vertices1[0]) * (floatsPerVertex + floatsPerUV));

		glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
		glBindVertexArray(mesh.vao);

		// Create VBO
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

		// Strides between vertex coordinates
		GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

		// Create Vertex Attribute Pointers
		glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
		glEnableVertexAttribArray(2);
	}
	/*
		Switch screen
	*/
	else if (id == 2) {

		mesh.nVertices = sizeof(vertices2) / (sizeof(vertices2[0]) * (floatsPerVertex + floatsPerUV));

		glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
		glBindVertexArray(mesh.vao);

		// Create VBO
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

		// Strides between vertex coordinates
		GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

		// Create Vertex Attribute Pointers
		glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
		glEnableVertexAttribArray(2);

	}
	/*
		Wooden plane
	*/
	else if (id == 3) {

		mesh.nVertices = sizeof(vertices3) / (sizeof(vertices3[0]) * (floatsPerVertex + floatsPerUV));

		glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
		glBindVertexArray(mesh.vao);

		// Create VBO
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices3), vertices3, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

		// Strides between vertex coordinates
		GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

		// Create Vertex Attribute Pointers
		glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
		glEnableVertexAttribArray(2);

	}
	/*
		rubiks cube left face
	*/
	else if (id == rubiksMesh.leftFaceId) {

		mesh.nVertices = sizeof(leftVerts) / (sizeof(leftVerts[0]) * (floatsPerVertex + floatsPerUV));

		glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
		glBindVertexArray(mesh.vao);

		// Create VBO
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(leftVerts), leftVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

		// Strides between vertex coordinates
		GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

		// Create Vertex Attribute Pointers
		glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
		glEnableVertexAttribArray(2);

	}
	/*
		Rubiks cube back face
	*/
	else if (id == rubiksMesh.backFaceId) {

		mesh.nVertices = sizeof(backVerts) / (sizeof(backVerts[0]) * (floatsPerVertex + floatsPerUV));

		glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
		glBindVertexArray(mesh.vao);

		// Create VBO
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(backVerts), backVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

		// Strides between vertex coordinates
		GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

		// Create Vertex Attribute Pointers
		glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
		glEnableVertexAttribArray(2);

	}
	/*
		rubiks cube front face
	*/
	else if (id == rubiksMesh.frontFaceId) {

		mesh.nVertices = sizeof(frontVerts) / (sizeof(frontVerts[0]) * (floatsPerVertex + floatsPerUV));

		glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
		glBindVertexArray(mesh.vao);

		// Create VBO
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(frontVerts), frontVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

		// Strides between vertex coordinates
		GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

		// Create Vertex Attribute Pointers
		glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
		glEnableVertexAttribArray(2);

	}
	/*
		Rubiks cube top face
	*/
	else if (id == rubiksMesh.topFaceId) {

		mesh.nVertices = sizeof(topVerts) / (sizeof(topVerts[0]) * (floatsPerVertex + floatsPerUV));

		glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
		glBindVertexArray(mesh.vao);

		// Create VBO
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(topVerts), topVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

		// Strides between vertex coordinates
		GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

		// Create Vertex Attribute Pointers
		glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);
		
		glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
		glEnableVertexAttribArray(2);

	}
	/*
		Rubiks cube right face
	*/
	else if (id == rubiksMesh.rightFaceId) {

		mesh.nVertices = sizeof(rightVerts) / (sizeof(rightVerts[0]) * (floatsPerVertex + floatsPerUV));

		glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
		glBindVertexArray(mesh.vao);

		// Create VBO
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(rightVerts), rightVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

		// Strides between vertex coordinates
		GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

		// Create Vertex Attribute Pointers
		glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
		glEnableVertexAttribArray(2);

	}
	/*
		Rubiks cube bottom face
	*/
	else if (id == rubiksMesh.bottomFaceId) {

		mesh.nVertices = sizeof(bottomVerts) / (sizeof(bottomVerts[0]) * (floatsPerVertex + floatsPerUV));

		glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
		glBindVertexArray(mesh.vao);

		// Create VBO
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(bottomVerts), bottomVerts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

		// Strides between vertex coordinates
		GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

		// Create Vertex Attribute Pointers
		glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
		glEnableVertexAttribArray(2);

	}
	/*
		Raspberry pi top side
	*/
	else if (id == piMesh.topId) {

		mesh.nVertices = sizeof(piTop) / (sizeof(piTop[0]) * (floatsPerVertex + floatsPerUV));

		glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
		glBindVertexArray(mesh.vao);

		// Create VBO
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(piTop), piTop, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

		// Strides between vertex coordinates
		GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

		// Create Vertex Attribute Pointers
		glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
		glEnableVertexAttribArray(2);

	}
	/*
		Raspberry pi body
	*/
	else if (id == piMesh.bodyId) {

		mesh.nVertices = sizeof(piBody) / (sizeof(piBody[0]) * (floatsPerVertex + floatsPerUV));

		glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
		glBindVertexArray(mesh.vao);

		// Create VBO
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(piBody), piBody, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

		// Strides between vertex coordinates
		GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

		// Create Vertex Attribute Pointers
		glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
		glEnableVertexAttribArray(2);

	}
	/*
		Raspberry pi bottom side
	*/
	else if (id == piMesh.bottomId) {

		mesh.nVertices = sizeof(piBottom) / (sizeof(piBottom[0]) * (floatsPerVertex + floatsPerUV));

		glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
		glBindVertexArray(mesh.vao);

		// Create VBO
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(piBottom), piBottom, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

		// Strides between vertex coordinates
		GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

		// Create Vertex Attribute Pointers
		glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
		glEnableVertexAttribArray(2);

	}
	/*
		Lip balm body
	*/
	else if (id == balmMesh.bodyId) {
		
		mesh.nVertices = sizeof(balmBody) / (sizeof(balmBody[0]) * (floatsPerVertex + floatsPerUV));

		glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
		glBindVertexArray(mesh.vao);

		// Create VBO
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(balmBody), balmBody, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

		// Strides between vertex coordinates
		GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

		// Create Vertex Attribute Pointers
		glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
		glEnableVertexAttribArray(2);
		
	}
	/*
		Lip Balm top
	*/
	else if (id == balmMesh.topId) {

		mesh.nVertices = sizeof(balmTop) / (sizeof(balmTop[0]) * (floatsPerVertex + floatsPerUV));

		glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
		glBindVertexArray(mesh.vao);

		// Create VBO
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(balmTop), balmTop, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

		// Strides between vertex coordinates
		GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

		// Create Vertex Attribute Pointers
		glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
		glEnableVertexAttribArray(2);

	}
	/*
		Lip balm Bottom
	*/
	else if (id == balmMesh.bottomId) {

		mesh.nVertices = sizeof(balmBot) / (sizeof(balmBot[0]) * (floatsPerVertex + floatsPerUV));

		glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
		glBindVertexArray(mesh.vao);

		// Create VBO
		glGenBuffers(1, &mesh.vbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
		glBufferData(GL_ARRAY_BUFFER, sizeof(balmBot), balmBot, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

		// Strides between vertex coordinates
		GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

		// Create Vertex Attribute Pointers
		glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
		glEnableVertexAttribArray(2);

	}
}

void UDestroyMesh(GLMesh& mesh)
{
	glDeleteVertexArrays(1, &mesh.vao);
	glDeleteBuffers(1, &mesh.vbo);
}

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
	if (image)
	{
		flipImageVertically(image, width, height, channels);

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		if (channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (channels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			cout << "Not implemented to handle image with " << channels << " channels" << endl;
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		return true;
	}

	// Error loading the image
	return false;
}

void UDestroyTexture(GLuint textureId)
{
	glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
	// Compilation and linkage error reporting
	int success = 0;
	char infoLog[512];

	// Create a Shader program object.
	programId = glCreateProgram();

	// Create the vertex and fragment shader objects
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Retrive the shader source
	glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
	glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

	// Compile the vertex shader, and print compilation errors (if any)
	glCompileShader(vertexShaderId); // compile the vertex shader
	// check for shader compile errors
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glCompileShader(fragmentShaderId); // compile the fragment shader
	// check for shader compile errors
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	// Attached compiled shaders to the shader program
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);

	glLinkProgram(programId);   // links the shader program
	// check for linking errors
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glUseProgram(programId);    // Uses the shader program

	return true;
}

void UDestroyShaderProgram(GLuint programId)
{
	glDeleteProgram(programId);
}