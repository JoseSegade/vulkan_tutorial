#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
  mat4 view;
  mat4 projection;
  mat4 viewProjection;
} cameraData;

layout(std140, set = 0, binding = 1) readonly buffer storageBuffer {
  mat4 model[];
} objectData;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexTexCoord;
layout(location = 3) in vec3 vertexNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;

void main() {
  gl_Position = cameraData.viewProjection * 
    objectData.model[gl_InstanceIndex] * vec4(vertexPosition, 1.0f);
  fragColor = vertexColor;
  fragTexCoord = vertexTexCoord;
  fragNormal = normalize(
    objectData.model[gl_InstanceIndex] * vec4(vertexNormal, 0.0f)).xyz;
}
