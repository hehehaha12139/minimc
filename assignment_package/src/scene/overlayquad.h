#ifndef OVERLAYQUAD_H
#define OVERLAYQUAD
#include "drawable.h"

class OverlayQuad : public Drawable
{
    char type;

public:
    OverlayQuad(OpenGLContext* context, char t = 'b');
    // Creates VBO data to make a visual representation of the currently selected Vertex
    void create() override;
};

#endif // OVERLAYQUAD_H
