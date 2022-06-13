#version 460 core

layout( points ) in;
layout(line_strip, max_vertices = 6) out;

layout (std140, binding = 0) uniform matrices
{
    mat4 modelingMatrix;
    mat4 viewingMatrix;
    mat4 projectionMatrix;
    float terrainSpan;
    uint vertexCount;
};

in VS_GS_INTERFACE
{
    vec4 pointWorldCoord;
} gs_in[];

out vec4 fragWorldPos;
out vec3 fragWorldNor;


int coords[6][2] = {
    {0, 0},
    {1, 0},
    {1, 1},
    {0, 0},
    {0, 1},
    {1, 1}
};

vec3 gradients[16] = {
    vec3(1, 1, 0),
    vec3(-1, 1, 0),
    vec3(1, -1, 0),
    vec3(-1, -1, 0),
    vec3(1, 0, 1),
    vec3(-1, 0, 1),
    vec3(1, 0, -1),
    vec3(-1, 0, -1),
    vec3(0, 1, 1),
    vec3(0, -1, 1),
    vec3(0, 1, -1),
    vec3(0, -1, -1),
    vec3(1, 1, 0),
    vec3(-1, 1, 0),
    vec3(0, -1, 1),
    vec3(0, -1, -1)
};

int table[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

float perlinNoise(vec2);

void main(void)
{
    //uint row, column;
    //column = gl_InstanceID % vertexCount;
    //row = gl_InstanceID / vertexCount;

    float cellSize = terrainSpan * 2 / vertexCount;
    //cellSize = 60.f/vertexCount;
    //cellSize = terrainSpan/1000;

    int i, j, k;
    //for(i=0; i < gl_in.length(); i++)
    fragWorldNor = vec3(0.0f, 1.f, 0.0f);
    for(k = 0; k < 6; k++)
    {
        i = coords[k][0];
        j = coords[k][1];
        fragWorldPos = gs_in[0].pointWorldCoord
            + i * vec4(cellSize, 0, 0, 0)
            + j * vec4(0, 0, cellSize, 0)
            + vec4(0,1,0,0)* perlinNoise(
                    vec2(gs_in[0].pointWorldCoord.xz));

        gl_Position = projectionMatrix * viewingMatrix * fragWorldPos;
        EmitVertex();
    }
            //fragWorldPos = gs_in[0].pointWorldCoord
            //                + 0 * vec4(cellSize, 0, 0, 0)
            //                + 1 * vec4(0, 0, cellSize, 0);

            //gl_Position = projectionMatrix * viewingMatrix * fragWorldPos;
            //EmitVertex();

            //fragWorldPos = gs_in[0].pointWorldCoord
            //                + 0 * vec4(cellSize, 0, 0, 0)
            //                + 0 * vec4(0, 0, cellSize, 0);

            //gl_Position = projectionMatrix * viewingMatrix * fragWorldPos;
            //EmitVertex();

            //fragWorldPos = gs_in[0].pointWorldCoord
            //                + 1 * vec4(cellSize, 0, 0, 0)
            //                + 0 * vec4(0, 0, cellSize, 0);

            //gl_Position = projectionMatrix * viewingMatrix * fragWorldPos;
            //EmitVertex();

            //fragWorldPos = gs_in[0].pointWorldCoord
            //                + 0 * vec4(cellSize, 0, 0, 0)
            //                + 1 * vec4(0, 0, cellSize, 0);

            //gl_Position = projectionMatrix * viewingMatrix * fragWorldPos;
            //EmitVertex();

            //fragWorldPos = gs_in[0].pointWorldCoord
            //                + 1 * vec4(cellSize, 0, 0, 0)
            //                + 1 * vec4(0, 0, cellSize, 0);

            //gl_Position = projectionMatrix * viewingMatrix * fragWorldPos;
            //EmitVertex();

            //fragWorldPos = gs_in[0].pointWorldCoord
            //                + 1 * vec4(cellSize, 0, 0, 0)
            //                + 0 * vec4(0, 0, cellSize, 0);

            //gl_Position = projectionMatrix * viewingMatrix * fragWorldPos;
            //EmitVertex();

   
    EndPrimitive();

}

float fade(float t)
{
    t = abs(t);
    if (t < 1)
        return  t * t * t * (t * (t * -6 + 15) - 10) + 1;
    else
        return 0;
}

vec2 getGradient(int i, int j)
{
    int idx;
    idx = table[abs(j) % 16];
    idx = table[abs(i + idx) % 16];
    return gradients[idx].xy;
}

float perlinNoise(vec2 coord)
{
    int x = int(floor(coord.x));
    int y = int(floor(coord.y));

    float dx = coord.x - x;
    float dy = coord.y - y;

    float u = fade(dx);
    float v = fade(dy);

    float BL = fade(dx) * fade(dy) * dot(getGradient(x  , y  ), vec2(dx  , dy  )),
          BR = fade(dx-1) * fade(dy) * dot(getGradient(x+1, y  ), vec2(dx-1, dy  )),
          TL = fade(dx) * fade(dy-1) * dot(getGradient(x  , y+1), vec2(dx  , dy-1)),
          TR = fade(dx-1) * fade(dy-1) * dot(getGradient(x+1, y+1), vec2(dx-1, dy-1));

    //float c = mix(mix(BL, TL, u), 
    //              mix(BR, TR, u),
    //                          v);
    float c = BL + BR + TL +TR;

    return (c+1)/2;
}
