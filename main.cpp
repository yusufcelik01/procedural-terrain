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
#include <glm/gtx/normal.hpp>
#include <glm/gtx/intersect.hpp>

#define BUFFER_OFFSET(i) ((char*)NULL + (i))
#define EPSILON 1e-3
#define CAR_STOP_TRESHOLD 0.008

using namespace std;

GLuint gProgram[16];
GLuint terrainPrograms[8];

size_t numberOfObj = 0;
size_t numberOfPrograms = 0;
size_t numberOfTerrainProg = 0;

int gWidth, gHeight;


GLint modelingMatrixLoc[16];
GLint viewingMatrixLoc[16];
GLint projectionMatrixLoc[16];
GLint eyePosLoc[16];

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;

float cameraZoom = 120.f; //45.0f;
int width = 1200, height = 900;
float lastX = width/2.0f;
float lastY = height/2.0f;
float yaw = -90.0f;
float pitch = 0.0f;



glm::vec3 eyePos(0.01f, 4.f, 0.01f);
//glm::vec3 eyePos(0.f, 4.f, 8.f);
glm::vec3 cameraFront(0.f, 0.f, -1.f);
glm::vec3 cameraUp(0.f, 1.f, 0.f);

glm::vec3 carPos(0.01f, 0.f, 0.01f);
glm::vec3 carDir(0.f, 0.f, -1.f);

float carSpeed = 0.f;
float carYaw = -90;

int activeProgramIndex = 0;
int activeTerrainProgIndex = 0;
int wireframe = 0;

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


float perlinNoise(glm::vec3 texCoords);


GLuint vertexCount = 1000;
GLfloat terrainSpan = 30;
GLfloat noiseScale = 1.20;

vector<Vertex> gVertices[5];
vector<Texture> gTextures[5];
vector<Normal> gNormals[5];
vector<Face> gFaces[5];

GLuint vao[5];
GLuint ubo[4];
size_t uboSizes[4];

GLuint gVertexAttribBuffer[5], gIndexBuffer[5];
GLint gInVertexLoc[5], gInNormalLoc[5];
int gVertexDataSizeInBytes[5], gNormalDataSizeInBytes[5];

void initTerrain()
{
    
    gVertices[numberOfObj].push_back(Vertex(0.0f, 0.0f, 0.0f));
    gNormals[numberOfObj].push_back(Normal(0, 1, 0));

    int index[3];
    index[0] = 0;
    index[1] = 0;
    index[2] = 0;
    gFaces[numberOfObj].push_back(Face(index, index, index));


    numberOfObj++;
}

void printVec(glm::vec3 v)
{
    cout << "("  << v.x 
         << ", " << v.y
         << ", " << v.z << ")";
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


GLuint createGS(const char* shaderName)
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

    GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(gs, 1, &shader, &length);
    glCompileShader(gs);

    char output[1024] = {0};
    glGetShaderInfoLog(gs, 1024, &length, output);
    printf("GS compile log: %s\n", output);

	return gs;
}

void initTerrainShaders()
{
	// Create the programs

    
    terrainPrograms[0] = glCreateProgram();
    //terrainPrograms[1] = glCreateProgram();
    //gProgram[numberOfTerrainProg] = glCreateProgram();

	// Create the shaders for both programs

    GLuint vs1 = createVS("terrain-vert.glsl");
    GLuint fs1 = createFS("terrain-frag.glsl");
    GLuint gs1 = createGS("terrain-geo.glsl");

	glAttachShader(terrainPrograms[0], vs1);
	glAttachShader(terrainPrograms[0], fs1);
	glAttachShader(terrainPrograms[0], gs1);


	// Link the programs
    glLinkProgram(terrainPrograms[0]);
	GLint status;
	glGetProgramiv(terrainPrograms[0], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		cout << "terrain program 1 link failed" << endl;
		exit(-1);
	}

    numberOfTerrainProg = 1;
}

void initShaders()
{
	// Create the programs

    gProgram[0] = glCreateProgram();
	gProgram[1] = glCreateProgram();

	// Create the shaders for both programs

    GLuint vs1 = createVS("vert.glsl");
    GLuint fs1 = createFS("frag.glsl");
    GLuint gs1 = createGS("geo.glsl");

	GLuint vs2 = createVS("vert2.glsl");
	GLuint fs2 = createFS("frag2.glsl");
    GLuint gs2 = createGS("geo2.glsl");

	// Attach the shaders to the programs

	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);
	glAttachShader(gProgram[0], gs1);

	glAttachShader(gProgram[1], vs2);
	glAttachShader(gProgram[1], fs2);
	glAttachShader(gProgram[1], gs2);

	// Link the programs
        cout << glGetError() << endl;

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

	//for (int i = 0; i < 2; ++i)
	//{
	//	modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
	//	viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
	//	projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
	//	eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	//}

    numberOfPrograms = 2;
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

