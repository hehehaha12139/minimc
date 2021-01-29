#version 150

uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_ModelInvTr;  // The inverse transpose of the model matrix.
                            // This allows us to transform the object's normals properly
                            // if the object has been non-uniformly scaled.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself

uniform mat4 u_LightMatrix;

uniform vec3 u_LightPos;

in vec4 vs_Pos;             // The array of vertex positions passed to the shader

in vec4 vs_Nor;             // The array of vertex normals passed to the shader



in vec4 vs_Col;             // The array of vertex colors passed to the shader.

out vec4 fs_Col;
out vec4 fs_LightVec;       // The direction in which our virtual light lies, relative to each vertex. This is implicitly passed to the fragment shader.
out vec4 fs_Nor;

out VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec4 FragPosLightSpace;
} vs_out;

void main()
{
    fs_Col = vs_Col;
    fs_LightVec =  vec4(normalize(u_LightPos - vec3(vs_Pos)), 1.0);

    vec4 modelposition = u_Model * vs_Pos;   // Temporarily store the transformed vertex positions for use below
    gl_Position = u_ViewProj * modelposition;// gl_Position is a built-in variable of OpenGL which is
                                             // used to render the final positions of the geometry's vertices
    vs_out.FragPos = vec3(modelposition);
    vs_out.FragPosLightSpace = u_LightMatrix * modelposition;
    vs_out.Normal = mat3(u_ModelInvTr) * vec3(vs_Nor);
    mat3 invTranspose = mat3(u_ModelInvTr);
    fs_Nor = vec4(invTranspose * vec3(vs_Nor), 0);
}
