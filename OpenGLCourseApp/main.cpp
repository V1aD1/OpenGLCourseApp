#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "CommonValues.h"
#include "PointLight.h"
#include "SpotLight.h"

#include "Mesh.h"
#include "Shader.h"
#include  "Window.h"
#include "Camera.h"
#include "Texture.h"
#include "DirectionalLight.h"
#include "Material.h"

#include <assimp/Importer.hpp>
#include "Model.h"

const float toRadians = 3.14159265f / 180.0f;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;
Camera camera;

Texture brickTexture, dirtTexture, plainTexture;

Material shinyMaterial;
Material dullMaterial;

Model xWing;

DirectionalLight mainLight;
PointLight pointLights[MAX_POINT_LIGHTS];
SpotLight spotLights[MAX_SPOT_LIGHTS];

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

// vertex shader
static const char* vShader = "Shaders/shader.vert";

//fragment shader
static const char* fShader = "Shaders/shader.frag";

void CalcAverageNormals(unsigned int* indices,
	unsigned int indiceCount,
	GLfloat* vertices,
	unsigned int verticeCount,
	unsigned int vLength,
	unsigned int normalOffset)
{

	//add all normal values to each vertex
	for (size_t i = 0; i < indiceCount; i += 3) {
		unsigned int in0 = indices[i] * vLength;
		unsigned int in1 = indices[i + 1] * vLength;
		unsigned int in2 = indices[i + 2] * vLength;
		glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
		glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
		glm::vec3 normal = glm::cross(v1, v2);
		normal = glm::normalize(normal);

		in0 += normalOffset; in1 += normalOffset; in2 += normalOffset;
		vertices[in0] += normal.x;
		vertices[in0 + 1] += normal.y;
		vertices[in0 + 2] += normal.z;

		vertices[in1] += normal.x;
		vertices[in1 + 1] += normal.y;
		vertices[in1 + 2] += normal.z;

		vertices[in2] += normal.x;
		vertices[in2 + 1] += normal.y;
		vertices[in2 + 2] += normal.z;
	}

	//normalize normal value for each vertex
	for (size_t i = 0; i < verticeCount / vLength; i++) {
		unsigned int nOffset = i * vLength + normalOffset;
		glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
		vec = glm::normalize(vec);
		vertices[nOffset] = vec.x;
		vertices[nOffset + 1] = vec.y;
		vertices[nOffset + 2] = vec.z;
	}
}

void CreateObjects() {
	unsigned int indices[] = {
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	//triangle vertices
	GLfloat vertices[] = {
		//		x	   y	 z	   u     v  norm.x  norm.y  norm.z
			-1.0f, -1.0f, -0.6f, 0.0f, 0.0f,   0.0f,   0.0f,   0.0f,
			 0.0f, -1.0f, 1.0f, 0.5f, 0.0f,   0.0f,   0.0f,   0.0f,
			 1.0f, -1.0f, -0.6f, 1.0f, 0.0f,   0.0f,   0.0f,   0.0f,
			 0.0f,  1.0f, 0.0f, 0.5f, 1.0f,   0.0f,   0.0f,   0.0f
	};

	unsigned int floorIndices[] =
	{
		0, 2 , 1,
		1, 2, 3,
	};

	GLfloat floorVertices[] = {
		-10.0f, 0.0f, -10.0f, 0.0f, 0.0f,   0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, -10.0f,  10.0f, 0.0f,  0.0f, -1.0f, 0.0f,
		-10.0f, 0.0f, 10.0f,  0.0f, 10.0f,  0.0f, -1.0f, 0.0f,
		10.0f, 0.0f, 10.0f,   10.0f, 10.0f, 0.0f, -1.0f, 0.0f,
	};

	CalcAverageNormals(indices, 12, vertices, 32, 8, 5);

	Mesh* obj1 = new Mesh();
	obj1->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj1);

	Mesh* obj2 = new Mesh();
	obj2->CreateMesh(vertices, indices, 32, 12);
	meshList.push_back(obj2);

	Mesh* obj3 = new Mesh();
	obj3->CreateMesh(floorVertices, floorIndices, 32, 6);
	meshList.push_back(obj3);
}

void CreateShaders() {
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(*shader1);
}

void RenderCustomObject(Shader shader, glm::mat4 projection, glm::mat4 model, Texture texture, Material material, Mesh* mesh) {
	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0, uniformSpecularIntensity = 0, uniformShininess = 0;

	uniformModel = shaderList[0].GetModelLocation();
	/*uniformProjection = shaderList[0].GetProjectionLocation();
	uniformView = shaderList[0].GetViewLocation();

	uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
	uniformShininess = shaderList[0].GetShininessLocation();

	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

	//must be called before RenderMesh()
	texture.UseTexture();
	material.UseMaterial(uniformSpecularIntensity, uniformShininess);
	mesh->RenderMesh();*/

	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
}