void initUBO()
{
    glGenBuffers(1, &ubo[0]);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo[0]);

    uboSizes[0] = sizeof(glm::mat4) * 3;//matrices;
    uboSizes[1] = sizeof(GLuint)*2 + sizeof(GLfloat)*2;//matrices;

    glBufferData(GL_UNIFORM_BUFFER, uboSizes[0] + uboSizes[1], 0, GL_DYNAMIC_COPY);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo[0], 0, uboSizes[0]+uboSizes[1]);
    //glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo[0]);
    //glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo[0]);
    //glBindBufferRange(GL_UNIFORM_BUFFER, 1, ubo[0], uboSizes[0], uboSizes[1]);
}

void updateUniforms()
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo[0]);

    //matrices block

    //GLuint uniformBlockIndex;
    //GLsizei uniformBlockSize;
    //uniformBlockIndex = glGetUniformBlockIndex(terrainPrograms[0], "matrices");
    //glGetActiveUniformBlockiv(terrainPrograms[0], uniformBlockIndex,
    //                                 GL_UNIFORM_BLOCK_DATA_SIZE,
    //                                 &uniformBlockSize);

    //cout << "uniformBlockSize " << uniformBlockSize << endl;
    //cout << "float " << sizeof(GLfloat) << endl;
    //cout << "uint " << sizeof(GLuint) << endl;
    //cout << "mat4 " << sizeof(glm::mat4) << endl;
    //cout << "mat4 " << sizeof(glm::mat4) << endl;
    //cout << "ubosizes " << uboSizes[0] + uboSizes[1] << endl;

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(modelingMatrix));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(viewingMatrix));
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projectionMatrix));

    //terrainData block
    glBufferSubData(GL_UNIFORM_BUFFER, uboSizes[0], sizeof(GLfloat), &terrainSpan);
    glBufferSubData(GL_UNIFORM_BUFFER, uboSizes[0] + 1 * sizeof(GLfloat), sizeof(GLuint), &vertexCount);
    glBufferSubData(GL_UNIFORM_BUFFER, uboSizes[0] + 1 * sizeof(GLfloat)+sizeof(GLuint), sizeof(GLuint), &noiseScale);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void init() 
{
    initTerrain();

	//ParseObj("armadillo.obj");
	ParseObj("bunny.obj");

    glEnable(GL_DEPTH_TEST);
    //initShaders();
    initTerrainShaders();
    initVBO(0);
    initVBO(1);

    initUBO();
    updateUniforms();

    wireframe = 1; glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void drawTerrain(size_t terrainId)
{
	glUseProgram(terrainPrograms[activeTerrainProgIndex]);
	//glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	//glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
	//glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
	//glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));
    /////////

    glBindVertexArray(vao[terrainId]);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer[terrainId]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer[terrainId]);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes[terrainId]));

	//glDrawElementsInstanced(GL_POINTS, gFaces[terrainId].size(), GL_UNSIGNED_INT, 0, 1000*1000);
	glDrawElementsInstanced(GL_POINTS, gFaces[terrainId].size(), GL_UNSIGNED_INT, 0, vertexCount*vertexCount);
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

