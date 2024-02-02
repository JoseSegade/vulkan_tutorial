#version 450

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform constants {
  mat4 model;
} ObjectData;

vec2 positions[3] = vec2[](
  vec2( 0.0f, -0.5f),
  vec2( 0.5f,  0.5f),
  vec2(-0.5f,  0.5f)
);

vec3 colors[3] = vec3[](
  vec3(1.0f, 0.0f, 0.0f),
  vec3(0.0f, 1.0f, 0.0f),
  vec3(0.0f, 0.0f, 1.0f)
);

void main() {
  gl_Position = ObjectData.model * vec4(positions[gl_VertexIndex], 0.0f, 1.0f);
  fragColor = colors[gl_VertexIndex];
}
