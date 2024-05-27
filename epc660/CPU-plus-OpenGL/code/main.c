#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include <GLFW/glfw3.h>

#include "opengl_renderer.c"
#include "network.c"

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

void calculate_point_cloud(opengl_frame *frame, int *depth_map, int depth_map_width, int depth_map_height)
{
    int insert_index = 0;
    int depth_map_count = depth_map_width * depth_map_height;
    for(int i = 0; i < depth_map_count; ++i)
    {
        int pixel[2] = { i % depth_map_width, i / depth_map_width };
        int principal_point[2] = { depth_map_width / 2, depth_map_height / 2 };
        
        int d0 = (depth_map[i + depth_map_count * 0] & 0xFFF) - 2048;
        int d1 = (depth_map[i + depth_map_count * 1] & 0xFFF) - 2048;
        int d2 = (depth_map[i + depth_map_count * 2] & 0xFFF) - 2048;
        int d3 = (depth_map[i + depth_map_count * 3] & 0xFFF) - 2048;
        
        float diff0 = (float)(d3 - d1);
        float diff1 = (float)(d2 - d0);

        float c = 300000000.0f;
        float f = 12000000.0f;
        float pi = 3.1416f;

        float depth = (c / 2) * (1 / (2 * pi * f)) * (pi + atan2f(diff0, diff1));

        float focal_length = 50.0f * 3.7f; // pixels per mm * focal length [mm]

        float x = (pixel[0] - principal_point[0]) / focal_length;
        float y = (pixel[1] - principal_point[1]) / focal_length;

        float z = depth / sqrtf(x * x + y * y + 1);

        color_point point;
        point.xyz[0] = x * z;
        point.xyz[1] = y * z;
        point.xyz[2] = -z;
        
        float min_z = 0.0f;
        float max_z = 12.5f;
        
        if(z != 0.0f && z >= min_z && z <= max_z)
        {
            // interpolate

    #define clamp(x, low, high) (x) < (low) ? (low) : ((x) > (high) ? (high) : (x))
            
            float hue = (-point.xyz[2] - min_z) / (max_z - min_z);
            hue = clamp(hue, 0.0f, 1.0f);

            // the hue of the hsv color goes from red to red so we want to scale with 2/3 which is blue
            float range = 2.0f / 3.0f;
            
            hue *= range;
            hue = range - hue;

            point.rgb[0] = hue;
            point.rgb[1] = 1.0f;
            point.rgb[2] = 1.0f;

            frame->vertex_array[insert_index++] = point;
        }
    }

    frame->vertex_count = insert_index;
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
            
            glfwSwapInterval(0);
            
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
                
                depth_image_dimension dim = { depth_map_width, depth_map_height };
                open_gl *opengl = opengl_init(&dim);
                
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
                
                float delta_time = 0.0f;
                
                while(!glfwWindowShouldClose(window))
                {
                    double frame_time_start = glfwGetTime();
                    
                    handle_input(window, control, delta_time);
                    
                    v2u render_dim;
                    glfwGetFramebufferSize(window, (int *)&render_dim.x, (int *)&render_dim.y);
                    
                    opengl_frame *frame = opengl_begin_frame(opengl, render_dim);
                                       
                    // Here we are waiting for the producer thread to signal that the Buffer is full. We time out at 5ms which is ~200 Hz.
                    if(WaitForOtherThread(5))
                    {
                        // to_proper_layout() lays the depth data out in 4 consecutive images. Here we use the extra memory we allocated earlier.
                        to_proper_layout(depth_map, depth_map_size, depth_image_size, depth_map_width, depth_map_height, scratch_memory);
                        calculate_point_cloud(frame, (int *)depth_map, depth_map_width, depth_map_height);
                        
                        // Signal that the buffer was read so that the producer thread can start filling in the depth buffer.
                        SignalOtherThread();
                    }
                    
                    opengl_end_frame(opengl, frame, control);
                    
                    glfwSwapBuffers(window);
                    glfwPollEvents();
                    
                    double frame_time_end = glfwGetTime();
                    delta_time = (float)(frame_time_end - frame_time_start);
                    PrintFPS(delta_time);
                }

                free(scratch_memory);
                free(depth_map);

                TerminateMyThread();

                Disconnect(Connection.Host);
            }
            else
            {
                fprintf(stderr, "Could not establish a connection.\n");
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
