#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <unistd.h>


//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#include "helpers.hpp"

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

GLuint gProgram[5];

int gWidth, gHeight;

GLint modelingMatrixLoc[5];
GLint viewingMatrixLoc[5];
GLint projectionMatrixLoc[5];
GLint eyePosLoc[5];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;

float cameraZoom = 120.f; //45.0f;
int width = 1200, height = 900;
float lastX = width/2.0f;
float lastY = height/2.0f;
float yaw = -90.0f;
float pitch = 0.0f;



glm::vec3 eyePos(0.f, 4.f, 8.f);
glm::vec3 cameraFront(0.f, 0.f, -1.f);
glm::vec3 cameraUp(0.f, 1.f, 0.f);

//glm::vec3 eyePos(0.f, 0.f, 0.f);
//glm::vec3 cameraFront(0.f, 0.f, -1.f);
//glm::vec3 cameraUp(0.f, 1.f, 0.f);

//glm::vec3 eyePos(15.f, 15.f, 15.f);
//glm::vec3 cameraFront(-1.f, -1.f, -1.f);
//glm::vec3 cameraUp(-1.f, 1.7f, -1.f);

int activeProgramIndex = 0;

float deltaTime = 0.f; 
float currentTime = 0.f;
float lastTime = 0.f;

struct Vertex
{
    Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Texture
{
    Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
    GLfloat u, v;
};

struct Normal
{
    Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
    GLuint vIndex[3], tIndex[3], nIndex[3];
};

size_t numberOfObj = 0;

vector<Vertex> gVertices[5];
vector<Texture> gTextures[5];
vector<Normal> gNormals[5];
vector<Face> gFaces[5];

GLuint vao[5];
GLuint gVertexAttribBuffer[5], gIndexBuffer[5];
GLint gInVertexLoc[5], gInNormalLoc[5];
int gVertexDataSizeInBytes[5], gNormalDataSizeInBytes[5];

GLuint gTexCubeMap;
GLuint cubeVAO, cubeAttribBuffer, cubeIndexBuffer;
GLuint cubeSampler;

GLuint gTexMirrorColor, gTexMirrorDepth;
GLuint gFbo, gRbo;
GLuint fboEnvSampler;
size_t fboWidth = 2048;
size_t fboHeight = 2048;

//float cubeVertices[] = {
//    // positions
//    //front face
//    -1.0f,  1.0f, -1.0f, //0
//    -1.0f, -1.0f, -1.0f, //1
//     1.0f, -1.0f, -1.0f, //2
//     1.0f,  1.0f, -1.0f, //3
//
//    //back face
//    -1.0f,  1.0f, +1.0f, //4
//    -1.0f, -1.0f, +1.0f, //5
//     1.0f, -1.0f, +1.0f, //6
//     1.0f,  1.0f, +1.0f, //7
//};

float cubeVertices[] = {
    // positions
    //front face
    -0.5f,  0.5f, -0.5f, //0
    -0.5f, -0.5f, -0.5f, //1
     0.5f, -0.5f, -0.5f, //2
     0.5f,  0.5f, -0.5f, //3

    //back face
    -0.5f,  0.5f, +0.5f, //4
    -0.5f, -0.5f, +0.5f, //5
     0.5f, -0.5f, +0.5f, //6
     0.5f,  0.5f, +0.5f, //7
};

int cubeIndices[] = {
    //front face
    0, 1, 2,
    2, 3, 0,
    //back face
    6, 5, 4,
    6, 4, 7,
    //left face
    0, 4, 5,
    0, 5, 1,
    //right face
    6, 3, 2,
    6, 7, 3,
    //top face
    7, 4, 0,
    0, 3, 7,
    //bottom face
    6, 2, 1,
    1, 5, 6
};

void printGlError()
{
    GLenum glErrNo;
    glErrNo = glGetError();
    //std::cout << "gl error no: " << glErrNo << std::endl;
    switch(glErrNo)
    {
        case GL_NO_ERROR:
            std::cout << "GL_NO_ERROR"<< std::endl;
            break;
        case GL_INVALID_ENUM:
            std::cout << "GL_INVALID_ENUM"<< std::endl;
            break;
        case GL_INVALID_VALUE:
            std::cout << "GL_INVALID_VALUE"<< std::endl;
            break;
        case GL_INVALID_OPERATION:
            std::cout << "GL_INVALID_OPERATION"<< std::endl;
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION"<< std::endl;
            break;
        case GL_OUT_OF_MEMORY:
            std::cout << "GL_OUT_OF_MEMORY"<< std::endl;
            break;
        case GL_STACK_UNDERFLOW:
            std::cout << "GL_STACK_UNDERFLOW"<< std::endl;
            break;
        case GL_STACK_OVERFLOW:
            std::cout << "GL_STACK_OVERFLOW"<< std::endl;
            break;
    }

}

bool ParseObj(const string& fileName)
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            stringstream str(curLine);
            GLfloat c1, c2, c3;
            GLuint index[9];
            string tmp;