void display()
{
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	static float angle = 0;

	float angleRad = (float) (angle / 180.0) * M_PI;
	
	// Compute the modeling matrix

	//modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, -0.4f, -5.0f));
	//modelingMatrix = glm::rotate(modelingMatrix, angleRad, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 matT = glm::translate(glm::mat4(1.0), glm::vec3(-0.5f, -0.4f, -5.0f));   // same as above but more clear
    //glm::mat4 matR = glm::rotate(glm::mat4(1.0), angleRad, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 matRx = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
    glm::mat4 matRy = glm::rotate<float>(glm::mat4(1.0), (-90. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 matRz = glm::rotate<float>(glm::mat4(1.0), angleRad, glm::vec3(0.0, 0.0, 1.0));
    modelingMatrix = matRy * matRx;

    // Let's make some alternating roll rotation
    static float rollDeg = 0;
    static float changeRoll = 2.5;
    float rollRad = (float) (rollDeg / 180.f) * M_PI;
    //rollDeg += changeRoll;
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
    //pitchDeg += changePitch;
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
    drawModel(1);

	//angle += 0.5;
}

void setCamera()
{
    using namespace glm;

    const float carHeight = 0.4;
    vec3 pCar, p1, p2;
    pCar = eyePos - carHeight * cameraUp;
    pCar.y = perlinNoise(carPos) * noiseScale;

    p1 = pCar;
    p1.x += EPSILON;
    p1.y  = perlinNoise(p1) * noiseScale;

    p2 = pCar;
    p2.z += EPSILON;
    p2.y  = perlinNoise(p2) * noiseScale;

    float carYawInRads = (carYaw/180) * M_PI;
    carDir.x = cos(carYawInRads);
    carDir.y = 0.0f;
    carDir.z = sin(carYawInRads);

    vec3 up = normalize(cross(p2 - pCar, p1 - pCar));
    vec3 right = normalize(cross(cameraFront, up));
    vec3 gaze = normalize(cross(up, right));

    eyePos = pCar + up * carHeight;
    cameraFront = gaze;
    cameraUp = up;

    //cout << "up: "; printVec(up); cout << endl;
    //cout << "gaze: "; printVec(gaze); cout << endl;
    //cout << "right: "; printVec(right); cout << endl << endl;
    if(carSpeed > CAR_STOP_TRESHOLD)
    {
        eyePos += cameraFront //* dot(carDir, cameraFront)
                        * carSpeed * deltaTime;
    }

}
void setCar()
{
    using namespace glm;
    if(carSpeed > CAR_STOP_TRESHOLD)
    {
        carPos += carDir * dot(carDir, cameraFront)
                        * carSpeed * deltaTime;
    }

    vec3 pCar, p1, p2;
    pCar = carPos;
    pCar.y = perlinNoise(carPos) * noiseScale;

    p1 = carPos;
    p1.x += EPSILON;
    p1.y  = perlinNoise(p1) * noiseScale;

    p2 = carPos;
    p2.z += EPSILON;
    p2.y  = perlinNoise(p2) * noiseScale;

    float carYawInRads = (carYaw/180) * M_PI;
    carDir.x = cos(carYawInRads);
    carDir.y = 0.0f;
    carDir.z = sin(carYawInRads);

    vec3 up = normalize(cross(p2 - pCar, p1 - pCar));
    vec3 right = normalize(cross(carDir, up));
    vec3 gaze = normalize(cross(up, right));

    eyePos = pCar + vec3(0, 0.4, 0);
    cameraFront = gaze;
    cameraUp = up;

    //cout << "up: "; printVec(up); cout << endl;
    //cout << "gaze: "; printVec(gaze); cout << endl;
    //cout << "right: "; printVec(right); cout << endl << endl;
}



void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);


    //handle euler angles
    float yawInRads = (yaw/180) * M_PI;
    float pitchInRads = (pitch/180) * M_PI;

    //glm::vec3 cameraDir;
    //cameraDir.x = cos(yawInRads) * cos(pitchInRads);
    //cameraDir.y = sin(pitchInRads);
    //cameraDir.z = sin(yawInRads) * cos(pitchInRads);

    //cameraFront = glm::normalize(cameraDir);

	float fovyRad = (float) (cameraZoom / 180.0) * M_PI;
    float aspectRat = (float) 1.0f;

	projectionMatrix = glm::perspective(fovyRad, aspectRat, 0.01f, 100.0f);

    viewingMatrix = glm::lookAt(eyePos,
                                eyePos + cameraFront,
                                glm::normalize(cameraUp));
}
//void reshape(GLFWwindow* window, int w, int h)
//{
//    w = w < 1 ? 1 : w;
//    h = h < 1 ? 1 : h;
//
//    gWidth = w;
//    gHeight = h;
//
//    glViewport(0, 0, w, h);
//
//    //glMatrixMode(GL_PROJECTION);
//    //glLoadIdentity();
//    //glOrtho(-10, 10, -10, 10, -10, 10);
//    //gluPerspective(45, 1, 1, 100);
//
//	// Use perspective projection
//
//	float fovyRad = (float) (45.0 / 180.0) * M_PI;
//	projectionMatrix = glm::perspective(fovyRad, 1.0f, 1.0f, 100.0f);
//
//	// Assume default camera position and orientation (camera is at
//	// (0, 0, 0) with looking at -z direction and its up vector pointing
//	// at +y direction)
//
//	viewingMatrix = glm::mat4(1);
//
//    //glMatrixMode(GL_MODELVIEW);
//    //glLoadIdentity();
//}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        activeProgramIndex = 0;
        //std::cout << "active program 0" << std::endl;
    }
    else if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        //glShadeModel(GL_SMOOTH);
        activeProgramIndex = 1;
    }

    const float cameraAcceleration = 0.25f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        carSpeed += cameraAcceleration * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        carSpeed -= cameraAcceleration * deltaTime;
    }
    //TODO this should be changed to key L
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
    {
        //activeTerrainProgIndex = !activeTerrainProgIndex;
        if(wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            wireframe = 0;
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            wireframe = 1;
        }
    }


    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        terrainSpan += 1.f;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        if(terrainSpan > 1.f)
        {
            terrainSpan -= 1.f;
        }
    }
    
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        vertexCount += 100;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if(vertexCount > 100)
        {
            vertexCount -= 100;
        }
    }
    
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        noiseScale +=0.05;
        cout << "noiseScale: " << noiseScale << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        noiseScale -=0.05;
        cout << "noiseScale: " << noiseScale << endl;
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

    //yaw += xoffset;
    //pitch += yoffset;
    carYaw += xoffset;


    // make sure that when pitch is out of bounds, screen doesn't get flipped
    

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

}


