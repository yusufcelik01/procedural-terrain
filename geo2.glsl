#version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;

in vec4 vertWorldPos[];
in vec3 vertWorldNor[];

out vec4 fragWorldPos;
out vec3 fragWorldNor;

float shrinkFactor = 0.2;

void main(void)
{
    int i;
    float one_min_fs = 1.0f - shrinkFactor;
    vec4 center  = (vertWorldPos[0]+
                    vertWorldPos[1]+
                    vertWorldPos[2] )/3;

    
    vec3 inVertex;
    for(i = 0; i < gl_in.length(); i++)
    {
        inVertex = mix(vertWorldPos[i], center, shrinkFactor).xyz;
        gl_Position = projectionMatrix * viewingMatrix * vec4(inVertex, 1);
        fragWorldPos = vertWorldPos[i];
        fragWorldNor = vertWorldNor[i];

        EmitVertex();
    }


}