            if (curLine.length() >= 2)
            {
                if (curLine[0] == 'v')
                {
                    if (curLine[1] == 't') // texture
                    {
                        str >> tmp; // consume "vt"
                        str >> c1 >> c2;
                        gTextures[numberOfObj].push_back(Texture(c1, c2));
                    }
                    else if (curLine[1] == 'n') // normal
                    {
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        gNormals[numberOfObj].push_back(Normal(c1, c2, c3));
                    }
                    else // vertex
                    {
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        gVertices[numberOfObj].push_back(Vertex(c1, c2, c3));
                    }
                }
                else if (curLine[0] == 'f') // face
                {
                    str >> tmp; // consume "f"
					char c;
					int vIndex[3],  nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0]; 
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1]; 
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2]; 

//					assert(vIndex[0] == nIndex[0] &&
//						   vIndex[1] == nIndex[1] &&
//						   vIndex[2] == nIndex[2]); // a limitation for now
//
					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

                    gFaces[numberOfObj].push_back(Face(vIndex, tIndex, nIndex));
                }
                else
                {
                    cout << "Ignoring unidentified line in obj file: " << curLine << endl;
                }
            }

            //data += curLine;
            if (!myfile.eof())
            {
                //data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

	assert(gVertices[numberOfObj].size() == gNormals[numberOfObj].size());

    numberOfObj++;
    return true;
}

bool ReadDataFromFile(
    const string& fileName, ///< [in]  Name of the shader file
    string&       data)     ///< [out] The contents of the file
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            data += curLine;
            if (!myfile.eof())
            {
                data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

    return true;
}

GLuint createVS(const char* shaderName)
{
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

	return vs;
}

GLuint createFS(const char* shaderName)
{
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

	return fs;
}

void initShaders()
{
	// Create the programs

    gProgram[0] = glCreateProgram();
	gProgram[1] = glCreateProgram();

	// Create the shaders for both programs

    GLuint vs1 = createVS("vert.glsl");
    GLuint fs1 = createFS("frag.glsl");

	GLuint vs2 = createVS("vert2.glsl");
	GLuint fs2 = createFS("frag2.glsl");

	// Attach the shaders to the programs

	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);

	glAttachShader(gProgram[1], vs2);
	glAttachShader(gProgram[1], fs2);

	// Link the programs

    glLinkProgram(gProgram[0]);
	GLint status;
	glGetProgramiv(gProgram[0], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	glLinkProgram(gProgram[1]);
	glGetProgramiv(gProgram[1], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "Program link failed" << endl;
		exit(-1);
	}

	// Get the locations of the uniform variables from both programs

	for (int i = 0; i < 2; ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}
}

