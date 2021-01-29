#version 150
// noOp.frag.glsl:
// A fragment shader used for post-processing that simply reads the
// image produced in the first render pass by the surface shader
// and outputs it to the frame buffer

in vec2 fs_UV;

out vec3 color;

uniform sampler2D u_RenderedTexture;

void main()
{
    if (fs_UV.x > 0.499 && fs_UV.x < 0.501 && fs_UV.y > 0.48 && fs_UV.y < 0.52)
        color = vec3(1);
    else if (fs_UV.y > 0.4985 && fs_UV.y < 0.5015 && fs_UV.x > 0.485 && fs_UV.x < 0.515)
        color = vec3(1);
    else
        color = texture(u_RenderedTexture, fs_UV).rgb;
}
