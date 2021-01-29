#version 150
// ^ Change this to version 130 if you have compatibility issues

//This is a vertex shader. While it is called a "shader" due to outdated conventions, this file
//is used to apply matrix transformations to the arrays of vertex data passed to it.
//Since this code is run on your GPU, each vertex is transformed simultaneously.
//If it were run on your CPU, each vertex would have to be processed in a FOR loop, one at a time.
//This simultaneous transformation allows your program to run much faster, especially when rendering
//geometry with millions of vertices.

uniform mat4 u_Model;       // The matrix that defines the transformation of the
                            // object we're rendering. In this assignment,
                            // this will be the result of traversing your scene graph.

uniform mat4 u_ModelInvTr;  // The inverse transpose of the model matrix.
                            // This allows us to transform the object's normals properly
                            // if the object has been non-uniformly scaled.

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself

uniform vec4 u_Color;       // When drawing the cube instance, we'll set our uniform color to represent different block types.

in vec4 vs_Pos;             // The array of vertex positions passed to the shader

in vec4 vs_Nor;             // The array of vertex normals passed to the shader

in vec4 vs_Col;             // The array of vertex colors passed to the shader.

out vec4 fs_Nor;            // The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
out vec4 fs_LightVec;       // The direction in which our virtual light lies, relative to each vertex. This is implicitly passed to the fragment shader.

out vec4 fs_ViewVec;

uniform int u_Time;

out vec4 fs_Col;            // The color of each vertex. This is implicitly passed to the fragment shader.

in vec2 vs_UV;              // The array of vertex texture coordinates passed to the shader
out vec2 fs_UV;             // The UV of each vertex. This is implicitly passed to the fragment shader.

const vec4 lightDir = vec4(0,1,0,0);  // The direction of our virtual light, which is used to compute the shading of
                                        // the geometry in the fragment shader.

uniform mat4 u_LightMatrix;

uniform vec3 u_LightPos;

out VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec4 FragPosLightSpace;
} vs_out;


in float vs_IsAnimated;
in float vs_BlinnPower;
out float fs_IsAnimated;
out float fs_BlinnPower;


void main()
{
    fs_UV = vs_UV;    // Pass the vertex UVs to the fragment shader for interpolation
    fs_Col = vs_Col;                         // Pass the vertex colors to the fragment shader for interpolation
    fs_BlinnPower = vs_BlinnPower;
    fs_IsAnimated = vs_IsAnimated;
    mat3 invTranspose = mat3(u_ModelInvTr);
    fs_Nor = vec4(invTranspose * vec3(vs_Nor), 0);          // Pass the vertex normals to the fragment shader for interpolation.
                                                            // Transform the geometry's normals by the inverse transpose of the
                                                            // model matrix. This is necessary to ensure the normals remain
                                                            // perpendicular to the surface after the surface is transformed by
                                                            // the model matrix.


    vec4 modelposition = u_Model * vs_Pos;   // Temporarily store the transformed vertex positions for use below
    fs_LightVec = vec4(normalize(u_LightPos - vec3(vs_Pos)), 1.0);  // Compute the direction in which the light source lies

    gl_Position = u_ViewProj * modelposition;// gl_Position is a built-in variable of OpenGL which is
                                             // used to render the final positions of the geometry's vertices

    vs_out.FragPos = vec3(modelposition);
    vs_out.FragPosLightSpace = u_LightMatrix * modelposition;
    vs_out.Normal = mat3(u_ModelInvTr) * vec3(vs_Nor);
}