void initVBO(size_t objId)
{
    glGenVertexArrays(1, &vao[objId]);
    assert(vao[objId] > 0);
    glBindVertexArray(vao[objId]);
    cout << "vao = " << vao[objId] << endl;

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	assert(glGetError() == GL_NONE);

	glGenBuffers(1, &gVertexAttribBuffer[objId]);
	glGenBuffers(1, &gIndexBuffer[objId]);

	assert(gVertexAttribBuffer[objId] > 0 && gIndexBuffer[objId] > 0);

	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer[objId]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer[objId]);

	gVertexDataSizeInBytes[objId] = gVertices[objId].size() * 3 * sizeof(GLfloat);
	gNormalDataSizeInBytes[objId] = gNormals[objId].size() * 3 * sizeof(GLfloat);
	int indexDataSizeInBytes = gFaces[objId].size() * 3 * sizeof(GLuint);
	GLfloat* vertexData = new GLfloat [gVertices[objId].size() * 3];
	GLfloat* normalData = new GLfloat [gNormals[objId].size() * 3];
	GLuint* indexData = new GLuint [gFaces[objId].size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

	for (int i = 0; i < gVertices[objId].size(); ++i)
	{
		vertexData[3*i] = gVertices[objId][i].x;
		vertexData[3*i+1] = gVertices[objId][i].y;
		vertexData[3*i+2] = gVertices[objId][i].z;

        minX = std::min(minX, gVertices[objId][i].x);
        maxX = std::max(maxX, gVertices[objId][i].x);
        minY = std::min(minY, gVertices[objId][i].y);
        maxY = std::max(maxY, gVertices[objId][i].y);
        minZ = std::min(minZ, gVertices[objId][i].z);
        maxZ = std::max(maxZ, gVertices[objId][i].z);
	}

    std::cout << "minX = " << minX << std::endl;
    std::cout << "maxX = " << maxX << std::endl;
    std::cout << "minY = " << minY << std::endl;
    std::cout << "maxY = " << maxY << std::endl;
    std::cout << "minZ = " << minZ << std::endl;
    std::cout << "maxZ = " << maxZ << std::endl;

	for (int i = 0; i < gNormals[objId].size(); ++i)
	{
		normalData[3*i] = gNormals[objId][i].x;
		normalData[3*i+1] = gNormals[objId][i].y;
		normalData[3*i+2] = gNormals[objId][i].z;
	}

	for (int i = 0; i < gFaces[objId].size(); ++i)
	{
		indexData[3*i] = gFaces[objId][i].vIndex[0];
		indexData[3*i+1] = gFaces[objId][i].vIndex[1];
		indexData[3*i+2] = gFaces[objId][i].vIndex[2];
	}


	glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes[objId] + gNormalDataSizeInBytes[objId], 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes[objId], vertexData);
	glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes[objId], gNormalDataSizeInBytes[objId], normalData);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

	// done copying; can free now
	delete[] vertexData;
	delete[] normalData;
	delete[] indexData;

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes[objId]));
}


