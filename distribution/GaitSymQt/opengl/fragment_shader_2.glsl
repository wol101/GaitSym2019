#version 330

// this is a fixed colour shader

in vec4 color;
out vec4 fragColor;

void main()
{
    fragColor = color;
}

