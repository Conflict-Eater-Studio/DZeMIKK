#version 460 core

layout(location = 0) in vec3 aPos;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform mat4 model;
uniform vec3 viewPos;

out vec3 vLocalPos;
out vec3 vLocalCamPos;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    
    // The cube's vertices are defined within [-0.5, 0.5] in local space
    vLocalPos = aPos;
    
    // Transform camera position to local space of the cube
    mat4 invModel = inverse(model);
    vLocalCamPos = vec3(invModel * vec4(viewPos, 1.0));
    
    gl_Position = projection * view * worldPos;
}
