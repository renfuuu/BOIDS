#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec3 meshColor;

void main()
{    
    color = vec4(meshColor, 1.0);
} 