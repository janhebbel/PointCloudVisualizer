#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>

#if defined(_WIN32)
	#define GLFW_EXPOSE_NATIVE_WIN32
	#define GLFW_EXPOSE_NATIVE_WGL
#elif defined(__linux__)
	#define GLFW_EXPOSE_NATIVE_X11
	#define GLFW_EXPOSE_NATIVE_GLX
#else
	#error Using unsupported operating system.
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

typedef struct {
	const int CountTo;
	char *Msg;
	char *Unit;
	int Count;
	double Acc;
} average;

static void PrintAverage(average *Average, double Value) {
    if(Average->Count == Average->CountTo)
    {
        double Avg = Average->Acc / (double)Average->CountTo;
        printf("%s: %f %s\n", Average->Msg, Avg, Average->Unit);
        Average->Acc = 0;
        Average->Count = 0;
    }
    else
    {
        Average->Acc += Value;
        Average->Count++;
    }
}

#include "linalg.h"
#include "types.h"
#include "k4a.c"
#include "opengl.c"
#include "opencl.c"
#include "opencl_opengl.c"

struct scroll_update { 
    double yoffset;
    int    updated;
};

static struct scroll_update global_scroll_update;

void handle_input(GLFWwindow *Window, view_control *control, float delta_time)
{
    //
    // mouse input
    double xpos, ypos;
    static double last_xpos, last_ypos;
    
    glfwGetCursorPos(Window, &xpos, &ypos);
    
    float dx = (float)(xpos - last_xpos) * control->sensitivity;
    float dy = -(float)(ypos - last_ypos) * control->sensitivity;

    // first person camera controller
    if(GLFW_PRESS == glfwGetMouseButton(Window, GLFW_MOUSE_BUTTON_RIGHT))
    {
        static float yaw = -0.25f;
        static float pitch = 0.0f;

        yaw += dx;
        pitch += dy;

        if(pitch > 0.245f) pitch = 0.245f;
        else if(pitch < -0.245f) pitch = -0.245f;

        control->forward.x = linalg_cos(yaw) * linalg_cos(pitch);
        control->forward.y = linalg_sin(pitch);
        control->forward.z = linalg_sin(yaw) * linalg_cos(pitch);
        control->forward = v3f_normalize(control->forward);

        int state;
        v3f add = {0};
        
        state = glfwGetKey(Window, GLFW_KEY_W);
        if(state == GLFW_PRESS)
        {
            add = v3f_add(add, control->forward);
        }
        state = glfwGetKey(Window, GLFW_KEY_A);
        if(state == GLFW_PRESS)
        {
            add = v3f_sub(add, v3f_normalize(v3f_cross(control->forward, control->up)));
        }
        state = glfwGetKey(Window, GLFW_KEY_S);
        if(state == GLFW_PRESS)
        {
            add = v3f_sub(add, control->forward);
        }
        state = glfwGetKey(Window, GLFW_KEY_D);
        if(state == GLFW_PRESS)
        {
            add = v3f_add(add, v3f_normalize(v3f_cross(control->forward, control->up)));
        }

        control->position = v3f_add(control->position, v3f_scale(add, control->speed * delta_time));
    }
    
    last_xpos = xpos;
    last_ypos = ypos;

    if(global_scroll_update.updated)
    {
        float dfov = (float)global_scroll_update.yoffset / 100.0f;
        float new_fov = control->fov + dfov;
        if(new_fov > 0.0f && new_fov < 0.4f)
        {
            control->fov = new_fov;
        }
        
        global_scroll_update.updated = 0;
    }
}

