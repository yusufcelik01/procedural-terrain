#version 460 core

layout(location=0) in vec3 inPoint;
layout(location=1) in vec3 inNormal;//actually not gonna use this

//uniform mat4 modelingMatrix;
//uniform mat4 viewingMatrix;
//uniform mat4 projectionMatrix;

layout (std140, binding = 0) uniform matrices
{
    mat4 modelingMatrix;
    mat4 viewingMatrix;
    mat4 projectionMatrix;
    float terrainSpan;
    uint vertexCount;
};

//layout (std140, binding = 0) uniform matrices
//{
//    mat4 modelingMatrix;
//    mat4 viewingMatrix;
//    mat4 projectionMatrix;
//};
//
//layout (std140, binding = 1) uniform terrainData
//{
//    float terrainSpan;
//    uint vertexCount;
//};


//int vertexCount = 1000; 
//float terrainSpan = 30;

//uniform int vertexCount;
//uniform float terrainSpan;

out VS_GS_INTERFACE
{
    vec4 pointWorldCoord;
} vs_out;

//TODO make these uniform

void main(void)
{
    uint row, column;
    column = gl_InstanceID % vertexCount;
    row = gl_InstanceID / vertexCount;


    float cellSize = terrainSpan * 2 / vertexCount;
    //cellSize = 60.f/vertexCount;
    //cellSize = terrainSpan/1000;
    //column = gl_InstanceID % 1000;
    //row = gl_InstanceID / 1000;
    

    vs_out.pointWorldCoord = vec4( -terrainSpan + column * cellSize,
                                   0,
                                   -terrainSpan + row * cellSize,
                                   1.0f );


    gl_Position = projectionMatrix * viewingMatrix * vs_out.pointWorldCoord;
}

