#version 150
out vec4 out_Col;
void main()
{
    gl_FragDepth = gl_FragCoord.z;
}
