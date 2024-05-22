#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#include <GLFW/glfw3.h>

//#include "testing.c"

#include "network.c"
#include "opengl_renderer.c"
//#include "write_to_ply.c"

#include "linalg.h"
#include "opengl_renderer.h"

struct scroll_update { 
    double yoffset;
    int    updated;
};
// NOTE: this has to be a global since we can only retrieve the scroll offset in the callback
static struct scroll_update global_scroll_update;

void handle_input(GLFWwindow *window, view_control *control, float delta_time)
{
    //
    // mouse input
    double xpos, ypos;
    static double last_xpos, last_ypos;
    
    glfwGetCursorPos(window, &xpos, &ypos);
    
    float dx = (float)(xpos - last_xpos) * control->sensitivity;
    float dy = -(float)(ypos - last_ypos) * control->sensitivity;

    // first person camera controller
    if(GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT))
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
        
        state = glfwGetKey(window, GLFW_KEY_W);
        if(state == GLFW_PRESS)
        {
            add = v3f_add(add, control->forward);
        }
        state = glfwGetKey(window, GLFW_KEY_A);
        if(state == GLFW_PRESS)
        {
            add = v3f_sub(add, v3f_normalize(v3f_cross(control->forward, control->up)));
        }
        state = glfwGetKey(window, GLFW_KEY_S);
        if(state == GLFW_PRESS)
        {
            add = v3f_sub(add, control->forward);
        }
        state = glfwGetKey(window, GLFW_KEY_D);
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if((button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else if((button == GLFW_MOUSE_BUTTON_RIGHT) && action == GLFW_RELEASE)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
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

void ToProperLayout(uint8_t *depth_map, size_t depth_map_size, size_t single_image_size, int width, int height, uint8_t *scratch_memory)
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
    // Initializing windowing library that works for Linux and Windows.
    if(glfwInit())
    {
        glfwSetErrorCallback(glfw_error_callback);

        // NOTE: Get an OpenGL 4.3 Core context
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        GLFWwindow *window = glfwCreateWindow(1280, 720, "Point Cloud Visualizer", NULL, NULL);
        if(window)
        {
            glfwMakeContextCurrent(window);
            
            // Disable vsync.
            glfwSwapInterval(0);
            
            // Setting up callback functions for mouse buttons and scroll wheel.
            glfwSetMouseButtonCallback(window, mouse_button_callback);
            glfwSetScrollCallback(window, scroll_callback);
            
            connection Connection =
            {
                INVALID_SOCKET,
                INVALID_SOCKET
            };

            // This will create a socket, bind it, listen and accept when a connection comes in.
            int Connected = Connect(&Connection);
            if(0 == Connected)
            {
                printf("Connected!\n");

                uint32_t depth_map_width = 320;
                uint32_t depth_map_height = 240;
                uint32_t depth_image_size = 307200;

                uint32_t depth_map_size = depth_image_size * 4;

                // Allocating here now so we don't have to malloc and free every time in the main loop.
                uint8_t *depth_map = (uint8_t *)malloc(depth_map_size);
                if(NULL == depth_map)
                {
                    fprintf(stderr, "Not enough memory available to run this process.\n");
                    exit(-1);
                }

                // Allocate memory to temporarily operate in when laying out memory properly.
                uint8_t *scratch_memory = (uint8_t *)malloc(depth_map_size);

                // This all relevant data the thread functions needs. (Kinda like normal function parameters.)
                get_depth_image_data ThreadDataIn = 
                {
                    Connection.Client,
                    depth_map,
                    depth_map_size,
                    depth_image_size
                };

                // Starts a "producer" thread that gets the data from the ToF-camera and puts it into depth_map.
                CreateMyThread(&ThreadDataIn);

                dimensions depth_image_dimensions = { depth_map_width, depth_map_height };
                open_gl *opengl = opengl_init(depth_image_dimensions);
                
                // view_control is a structure that gets modified in the handle_input() function and is then used to
                // create / use the appropriate matrices later when rendering in render_point_cloud().
                view_control control_ = {
                    .model = mat4_identity(),
                    .position = {0.0f, 0.0f, 0.0f},
                    .forward = {0.0f, 0.0f, -1.0f},
                    .up = {0.0f, 1.0f, 0.0f},
                    .fov = 0.18f,
                    .speed = 1.5f,
                    .sensitivity = 0.0003f
                };
                view_control *control = &control_;

                float point_size = 1.0f;
                
                float delta_time = 0.0f;
                
                // Starting the main loop.
                while(!glfwWindowShouldClose(window))
                {
                    double frame_time_start = glfwGetTime();
                    
                    handle_input(window, control, delta_time);
                    
                    dimensions render_dimensions;
                    glfwGetFramebufferSize(window, (int *)&render_dimensions.w, (int *)&render_dimensions.h);

                    // Here we are waiting for the producer thread to signal that the Buffer is full. We time out at 5ms which is ~200 Hz.
                    DWORD Result = WaitForSingleObject(EventBufferFull, 5); // 5 ms
                    if(Result == WAIT_OBJECT_0) // WAIT_OBJECT_0 == EventBufferFull was signaled
                    {
                        // ToProperLayout() lays the depth data out in 4 consecutive images. Here we use the extra memory we allocated earlier.
                        ToProperLayout(depth_map, depth_map_size, depth_image_size, depth_map_width, depth_map_height, scratch_memory);
                        calculate_point_cloud(opengl, depth_map, depth_image_size);
                        // Signal that the buffer was read so that the producer thread can start filling in the depth buffer.
                        SetEvent(EventBufferRead);
                    }

                    // Using OpenGL to draw to the screen.
                    render_point_cloud(opengl, render_dimensions, control, point_size);

                    glfwSwapBuffers(window); // This updates the monitor screen with the rendered image.
                    glfwPollEvents(); // This consults the operating system to handle and store input events.
                    
                    double frame_time_end = glfwGetTime();
                    delta_time = (float)(frame_time_end - frame_time_start);
                    
                    PrintFPS(delta_time);
                }

                free(scratch_memory);

                // I'm intentionally not freeing things religiously since they will get cleaned up by the operating system.
                // And it would just slow down the closing process for no reason.
                
                // Close the connection.
                Disconnect(Connection.Host);
            }
            else
            {
                fprintf(stderr, "Could not create the connection.\n");
            }
            
            glfwDestroyWindow(window);
        }
        else
        {
            fprintf(stderr, "Could not create GLFW window.\n");
        }
        
        glfwTerminate();
    }
    else
    {
        fprintf(stderr, "Could not initialize GLFW.\n");
    }
    
    return(0);
}
