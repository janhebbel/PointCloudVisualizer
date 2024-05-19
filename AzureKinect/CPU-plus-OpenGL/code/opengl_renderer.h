#ifndef OPENGL_RENDERER_H
#define OPENGL_RENDERER_H

#include "linalg.h"

typedef struct
{
    float xyz[3];
    float rgb[3];
} color_point;

typedef struct
{
    mat4 model;
    v3f position;
    v3f forward;
    v3f up;
    float fov;
    float speed;
    float sensitivity;
} view_control;

#endif
