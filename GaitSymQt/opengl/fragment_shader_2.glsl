#version 330 core

// this is a fixed colour shader

out highp vec4 fragColor;

in highp vec4 color;

void main()
{
    fragColor = color;
}

