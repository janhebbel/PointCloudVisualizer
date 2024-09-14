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

void calculate_point_cloud(opengl_frame *frame, v2f *xy_map, uint16_t *depth_map, int depth_map_count)
{
    //float focal_length = 1.8f; // 1.8 mm = 0.0018 m

    uint32_t insert_index = 0;
    for(size_t i = 0; i < depth_map_count; ++i)
    {
        float d = (float)depth_map[i];
        
        //int u = (int)i / depth_map_width;
        //int v = (int)i % depth_map_width;
        
        //float x_over_z = (u - (depth_map_width  / 2.0f)) / focal_length;
        //float y_over_z = (v - (depth_map_height / 2.0f)) / focal_length;
        
        //float xc = (float)u / focal_length;
        //float yc = (float)v / focal_length;
        
        //float z = d / sqrtf(1.0f + x_over_z * x_over_z + y_over_z * y_over_z);
        
        color_point point;
        point.xyz[0] = xy_map[i].x * d / 1000.0f;
        point.xyz[1] = -xy_map[i].y * d / 1000.0f;
        point.xyz[2] = -d / 1000.0f /*+ max_camera_z*/;
        
        if(point.xyz[2] != 0.0f)
        {
            // interpolate
            float min_z = 0.5f;
            float max_z = 3.86f;

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
                
                depth_image_dimension dim = {depth_map_width, depth_map_height};
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
                
                size_t depth_map_size = depth_map_count * sizeof(uint16_t);
                uint16_t *depth_map = (uint16_t *)malloc(depth_map_size);
                
                float delta_time = 0.0f;
                float total_time = 0.0f;
                
                timer TimerCompute = {1000};
                timer TimerWhole = {1000};
                
                unsigned FrameCount = 0;
                
                while(!glfwWindowShouldClose(window))
                {
                    double frame_time_start = glfwGetTime();
                    
                    // handle_input(window, control, delta_time);
                    control->position = (v3f){.x = 5 * linalg_sin(total_time / 2), .z = 5 * linalg_cos(total_time / 2)};
                    control->forward = (v3f){.x = -control->position.x, .y = -control->position.y, .z = -control->position.z};
                    
                    v2u render_dim;
                    glfwGetFramebufferSize(window, (int *)&render_dim.x, (int *)&render_dim.y);
                    
                    opengl_frame *frame = opengl_begin_frame(opengl, render_dim);
                    
                    camera_get_depth_map(camera, 0, depth_map, depth_map_size);

                    //
                    // filling the point cloud with points
                    double TimeBegin = glfwGetTime();
                    calculate_point_cloud(frame, xy_map, depth_map, depth_map_count);
                    double TimeEnd = glfwGetTime();
                    if (FrameCount >= 4) PrintAverageTime(&TimerCompute, (float)(TimeEnd - TimeBegin), "Compute");
                    // done with filling the point cloud
                    // 
                    
                    // Render
                    opengl_end_frame(opengl, frame, control);
                    
                    glfwSwapBuffers(window);
                    glfwPollEvents();
                    
                    double frame_time_end = glfwGetTime();
                    delta_time = (float)(frame_time_end - frame_time_start);
                    if (FrameCount >= 4) PrintAverageTime(&TimerWhole, delta_time, "Whole");
                    
                    FrameCount++;
                    
                    total_time += delta_time;
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