void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window))
    {
        //handle time
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        currentTime = glfwGetTime();

        glClearColor(0, 0, 0, 1);
        glClearDepth(1.0f);
        glClearStencil(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


        drawTerrain(0);
        //display();
        glfwSwapBuffers(window);
        glfwPollEvents();

        setCar();
        //setCamera();
        reshape(window, width, height);

        updateUniforms();
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
  //std::cout << "AN ERROR OCCURRED" << std::endl;
}


int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
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

    //TODO disable debug 
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);//TODO remove for the final program

    int width = 640, height = 480;
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
    glfwSetCursorPosCallback(window, handleMouse);
    glfwSetWindowSizeCallback(window, reshape);

    //TODO disable debug 
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallbackARB(messageCallBack, 0);

    reshape(window, width, height); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

glm::vec3 gradients[16] = {
    glm::vec3(1, 1, 0),
    glm::vec3(-1, 1, 0),
    glm::vec3(1, -1, 0),
    glm::vec3(-1, -1, 0),
    glm::vec3(1, 0, 1),
    glm::vec3(-1, 0, 1),
    glm::vec3(1, 0, -1),
    glm::vec3(-1, 0, -1),
    glm::vec3(0, 1, 1),
    glm::vec3(0, -1, 1),
    glm::vec3(0, 1, -1),
    glm::vec3(0, -1, -1),
    glm::vec3(1, 1, 0),
    glm::vec3(-1, 1, 0),
    glm::vec3(0, -1, 1),
    glm::vec3(0, -1, -1)
};

int table[16] = {
    14, 8, 9, 7, 5, 13, 4, 0, 12, 2, 3, 11, 6, 15, 10, 1
};

GLfloat fade(GLfloat t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

glm::vec3 grad(int i, int j, int k)
{
    if(i < 0) i = -i;
    if(j < 0) j = -j;
    if(k < 0) k = -k;
    int ind;
    ind = table[i % 16];
    ind = table[(j + ind) % 16];
    ind = table[(k + ind) % 16];
    
    return gradients[ind];
}


float perlinNoise(glm::vec3 texCoords)
{
    using glm::mix, glm::dot, glm::floor;

    int i = int(floor(texCoords.x)) & 255,
        j = int(floor(texCoords.y)) & 255,
        k = int(floor(texCoords.z)) & 255;

    float x = texCoords.x - floor(texCoords.x),
          y = texCoords.y - floor(texCoords.y),
          z = texCoords.z - floor(texCoords.z);
    
    float u = fade(x),
          v = fade(y),
          w = fade(z);

    glm::vec3 g000 = grad(i  , j  , k  ),
                     g100 = grad(i+1, j  , k  ),
                     g110 = grad(i+1, j+1, k  ),
                     g010 = grad(i  , j+1, k  ),
                     g011 = grad(i  , j+1, k+1),
                     g111 = grad(i+1, j+1, k+1),
                     g101 = grad(i+1, j  , k+1),
                     g001 = grad(i  , j  , k+1);

    glm::vec3 p000 = glm::vec3(x  , y  , z  ),
                     p100 = glm::vec3(x-1, y  , z  ), 
                     p110 = glm::vec3(x-1, y-1, z  ), 
                     p010 = glm::vec3(x  , y-1, z  ),
                     p011 = glm::vec3(x  , y-1, z-1), 
                     p111 = glm::vec3(x-1, y-1, z-1), 
                     p101 = glm::vec3(x-1, y  , z-1),
                     p001 = glm::vec3(x  , y  , z-1); 


    GLfloat c; 
    c = mix(mix(mix(dot(g000, p000), dot(g100, p100), u),
                mix(dot(g010, p010), dot(g110, p110), u), v),

            mix(mix(dot(g001, p001), dot(g101, p101), u),
                mix(dot(g011, p011), dot(g111, p111), u), v), w);



    return (c+1)/2.0f;
}

