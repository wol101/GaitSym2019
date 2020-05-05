#version 330

// this is a fixed colour shader

uniform mat4 mvpMatrix;

in vec4 vertex;
in vec4 vertexColor;

out vec4 color;

void main ()
{
    color = vertexColor;

    gl_Position = mvpMatrix * vertex;
}

