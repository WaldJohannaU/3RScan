#version 330 core

flat in vec3 colorV;
out vec4 color;

void main( )
{
    color = vec4(colorV, 1.0);
}