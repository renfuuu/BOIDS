#version 330 core

uniform vec3 meshColor;

in vec4 normal;
in vec4 light_direction;

out vec4 fragment_color;

void main() {
    float dot_nl = dot(normalize(light_direction), normalize(normal));
    dot_nl = clamp(dot_nl, 0.0, 1.0);
    fragment_color = clamp(dot_nl * vec4(meshColor, 1.0), 0.0, 1.0);
}