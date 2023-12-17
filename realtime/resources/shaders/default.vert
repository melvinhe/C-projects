#version 330 core

layout(location = 0) in vec3 inObjectSpacePosition;
layout(location = 1) in vec3 inObjectSpaceNormal;

out vec3 fragWorldPos;
out vec3 fragWorldNormal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {

    vec4 worldPosition = model * vec4(inObjectSpacePosition, 1.0);

    fragWorldPos = worldPosition.xyz;
    fragWorldNormal = normalize(mat3(transpose(inverse(model))) * inObjectSpaceNormal);

    gl_Position = projection * view * worldPosition;

}
