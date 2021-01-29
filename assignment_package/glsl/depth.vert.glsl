#version 150 core

in vec4 vs_Pos;

uniform mat4 u_LightMatrix;
uniform mat4 u_Model;

void main()
{
    gl_Position = u_LightMatrix * u_Model * vs_Pos;
}
