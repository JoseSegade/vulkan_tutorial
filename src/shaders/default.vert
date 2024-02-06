#version 450

layout(binding = 0) uniform UniformBufferObject {
  mat4 view;
  mat4 projection;
  mat4 viewProjection;
} cameraData;

layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec3 vertexColor;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform constants {
  mat4 model;
} ObjectData;

void main() {
  gl_Position = cameraData.viewProjection * 
    ObjectData.model * vec4(vertexPosition, 0.0f, 1.0f);
  fragColor = vertexColor;
}