void initTexture()
{
    glGenTextures(1, &gTexCubeMap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, gTexCubeMap);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    const char* cubeFaces[] = {"hw2_support_files/skybox_texture_sea/right.jpg",
                               "hw2_support_files/skybox_texture_sea/left.jpg",
                               "hw2_support_files/skybox_texture_sea/top.jpg",
                               "hw2_support_files/skybox_texture_sea/bottom.jpg",
                               "hw2_support_files/skybox_texture_sea/front.jpg",
                               "hw2_support_files/skybox_texture_sea/back.jpg"};

    unsigned char* img;
    for(int i=0; i < 6; ++i)
    {
        int width, height, nrOfChannels;

        img = load_image(cubeFaces[i], &width, &height, &nrOfChannels, 4);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8,
                width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);

        //free the images with std_image_free
    }

    glGenSamplers(1, &cubeSampler);

    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glSamplerParameteri(cubeSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(cubeSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(cubeSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(cubeSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(cubeSampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindSampler(0, cubeSampler);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    //std::cout << "cubemap errors: ";
    //printGlError();
}

void initCubeVBO()
{
    glGenVertexArrays(1, &cubeVAO);
    assert(cubeVAO > 0);
    glBindVertexArray(cubeVAO);

    glEnableVertexAttribArray(0);

    glGenBuffers(1, &cubeAttribBuffer);
    glGenBuffers(1, &cubeIndexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, cubeAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBuffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW); 

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //glDisableVertexAttribArray(cubeVAO);
}

void initCubeShaders()
{
    gProgram[2] = glCreateProgram();

    GLuint vs = createVS("cube_vert.glsl");
    GLuint fs = createFS("cube_frag.glsl");

	glAttachShader(gProgram[2], vs);
	glAttachShader(gProgram[2], fs);

    glLinkProgram(gProgram[2]);
    GLint status;

    glGetProgramiv(gProgram[2], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "cubemap program link failed" << endl;
		exit(-1);
	}

    modelingMatrixLoc[2] = glGetUniformLocation(gProgram[2], "modelingMatrix");
    viewingMatrixLoc[2] = glGetUniformLocation(gProgram[2], "viewingMatrix");
    projectionMatrixLoc[2] = glGetUniformLocation(gProgram[2], "projectionMatrix");
    eyePosLoc[2] = glGetUniformLocation(gProgram[2], "eyePos");

}

void initMirrorShaders()
{
    GLuint mirrorProgId = 3;
    gProgram[mirrorProgId] = glCreateProgram();

    GLuint vs = createVS("mirror_vert.glsl");
    GLuint fs = createFS("mirror_frag.glsl");

	glAttachShader(gProgram[mirrorProgId], vs);
	glAttachShader(gProgram[mirrorProgId], fs);

    glLinkProgram(gProgram[mirrorProgId]);
    GLint status;

    glGetProgramiv(gProgram[mirrorProgId], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "mirror obj program link failed" << endl;
		exit(-1);
	}

    modelingMatrixLoc[mirrorProgId] = glGetUniformLocation(gProgram[mirrorProgId], "modelingMatrix");
    viewingMatrixLoc[mirrorProgId] = glGetUniformLocation(gProgram[mirrorProgId], "viewingMatrix");
    projectionMatrixLoc[mirrorProgId] = glGetUniformLocation(gProgram[mirrorProgId], "projectionMatrix");
    eyePosLoc[mirrorProgId] = glGetUniformLocation(gProgram[mirrorProgId], "eyePos");

}


void initFBO()
{
    glGenFramebuffers(1, &gFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, gFbo);


        //glGenRenderbuffers(1, &gRbo);
        //glBindRenderbuffer(GL_RENDERBUFFER, gRbo);
        //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fboWidth, fboHeight);
        //glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenTextures(1, &gTexMirrorColor);
    glGenTextures(1, &gTexMirrorDepth);

    for(int i=0; i < 6; ++i)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP, gTexMirrorColor);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8,
                fboWidth, fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glBindTexture(GL_TEXTURE_CUBE_MAP, gTexMirrorDepth);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32F, fboWidth, fboHeight, 0,
                GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    }
    
    

    glGenSamplers(1, &fboEnvSampler);

    glSamplerParameteri(fboEnvSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(fboEnvSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(fboEnvSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(fboEnvSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(fboEnvSampler, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gRbo);
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
    //        GL_TEXTURE_2D, gRbo);



    //std::cout << "framebuffer errors: ";
    //printGlError();
}

void init() 
{
	ParseObj("bunny.obj");
	ParseObj("armadillo.obj");
	ParseObj("teapot.obj");
	//ParseObj("cube.obj");

    glEnable(GL_DEPTH_TEST);

    initShaders();
    initCubeShaders();
    initMirrorShaders();

    initVBO(0);
    initVBO(1);
    initVBO(2);
    initCubeVBO();
    initTexture();

    initFBO();
}

void drawModel(size_t objId)
{
    glBindVertexArray(vao[objId]);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer[objId]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer[objId]);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes[objId]));

	glDrawElements(GL_TRIANGLES, gFaces[objId].size() * 3, GL_UNSIGNED_INT, 0);
}


void displayObjects()
{
    //object 1 //bunny
    static float angle = 0;
    static float rotSpeed = 0.001;

    glm::mat4 matT = glm::translate<float>(glm::mat4(1.0f), glm::vec3(7.f, 0.f, 0.f));
    glm::mat4 matR = glm::rotate<float>(glm::mat4(1.0f), angle, glm::vec3(0.f, 0.f, 1.0f));

    modelingMatrix = matR * matT;

    angle += rotSpeed;

	glUseProgram(gProgram[activeProgramIndex]);
	glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));
    
    drawModel(0);
    //object 2 //armadillo

    static float angle2 = 0;
    static float rotSpeed2 = 0.002;

    matT = glm::translate<float>(glm::mat4(1.0f), glm::vec3(-5.0f, 0.f, 0.f));
    matR = glm::rotate<float>(glm::mat4(1.0f), angle2, glm::vec3(0.f, 1.f, 0.0f));

    modelingMatrix = matR * matT;

    angle2 += rotSpeed2;
    
	glUseProgram(gProgram[activeProgramIndex]);
	glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));
    
    drawModel(1);
}

