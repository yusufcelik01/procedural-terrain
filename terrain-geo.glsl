#version 460 core

layout( points ) in;
layout(triangle_strip, max_vertices = 4) out;

//uniform mat4 modelingMatrix;
//uniform mat4 viewingMatrix;
//uniform mat4 projectionMatrix;


layout (std140, binding = 0) uniform matrices
{
    mat4 modelingMatrix;
    mat4 viewingMatrix;
    mat4 projectionMatrix;
    //int vertexCount;
    //float terrainSpan;
};

in VS_GS_INTERFACE
{
    vec4 pointWorldCoord;
} gs_in[];

out vec4 fragWorldPos;
out vec3 fragWorldNor;

//TODO make these uniform
int vertexCount = 1000;
float terrainSpan = 30.0f;

void main(void)
{
    //uint row, column;
    //column = gl_InstanceID % vertexCount;
    //row = gl_InstanceID / vertexCount;

    float cellSize = terrainSpan * 2 / vertexCount;

    int i, j;
    //for(i=0; i < gl_in.length(); i++)
    fragWorldNor = vec3(0.0f, 1.f, 0.0f);
    for(j = 0; j < 2; j++)
    {
        for(i = 0; i < 2; i++)
        {
            fragWorldPos = gs_in[0].pointWorldCoord
                            + i * vec4(cellSize, 0, 0, 0)
                            + j * vec4(0, 0, cellSize, 0);

            gl_Position = projectionMatrix * viewingMatrix * fragWorldPos;
            EmitVertex();
        }
    }
   
    EndPrimitive();

}

