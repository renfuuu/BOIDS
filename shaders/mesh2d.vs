#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;

uniform vec4 light_position;
out vec4 vs_light_direction;

void main()
{
    TexCoords = vertex.zw;
    gl_Position = vec4(vertex.xyz, 1.0);
    vs_light_direction = light_position - gl_Position;
}