void display()
{
    //glClearColor(0, 0, 0, 1);
    //glClearDepth(1.0f);
    //glClearStencil(0);
    //glClear(GL_DEPTH_BUFFER_BIT);

	static float angle = 0;
    static float rotSpeed = 1.0f;

	float angleRad = (float) (angle / 180.0) * M_PI;
	
	// Compute the modeling matrix

	//modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, -0.4f, -5.0f));
	//modelingMatrix = glm::rotate(modelingMatrix, angleRad, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(5.0f, -0.4f, 0.0f));   // same as above but more clear
    //glm::mat4 matR = glm::rotate(glm::mat4(1.0), angleRad, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 matRx = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
    glm::mat4 matRy = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 matRz = glm::rotate<float>(glm::mat4(1.0), angleRad, glm::vec3(0.0, 0.0, 1.0));


    static float scalingFactor = 4.0f;
    glm::mat4 shrink = glm::scale<float>(glm::mat4(1.0f), glm::vec3(scalingFactor ,scalingFactor ,scalingFactor) );
    
    modelingMatrix = matRy * matRx * shrink;

    // Let's make some alternating roll rotation
    static float rollDeg = 0;
    static float changeRoll = 2.5;
    float rollRad = (float) (rollDeg / 180.f) * M_PI;
    rollDeg += changeRoll* rotSpeed;
    if (rollDeg >= 10.f || rollDeg <= -10.f)
    {
        changeRoll *= -1.f;
    }
    glm::mat4 matRoll = glm::rotate<float>(glm::mat4(1.0), rollRad, glm::vec3(1.0, 0.0, 0.0));

    // Let's make some pitch rotation
    static float pitchDeg = 0;
    static float changePitch = 0.1;
    float startPitch = 0;
    float endPitch = 90;
    float pitchRad = (float) (pitchDeg / 180.f) * M_PI;
    pitchDeg += changePitch* rotSpeed;
    if (pitchDeg >= endPitch)
    {
        changePitch = 0;
    }
    //glm::mat4 matPitch = glm::rotate<float>(glm::mat4(1.0), pitchRad, glm::vec3(0.0, 0.0, 1.0));
    //modelingMatrix = matRoll * matPitch * modelingMatrix; // gimbal lock
    //modelingMatrix = matPitch * matRoll * modelingMatrix;   // no gimbal lock

    glm::quat q0(0, 1, 0, 0); // along x
    glm::quat q1(0, 0, 1, 0); // along y
    glm::quat q = glm::mix(q0, q1, (pitchDeg - startPitch) / (endPitch - startPitch));

    float sint = sin(rollRad / 2);
    glm::quat rollQuat(cos(rollRad/2), sint * q.x, sint * q.y, sint * q.z);
    glm::quat pitchQuat(cos(pitchRad/2), 0, 0, 1 * sin(pitchRad/2));
    //modelingMatrix = matT * glm::toMat4(pitchQuat) * glm::toMat4(rollQuat) * modelingMatrix;
    modelingMatrix = matT * glm::toMat4(rollQuat) * glm::toMat4(pitchQuat) * modelingMatrix; // roll is based on pitch

    //cout << rollQuat.w << " " << rollQuat.x << " " << rollQuat.y << " " << rollQuat.z << endl;

	// Set the active program and the values of its uniform variables

	glUseProgram(gProgram[activeProgramIndex]);
	glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));

	// Draw the scene
    drawModel(0);

	angle += 0.5* rotSpeed;
}

