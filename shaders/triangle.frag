#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform vec3 triColor;

void main()
{    
    color = vec4(triColor, 1.0) * texture(image, TexCoords);
} 