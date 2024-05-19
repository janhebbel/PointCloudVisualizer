#ifndef TYPES_H
#define TYPES_H

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

#if defined(_WIN32)
typedef struct
{
	HGLRC GLContext;
	HDC DeviceContext;
} os_specifics;
#elif defined(__linux__)
typedef struct
{
	GLXContext GLContext;
	Display *X11Display;
} os_specifics;
#endif

#endif
