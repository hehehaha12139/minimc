#version 150

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_LightVec;
in vec4 fs_Col;
in vec4 fs_Nor;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

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
        vec4 diffuseColor = fs_Col;

        // Calculate the diffuse term for Lambert shading
        float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
        // Avoid negative lighting values
        diffuseTerm = clamp(diffuseTerm, 0, 1);

        float ambientTerm = 0.2;

        float shadow = shadowCalculation(fs_in.FragPosLightSpace);
        float lightIntensity = ambientTerm + (1.0 - shadow) * diffuseTerm;

        // Compute final shaded color
        out_Col = vec4(diffuseColor.rgb * lightIntensity, diffuseColor.a);
        //out_Col = vec4(vec3(shadow), 1.0f);
}