int main() {

	mainWindow = Window(1280, 800);
	mainWindow.Initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 2.0f, 0.1f);
	mainLight = DirectionalLight(1.0f, 1.0f, 1.0f,
		0.3f, 0.8f,
		2.0f, -1.0f, -2.0f);

	unsigned int pointLightCount = 0;

	/*pointLights[0] = PointLight(0.0f, 0.0f, 1.0f,
		0.5f, 1.0f,
		-5.0f, 1.0f, -5.0f,
		0.3f, 0.2f, 0.1f);

	pointLightCount++;

	pointLights[1] = PointLight(0.0f, 1.0f, 0.0f,
		0.1f, 1.0f,
		5.0f, 1.0f, -5.0f,
		0.3f, 0.2f, 0.1f);

	pointLightCount++;*/

	unsigned int spotLightCount = 0;
	/*spotLights[0] = SpotLight(0.8f, 0.1f, 0.1f,
		3.0f, 3.0f,
		3.0f, 4.0f, -3.0f,
		0.0f, -1.0f, 0.0f,
		0.3f, 0.2f, 0.1f,
		20.0f);

	spotLightCount++;

	spotLights[1] = SpotLight(0.1f, 0.1f, 0.8f,
		3.0f, 3.0f,
		-3.0f, 4.0f, -3.0f,
		0.0f, -1.0f, 0.0f,
		0.3f, 0.2f, 0.1f,
		5.0f);

	spotLightCount++;*/

	brickTexture = Texture("Textures/brick.png");
	brickTexture.LoadTextureA();
	dirtTexture = Texture("Textures/dirt.png");
	dirtTexture.LoadTextureA();
	plainTexture = Texture("Textures/plain.png");
	plainTexture.LoadTextureA();

	shinyMaterial = Material(4.0f, 256);
	dullMaterial = Material(0.3f, 4);

	xWing = Model();
	xWing.LoadModel("Models/x-wing.obj");

	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0,
		uniformEyePosition = 0, uniformSpecularIntensity = 0, uniformShininess = 0;

	glm::mat4 projection = glm::perspective(45.0f, mainWindow.GetBufferWidth() / mainWindow.GetBufferHeight(), 0.1f, 100.0f);

	//loop until window closed
	while (!mainWindow.GetShouldClose()) {
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		//get and handle user input events
		glfwPollEvents();

		camera.KeyControl(mainWindow.GetKeys(), deltaTime);
		camera.MouseControl(mainWindow.GetAndResetXChange(), mainWindow.GetAndResetYChange());

		//clear window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		shaderList[0].UseShader();
		
		//set camera to have a flashlight
		//spotLights[0].SetFlash(camera.GetCameraPosition(), camera.GetCameraDirection());

		shaderList[0].SetDirectionalLght(&mainLight);
		shaderList[0].SetPointLights(pointLights, pointLightCount);
		shaderList[0].SetSpotLights(spotLights, spotLightCount);
		
		uniformModel = shaderList[0].GetModelLocation();
		uniformProjection = shaderList[0].GetProjectionLocation();
		uniformView = shaderList[0].GetViewLocation();

		uniformEyePosition = shaderList[0].GetEyePositionLocation();
		uniformSpecularIntensity = shaderList[0].GetSpecularIntensityLocation();
		uniformShininess = shaderList[0].GetShininessLocation();

		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.CalculateViewMatrix()));
		glUniform3f(uniformEyePosition, camera.GetCameraPosition().x, camera.GetCameraPosition().y, camera.GetCameraPosition().z);

		glm::mat4 model;
		/*model = glm::translate(model, glm::vec3(0.0f, -1.0f, -6.0f));
		//model = glm::rotate(model, 45.0f * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

		//must be called before RenderMesh()
		brickTexture.UseTexture();
		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[0]->RenderMesh();*/

		/*model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.0f, 1.4f, -6.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		dirtTexture.UseTexture();
		dullMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[1]->RenderMesh();*/

		model = glm::mat4();
		model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		plainTexture.UseTexture();
		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		meshList[2]->RenderMesh();

		model = glm::mat4();
		model = glm::translate(model, glm::vec3(-10.0f, -2.0f, 5.0f));
		model = glm::scale(model, glm::vec3(0.006f, 0.006f, 0.006f));
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
		shinyMaterial.UseMaterial(uniformSpecularIntensity, uniformShininess);
		xWing.RenderModel();

		glUseProgram(0);

		mainWindow.SwapBuffers();
	}

	return 0;
}