void mouse_button_callback(GLFWwindow* Window, int button, int action, int mods)
{
    if((button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_PRESS)
    {
        glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else if((button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_RELEASE)
    {
        glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void scroll_callback(GLFWwindow* Window, double xoffset, double yoffset)
{
    global_scroll_update.yoffset = -yoffset;
    global_scroll_update.updated = 1;
}

void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main(void)
{
	int ExitCode = 0;
	
	if(glfwInit())
	{
		glfwSetErrorCallback(glfw_error_callback);
		
		//glfwWindowHint(GLFW_RESIZABLE, false);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		uint32_t WindowWidth = 1280;
		uint32_t WindowHeight = 720;
        GLFWwindow *Window = glfwCreateWindow(WindowWidth, WindowHeight, "Point Cloud Visualizer", NULL, NULL);
		if(Window)
		{
			glfwMakeContextCurrent(Window);
			
			glfwSwapInterval(0);
			
			glfwSetMouseButtonCallback(Window, mouse_button_callback);
			glfwSetScrollCallback(Window, scroll_callback);
			
			camera_config config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
            config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
            config.camera_fps = K4A_FRAMES_PER_SECOND_30;
            config.synchronized_images_only = false;

            tof_camera camera_ = camera_init(&config);
            tof_camera *Camera = &camera_;
			
			if(Camera->device)
			{
				uint32_t DepthMapWidth = Camera->max_capture_width;
				int DepthMapHeight = Camera->max_capture_height;
                int DepthMapCount = DepthMapWidth * DepthMapHeight;
				
				k4a_calibration_t calibration;
                k4a_device_get_calibration(Camera->device, config.depth_mode, config.color_resolution, &calibration);
                
                k4a_image_t xy_image = NULL;
                k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM,
                                 calibration.depth_camera_calibration.resolution_width,
                                 calibration.depth_camera_calibration.resolution_height,
                                 calibration.depth_camera_calibration.resolution_width * (int)sizeof(k4a_float2_t),
                                 &xy_image);
                
                k4a_create_xy_table(&calibration, xy_image);
                v2f *XYMap = (v2f *)k4a_image_get_buffer(xy_image);
				
				size_t DepthMapSize = DepthMapCount * sizeof(uint16_t);
                uint16_t *DepthMap = (uint16_t *)malloc(DepthMapSize);
				
				open_gl *OpenGL = OpenGLInit(WindowWidth, WindowHeight);
				
				os_specifics OS = 
				{
				#if defined(_WIN32)
					glfwGetWGLContext(Window),
					GetDC(glfwGetWin32Window(Window))
				#elif defined(__linux__)
					glfwGetGLXContext(Window),
					glfwGetX11Display()
				#endif
				};
				
				open_cl *OpenCL = OpenCLInit(DepthMapWidth, DepthMapHeight, WindowWidth, WindowHeight, DepthMap, XYMap, &OS, OpenGL->framebuffer_texture);
                
                view_control Control_ = {
                    .model = mat4_identity(),
                    .position = {0.0f, 0.0f, 3.0f},
                    .forward = {0.0f, 0.0f, -1.0f},
                    .up = {0.0f, 1.0f, 0.0f},
                    .fov = 0.18f,
                    .speed = 1.5f,
                    .sensitivity = 0.0003f
                };
                view_control *Control = &Control_;

                float PointSize = 1.0f;
                
                float DeltaTime = 0.0f;
                float TotalTime = 0.0f;

				average AvgDrawTimeCPU = {.CountTo = 1000, .Msg = "Draw CPU", "ms"};
                average AvgWholeTime = {.CountTo = 1000, .Msg = "Whole", "ms"};
                
                unsigned FrameCount = 0;
                
				while(!glfwWindowShouldClose(Window))
				{
					double FrameTimeStart = glfwGetTime();
					
					// Control->position = (v3f){.x = 3 * linalg_sin(TotalTime), .y = 3 * linalg_cos(TotalTime), .z = 3.0f};
                    // Control->forward = v3f_add(v3f_negate(Control->position), (v3f){.z = -3.0f});
					
					handle_input(Window, Control, DeltaTime);
					bool DepthMapUpdate = camera_get_depth_map(Camera, 0, DepthMap, DepthMapSize);
					
					uint32_t RenderWidth;
					uint32_t RenderHeight;
					glfwGetFramebufferSize(Window, (int *)&RenderWidth, (int *)&RenderHeight);
                    
					bool WindowSizeChanged = (OpenGL->framebuffer_width != RenderWidth || OpenGL->framebuffer_height != RenderHeight);
					if(WindowSizeChanged && RenderWidth > 0 && RenderHeight > 0)
					{
						CLGLUpdateSettings(OpenCL, OpenGL, RenderWidth, RenderHeight);
					}
					
					OpenCLRenderToTexture(OpenCL, Camera->min_depth, Camera->max_depth, DepthMap, DepthMapWidth, DepthMapHeight, Control, DepthMapUpdate);

                    double DrawTimeBegin = glfwGetTime();
					OpenGLRenderToScreen(OpenGL, RenderWidth, RenderHeight);
                    PrintAverage(&AvgDrawTimeCPU, (glfwGetTime() - DrawTimeBegin) * 1000);
                    
					glfwSwapBuffers(Window);
					glfwPollEvents();
					
					double FrameTimeEnd = glfwGetTime();
					DeltaTime = (float)(FrameTimeEnd - FrameTimeStart);
					
					PrintAverage(&AvgWholeTime, DeltaTime * 1000);
					
					FrameCount++;
					
					TotalTime += DeltaTime;
				}
                
				//OpenCLRelease(OpenCL);
				
				//camera_release(Camera);
			}
			else
			{
				fprintf(stderr, "Could not initialize Camera.\n");
				ExitCode = -3;
			}	
			
			glfwDestroyWindow(Window);
		}
		else
		{
			fprintf(stderr, "Could not create GLFW Window.\n");
			ExitCode = -2;
		}
		
		glfwTerminate();
	}
	else
	{
		fprintf(stderr, "Could not initialize GLFW.\n");
		ExitCode = -1;
	}
	
	return(ExitCode);
}
