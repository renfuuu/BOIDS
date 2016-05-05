#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec3 meshColor;

in vec4 face_normal;
in vec4 light_direction;
in vec4 world_position;


void main()
{    
	float dot_nl = dot(normalize(light_direction), normalize(face_normal));
	dot_nl = clamp(dot_nl, 0.0, 1.0);
    color = clamp(dot_nl * vec4(meshColor, 1.0), 0.0, 1.0);
} 