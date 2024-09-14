#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#include <GLFW/glfw3.h>

typedef struct s_timer {
    const int FramesToSumUp;
    int Count;
    float Acc;
} timer;

// Prints the average time in ms, assuming DeltaTime is in seconds after accumulating Timer->FramesToSumUp times
static void PrintAverageTime(timer *Timer, float DeltaTime, const char *text)
{
    if(Timer->Count == Timer->FramesToSumUp)
    {
        float AvgFrameTime = Timer->Acc / (float)Timer->FramesToSumUp;
        printf("%s: %f ms\n", text, AvgFrameTime * 1000.0f);
        Timer->Acc = 0;
        Timer->Count = 0;
    }
    else
    {
        Timer->Acc += DeltaTime;
        Timer->Count++;
    }
}

#include "k4a.c"
#include "opengl_renderer.c"
#include "write_to_ply.c"

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
            
            camera_config config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
            config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
            config.camera_fps = K4A_FRAMES_PER_SECOND_30;
            config.synchronized_images_only = false;

            tof_camera camera_ = camera_init(&config);
            tof_camera *camera = &camera_;

            if(camera->device)
            {
                int depth_map_width = camera->max_capture_width;
                int depth_map_height = camera->max_capture_height;
                int depth_map_count = depth_map_width * depth_map_height;

                k4a_calibration_t calibration;
                k4a_device_get_calibration(camera->device, config.depth_mode, config.color_resolution, &calibration);
                
                k4a_image_t xy_image = NULL;
                k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM,
                                 calibration.depth_camera_calibration.resolution_width,
                                 calibration.depth_camera_calibration.resolution_height,
                                 calibration.depth_camera_calibration.resolution_width * (int)sizeof(k4a_float2_t),
                                 &xy_image);
                
                k4a_create_xy_table(&calibration, xy_image);
                v2f *xy_map = (v2f *)k4a_image_get_buffer(xy_image);
                
                dimensions depth_image_dimensions = {depth_map_width, depth_map_height};
                open_gl *opengl = opengl_init(depth_image_dimensions);
                
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
                
                size_t depth_map_size = depth_map_count * sizeof(uint16_t);
                uint16_t *depth_map = (uint16_t *)malloc(depth_map_size);
                
                float delta_time = 0.0f;
                float total_time = 0.0f;
                
                timer FrameTimer = {1000};
                
                unsigned FrameCount = 0;
                
                while(!glfwWindowShouldClose(window))
                {
                    double frame_time_start = glfwGetTime();
                    
                    // No input
                    // handle_input(window, control, delta_time);
                    
                    control->position = (v3f){.x = 5 * linalg_sin(total_time / 2), .z = 5 * linalg_cos(total_time / 2)};
                    control->forward = (v3f){.x = -control->position.x, .y = -control->position.y, .z = -control->position.z};
                    
                    dimensions render_dimensions;
                    glfwGetFramebufferSize(window, (int *)&render_dimensions.w, (int *)&render_dimensions.h);
                                        
                    camera_get_depth_map(camera, 0, depth_map, depth_map_size);

                    calculate_point_cloud(opengl, xy_map, depth_map);
                    render_point_cloud(opengl, render_dimensions, control, point_size);
                    
                    glfwSwapBuffers(window);
                    glfwPollEvents();
                    
                    double frame_time_end = glfwGetTime();
                    delta_time = (float)(frame_time_end - frame_time_start);
                    if (FrameCount >= 4) PrintAverageTime(&FrameTimer, delta_time, "Whole");
                    
                    total_time += delta_time;
                    FrameCount++;
                }
                
                // Calling this increases the closing time noticeably...
                //camera_release(camera);
            }
            else
            {
                fprintf(stderr, "Could not initialize camera.\n");
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
