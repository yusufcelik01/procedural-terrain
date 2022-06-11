#version 460 core

//layout( triangles ) in;
layout( points ) in;
layout(triangle_strip, max_vertices = 3) out;

in vec4 vertexColor[];
out vec4 color;

void main(void)
{
    int i;
    //for(i=0; i < gl_in.length(); i++)
    for(i=0; i < 3; i++)
    {
        gl_Position = gl_in[i].gl_Position;
        color = vertexColor[i];
        EmitVertex();
    }
    EndPrimitive();

}

