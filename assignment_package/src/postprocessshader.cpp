#include "postprocessshader.h"
#include <QDateTime>

//PostProcessShader::PostProcessShader(OpenGLContext *context)
//    : ShaderProgram(context),
//      attrPos(-1), attrUV(-1),
//      unifDimensions(-1), unifSampler2D(-1), unifTime(-1)
//{}

//PostProcessShader::~PostProcessShader()
//{}

//void PostProcessShader::setupMemberVars()
//{
//    attrPos = context->glGetAttribLocation(prog, "vs_Pos");
//    attrUV  = context->glGetAttribLocation(prog, "vs_UV");

//    unifTime = context->glGetUniformLocation(prog, "u_Time");
//    unifSampler2D = context->glGetUniformLocation(prog, "u_RenderedTexture");
//    unifDimensions = context->glGetUniformLocation(prog, "u_Dimensions");
//}

//void PostProcessShader::draw(Drawable& d)
//{
//    useMe();

//    // Set our "renderedTexture" sampler to user Texture Unit 0
//    context->glUniform1i(unifSampler2D, 0);

//    // Each of the following blocks checks that:
//    //   * This shader has this attribute, and
//    //   * This Drawable has a vertex buffer for this attribute.
//    // If so, it binds the appropriate buffers to each attribute.


//    // Bind the index buffer and then draw shapes from it.
//    // This invokes the shader program, which accesses the vertex buffers.
//    d.bindIdx();
//    context->glDrawElements(d.drawMode(), d.elemCount(), GL_UNSIGNED_INT, 0);

//    context->printGLErrorLog();
//}


//void PostProcessShader::setDimensions(glm::ivec2 dims)
//{
//    useMe();

//    if(unifDimensions != -1)
//    {
//        context->glUniform2i(unifDimensions, dims.x, dims.y);
//    }
//}

//void PostProcessShader::setTime(int t)
//{
//    useMe();

//    if(unifTime != -1)
//    {
//        context->glUniform1i(unifTime, t);
//    }
//}
