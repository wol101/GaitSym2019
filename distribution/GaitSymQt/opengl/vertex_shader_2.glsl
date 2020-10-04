#version 330 core

// this is a fixed colour shader

uniform highp mat4 mvpMatrix;

in highp vec4 vertex;
in highp vec4 vertexColor;

out highp vec4 color;

void main ()
{
    color = vertexColor;

    gl_Position = mvpMatrix * vertex;
}