void displayCube()
{
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    //float scalingFactor = 50.f;
    modelingMatrix =glm::mat4(1.0f);
    //modelingMatrix = glm::scale<float>(glm::mat4(1.0f), glm::vec3(scalingFactor, scalingFactor, scalingFactor) );
    modelingMatrix = glm::translate<float>(glm::mat4(1.0f), eyePos) * modelingMatrix;
    //modelingMatrix = glm::lookAt(glm::vec3(0.f, 0.f,0.f),
    //                             glm::normalize(eyePos + cameraFront),
    //                             glm::normalize(cameraUp));

    glBindTexture(GL_TEXTURE_CUBE_MAP, gTexCubeMap);
    glBindSampler(0, cubeSampler);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glUseProgram(gProgram[2]);
    glUniformMatrix4fv(projectionMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(viewingMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    glUniformMatrix4fv(modelingMatrixLoc[2], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[2], 1, glm::value_ptr(eyePos));

    //glEnableVertexAttribArray(cubeVAO);

	//glBindBuffer(GL_ARRAY_BUFFER, cubeAttribBuffer);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBuffer);

	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 0, 0);

    glBindVertexArray(cubeVAO);
	glDrawElements(GL_TRIANGLES, sizeof(cubeIndices)/sizeof(int), GL_UNSIGNED_INT, 0);
    //glDisableVertexAttribArray(cubeVAO);
}


void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);

    //glMatrixMode(GL_PROJECTION);
    //glLoadIdentity();
    //glOrtho(-10, 10, -10, 10, -10, 10);
    //gluPerspective(45, 1, 1, 100);


    //handle euler angles
    float yawInRads = (yaw/180) * M_PI;
    float pitchInRads = (pitch/180) * M_PI;

    glm::vec3 cameraDir;
    cameraDir.x = cos(yawInRads) * cos(pitchInRads);
    cameraDir.y = sin(pitchInRads);
    cameraDir.z = sin(yawInRads) * cos(pitchInRads);

    cameraFront = glm::normalize(cameraDir);

	// Use perspective projection
	float fovyRad = (float) (cameraZoom / 180.0) * M_PI;
    float aspectRat = (float) 1.0f;
	projectionMatrix = glm::perspective(fovyRad, aspectRat, 0.01f, 100.0f);

	//fovyRad = (float) (125.0 / 180.0) * M_PI;
	//projectionMatrix = glm::perspective(fovyRad, 1.0f, 1.0f, 100.0f);

	// Assume default camera position and orientation (camera is at
	// (0, 0, 0) with looking at -z direction and its up vector pointing
	// at +y direction)

	//viewingMatrix = glm::mat4(1);
    viewingMatrix = glm::lookAt(eyePos,
                                eyePos + cameraFront,
                                glm::normalize(cameraUp));
    //static float angle = 0.f;
    //viewingMatrix *= glm::rotate<float>(glm::mat4(1.0), (angle), glm::vec3(0.f, 1.f, 0.f));

    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();
    //angle += 0.05f;
}

void fillCubeTex(GLFWwindow* window)
{
    glViewport(0, 0, fboWidth, fboHeight);

    float fovRad = (float) (90.0f / 180.0) * M_PI;
    float aspectRat = 1.0f;
	projectionMatrix = glm::perspective(fovRad, aspectRat, 0.01f, 100.0f);
    
    glm::vec3 mirrorObjPos = glm::vec3(0.0f, 0.f, 0.f);
    glm::vec3 offScreenFront = glm::vec3(0.f, 0.0f, 1.0f);
    glm::vec3 offScreenUp = glm::vec3(0.f, -1.0f, 0.0f);



    for(size_t i = 0; i < 6; ++i)
    {
        switch(i)
        {
            case 0://+x
                offScreenFront = glm::vec3(1.f, 0.0f, 0.0f);
                offScreenUp = glm::vec3(0.f, -1.0f, 0.0f);
                break;
            case 1://-x 
                offScreenFront = glm::vec3(-1.f, 0.0f, 0.0f);
                offScreenUp = glm::vec3(0.f, -1.0f, 0.0f);
                break;
            case 2://+y
                offScreenFront = glm::vec3(0.f, 1.0f, 0.0f);
                offScreenUp = glm::vec3(0.f, 0.0f, 1.0f);
                break;
            case 3://-y
                offScreenFront = glm::vec3(0.f, -1.0f, 0.0f);
                offScreenUp = glm::vec3(0.f, 0.0f, -1.0f);
                break;
            case 4://+z
                offScreenFront = glm::vec3(0.f, 0.0f, 1.0f);
                offScreenUp = glm::vec3(0.f, -1.0f, 0.0f);
                break;
            case 5://-z
                offScreenFront = glm::vec3(0.f, 0.0f, -1.0f);
                offScreenUp = glm::vec3(0.f, -1.0f, 0.0f);
                break;
            default:
                break;
        }

        viewingMatrix = glm::lookAt(mirrorObjPos,
                mirrorObjPos + offScreenFront,
                glm::normalize(offScreenUp));

        //glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, gTexMirrorColor);
        //glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8,
        //        fboWidth, fboHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        //glBindTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, gTexMirrorDepth);
        
        //glNamedFramebufferTexture(gFbo, GL_COLOR_ATTACHMENT0, gTexMirrorColor, 0);
        //glNamedFramebufferTexture(gFbo, GL_DEPTH_ATTACHMENT, gTexMirrorDepth, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, gFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, gTexMirrorColor, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, gTexMirrorDepth, 0);

        //printGlError();
        
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;


        displayCube();
        //set depth buffer to max so env is always overwritten by objects
        glClearDepth(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);
        //display();//display diffuse obj
        displayObjects();

    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        //glShadeModel(GL_SMOOTH);
        activeProgramIndex = 0;
        //std::cout << "active program 0" << std::endl;
    }
    else if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        //glShadeModel(GL_SMOOTH);
        activeProgramIndex = 1;
        //std::cout << "active program 1" << std::endl;
    }
    else if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        //glShadeModel(GL_FLAT);
    }

    const float cameraSpeed = 11.65f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        eyePos += cameraFront * cameraSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        eyePos -= cameraFront * cameraSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        eyePos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        eyePos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        eyePos -= glm::normalize(cameraUp) * cameraSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        eyePos += glm::normalize(cameraUp) * cameraSpeed * deltaTime;
    }

    //if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    //{
    //    cameraZoom -= deltaTime * 3.f;
    //}
    //if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    //{
    //    cameraZoom += deltaTime * 3.f;
    //}

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    {
        pitch += 1.5f;
        if (pitch > 89.0f)
            pitch = 89.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        pitch -= 1.5f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        yaw += 1.5f;
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        yaw -= 1.5f;
    }

}

