#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)

#include <winsock2.h>
#include <ws2tcpip.h>
#include <strsafe.h>

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

#include "linalg.h"
#include "types.h"
#include "opengl.c"
#include "opencl.c"
#include "opencl_opengl.c"
#include "network.c"

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

static void PrintFPS(float DeltaTime)
{
	static int Count = 0;
	static float FrameTimeAcc = 0;
	static int StartingUp = 1;
    
    if(StartingUp)
    {
        Count++;
        if(Count > 100)
        {
            StartingUp = 0;
            Count = 0;
        }
    }
    else
    {
        int FramesToSumUp = 1000;
        if(Count == FramesToSumUp)
        {
            printf("%f ms, %f fps\n", (FrameTimeAcc/(float)FramesToSumUp), 1.0f / (FrameTimeAcc / (float)FramesToSumUp));
            FrameTimeAcc = 0;
            Count = 0;
        }
        else
        {
            FrameTimeAcc += DeltaTime;
            Count++;
        }
    }
}

void to_proper_layout(uint8_t *depth_map, size_t depth_map_size, size_t single_image_size, int width, int height, uint8_t *scratch_memory)
{
    memcpy(scratch_memory, depth_map, depth_map_size);

    size_t row_size = width * sizeof(int);

    int k = 0;
    for(int i = 0; i < 4; ++i)
    {
        uint8_t *scratch_image = scratch_memory + i * single_image_size;

        // go through image by 2 rows starting at the end and going to 1
        for(int j = height - 2; j >= 0; j -= 2)
        {
            uint8_t *scratch_row = scratch_image + j * row_size;
            uint8_t *row = depth_map + k * row_size;

            memcpy(row, scratch_row, row_size);

            ++k;
        }

        // go through image by 2 rows starting at beginning going to end
        for(int j = 1; j < height; j += 2)
        {
            uint8_t *scratch_row = scratch_image + j * row_size;
            uint8_t *row = depth_map + k * row_size;

            memcpy(row, scratch_row, row_size);
        
            ++k;
        }
    }
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
			
			connection Connection =
            {
                INVALID_SOCKET,
                INVALID_SOCKET
            };

            // This will create a socket, bind it, listen and accept when a connection comes in.
            int Connected = Connect(&Connection);
            if(0 == Connected)
			{
				uint32_t depth_map_width = 320;
                uint32_t depth_map_height = 240;
                uint32_t depth_image_size = 307200;

                uint32_t depth_map_size = depth_image_size * 4;

                uint8_t *depth_map = (uint8_t *)malloc(depth_map_size);
                if(NULL == depth_map)
                {
                    fprintf(stderr, "Not enough memory available to run this process.\n");
                    exit(-1);
                }

                // Allocate memory to temporarily operate in when laying out memory properly.
                uint8_t *scratch_memory = (uint8_t *)malloc(depth_map_size);
                if(NULL == scratch_memory)
                {
                    fprintf(stderr, "Not enough memory available to run.\n");
                    exit(-1);
                }

                // This all relevant data the thread functions needs. (Kinda like normal function parameters.)
                get_depth_image_data ThreadDataIn = 
                {
                    Connection.Client,
                    depth_map,
                    depth_map_size,
                    depth_image_size
                };

                // Starts a producer thread that gets the data from the ToF-camera and puts it into depth_map.
                CreateMyThread(&ThreadDataIn);
				
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
				
				open_cl *OpenCL = OpenCLInit(depth_map_width, depth_map_height, WindowWidth, WindowHeight, (int *)depth_map, &OS, OpenGL->framebuffer_texture);
                
                view_control Control_ = {
                    .model = mat4_identity(),
                    .position = {0.0f, 0.0f, 0.0f},
                    .forward = {0.0f, 0.0f, -1.0f},
                    .up = {0.0f, 1.0f, 0.0f},
                    .fov = 0.18f,
                    .speed = 1.5f,
                    .sensitivity = 0.0003f
                };
                view_control *Control = &Control_;

                float PointSize = 1.0f;
                
                float DeltaTime = 0.0f;
				
				while(!glfwWindowShouldClose(Window))
				{
					double FrameTimeStart = glfwGetTime();
					
					handle_input(Window, Control, DeltaTime);
					
					uint32_t RenderWidth;
					uint32_t RenderHeight;
					glfwGetFramebufferSize(Window, (int *)&RenderWidth, (int *)&RenderHeight);
                    
					bool WindowSizeChanged = (OpenGL->framebuffer_width != RenderWidth || OpenGL->framebuffer_height != RenderHeight);
					if(WindowSizeChanged && RenderWidth > 0 && RenderHeight > 0)
					{
						CLGLUpdateSettings(OpenCL, OpenGL, RenderWidth, RenderHeight);
					}

                    // Here we are waiting for the producer thread to signal that the Buffer is full. We time out at 5ms which is ~200 Hz.
                    if(WaitForOtherThread(5))
                    {
                        // to_proper_layout() lays the depth data out in 4 consecutive images. Here we use the extra memory we allocated earlier.
                        to_proper_layout(depth_map, depth_map_size, depth_image_size, depth_map_width, depth_map_height, scratch_memory);
    					OpenCLRenderToTexture(OpenCL, (int *)depth_map, depth_map_width, depth_map_height, Control);
                        
                        // Signal that the buffer was read so that the producer thread can start filling in the depth buffer.
                        SignalOtherThread();
                    }
					
					OpenGLRenderToScreen(OpenGL, RenderWidth, RenderHeight);
                    
					glfwSwapBuffers(Window);
					glfwPollEvents();
					
					double FrameTimeEnd = glfwGetTime();
					DeltaTime = (float)(FrameTimeEnd - FrameTimeStart);
					
					PrintFPS(DeltaTime);
				}
                
				free(scratch_memory);
                free(depth_map);

                TerminateMyThread();

                Disconnect(Connection.Host);
			}
			else
			{
				fprintf(stderr, "Could not establish a connection.\n");
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
