#version 330 core
layout (lines) in;
layout (line_strip, max_vertices = 2) out;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform vec4 light_position;

in vec4 vs_light_direction[];

out vec4 normal;
out vec4 light_direction;

void main() {
  int n = 0;
  normal = vec4(0.0, 0.0, 1.0f, 0.0);
  for (n = 0; n < gl_in.length(); n++) {
    light_direction = normalize(vs_light_direction[n]);
    gl_Position = projection * view * model * gl_in[n].gl_Position;
    EmitVertex();
  }
  EndPrimitive();
}