void handleMouse(GLFWwindow* window, double xposIn, double yposIn)
{

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    static bool mouseInit = true;
    if (mouseInit)
    {
        lastX = xpos;
        lastY = ypos;
        mouseInit = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;


    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
    {
        return;
    }

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

}

void drawMirrorObj(int objId)
{
    GLuint mirrorShaderId = 3;

    glBindTexture(GL_TEXTURE_CUBE_MAP, gTexMirrorColor);
    glBindSampler(0, fboEnvSampler);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    
    static float angle = 0; 
    static float rotSpeed = 0.007f;
    modelingMatrix = glm::rotate<float>(glm::mat4(1.0f), angle, glm::vec3(0.f, 1.f, 0.f));

    angle += rotSpeed;


	glUseProgram(gProgram[mirrorShaderId]);
	glUniformMatrix4fv(projectionMatrixLoc[mirrorShaderId], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(viewingMatrixLoc[mirrorShaderId], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	glUniformMatrix4fv(modelingMatrixLoc[mirrorShaderId], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	glUniform3fv(eyePosLoc[mirrorShaderId], 1, glm::value_ptr(eyePos));

    //glBindVertexArray
    drawModel(objId); 
}


void mainLoop( GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window))
    {
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        currentTime = glfwGetTime();
        


        //use the original environment
        //glActiveTexture(GL_TEXTURE0);

        glBindFramebuffer(GL_FRAMEBUFFER,  0);

        //draw diffuse objs and env
        reshape(window, width, height);
        displayCube();
        //set depth buffer to max so env is always overwritten by objects
        glClearDepth(1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);
        //display();//display diffuse obj
        displayObjects();
        
        //render to cubemap
        //TODO set viewport 
        fillCubeTex(window);

        reshape(window, width, height);

        glTextureBarrier();
        drawMirrorObj(2);

        glfwSwapBuffers(window);
        glfwPollEvents();
        //sleep(100);
    }
}


void APIENTRY messageCallBack(GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
  std::cout << "AN ERROR OCCURRED" << std::endl;
}


int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    cout << "Process ID: " << getpid() << endl;

    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);//TODO remove for the final program

    //int width = 640, height = 480;
    window = glfwCreateWindow(width, height, "Simple Example", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = {0};
    strcpy(rendererInfo, (const char*) glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*) glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    init();

    glfwSetKeyCallback(window, keyboard);
    glfwSetWindowSizeCallback(window, reshape);

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
    glfwSetCursorPosCallback(window, handleMouse);

    //glEnable(GL_DEBUG_OUTPUT);
    //glDebugMessageCallbackARB(messageCallBack, 0);

    reshape(window, width, height); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
