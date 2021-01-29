#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform vec4 u_Color; // The color with which to render this instance of geometry.

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Nor;
in vec4 fs_LightVec;
in vec4 fs_Col;
uniform int u_Time;
uniform vec3 u_ViewVec;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

uniform sampler2D u_Texture; // The texture to be read from by this shader
in vec2 fs_UV;
in float fs_IsAnimated;
in float fs_BlinnPower;

in VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec4 FragPosLightSpace;
} fs_in;

uniform sampler2D u_ShadowMap; // Shadow mapping texture

float shadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    // Sample the shadow mapping texture
    float closestDepth = texture(u_ShadowMap, projCoords.xy).r;

    // Shadow bias
    vec3 lightDir = vec3(normalize(fs_LightVec));
    float bias = max(0.05 * (1.0 - dot(fs_in.Normal, lightDir)), 0.05);

    // Compare frag depth and shadow depth
    float currentDepth = projCoords.z;

    float shadow = 0.0f;
    if(currentDepth > 1.0)
    {
        shadow = 0.0;
    }
    else
    {
        // PCF
        vec2 texelSize = 0.25f / textureSize(u_ShadowMap, 0);
        for(int x = -8; x <= 8; ++x)
        {
            for(int y = -8; y <= 8; ++y)
            {
                float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth ? 1.0:0.0;
            }
        }
        shadow /= 289.0f;
    }

    //shadow = texture(u_ShadowMap, projCoords.xy).r;
    //shadow = texture(u_ShadowMap, projCoords.xy).r;
    return shadow;
}


void main()
{
    // Material base color (before shading)
    vec2 UV = fs_UV;
    if (fs_IsAnimated > 0.f)
    {
        UV.x = UV.x + (float(u_Time % 200) / 200.f) * 0.0625f;
        while (UV.x < fs_UV.x)
        {
            UV.x = UV.x + 0.0625f;
        }

        UV.y = UV.y + (float(u_Time % 200) / 200.f) * 0.0625f;
        while (UV.y < fs_UV.y)
        {
            UV.y = UV.y + 0.0625f;
        }
    }

    vec4 diffuseColor = texture(u_Texture, UV);

    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);

    // Calculate the specular intensity of Blinn-Phong shading
    int exp = int(fs_BlinnPower);
    vec3 halfVec = vec3(fs_LightVec) + u_ViewVec;
    float specularIntensity = max(pow(dot(normalize(halfVec), normalize(vec3(fs_Nor))), exp), 0);

    float specularTerm = specularIntensity * exp;
    specularTerm = clamp(specularTerm, 0, 1);

    float ambientTerm = 0.2;

    float shadow = shadowCalculation(fs_in.FragPosLightSpace);



    float lightIntensity = ambientTerm + (1.0 - shadow)*(diffuseTerm + specularTerm);   //Add a small float value to the color multiplier
                                                        //to simulate ambient lighting. This ensures that faces that are not
                                                        //lit by our point light are not completely black.

    // Compute final shaded color
    out_Col = vec4(diffuseColor.rgb * lightIntensity, diffuseColor.a);
}
