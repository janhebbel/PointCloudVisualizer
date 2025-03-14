#include <k4a/k4a.h>

#include <math.h>

typedef k4a_device_configuration_t camera_config;

typedef struct
{
    k4a_device_t device;
    camera_config *config;
    uint32_t max_capture_width;
    uint32_t max_capture_height;
	float min_depth;
	float max_depth;
} tof_camera;

void camera_mode_get_image_dimensions(k4a_depth_mode_t mode, uint32_t *width, uint32_t *height)
{
    switch(mode)
    {
        case K4A_DEPTH_MODE_NFOV_2X2BINNED:
        {
            *width = 320;
            *height = 288;
            break;
        }
        
        case K4A_DEPTH_MODE_NFOV_UNBINNED:
        {
            *width = 640;
            *height = 576;
            break;
        }
        
        case K4A_DEPTH_MODE_WFOV_2X2BINNED:
        {
            *width = 512;
            *height = 512;
            break;
        }
        
        case K4A_DEPTH_MODE_WFOV_UNBINNED:
        {
            *width = 1024;
            *height = 1024;
            break;
        }
        
        default:
        {
            *width = 0;
            *height = 0;
            break;
        }
    }
}

void camera_mode_get_operating_range(k4a_depth_mode_t mode, float *min_depth, float *max_depth)
{
    switch(mode)
    {
        case K4A_DEPTH_MODE_NFOV_2X2BINNED:
        {
            *min_depth = 0.5f;
			*max_depth = 5.46f;
            break;
        }
        
        case K4A_DEPTH_MODE_NFOV_UNBINNED:
        {
            *min_depth = 0.5f;
			*max_depth = 3.86f;
            break;
        }
        
        case K4A_DEPTH_MODE_WFOV_2X2BINNED:
        {
            *min_depth = 0.25f;
			*max_depth = 2.88f;
            break;
        }
        
        case K4A_DEPTH_MODE_WFOV_UNBINNED:
        {
            *min_depth = 0.25f;
			*max_depth = 2.21f;
            break;
        }
        
        default:
        {
            *min_depth = 0;
            *max_depth = 0;
            break;
        }
    }
}

tof_camera camera_init(camera_config *config) 
{
    tof_camera camera = {0};
    camera.config = config;
    camera_mode_get_image_dimensions(config->depth_mode, &camera.max_capture_width, &camera.max_capture_height);
	camera_mode_get_operating_range(config->depth_mode, &camera.min_depth, &camera.max_depth);
    
    if(k4a_device_get_installed_count())
    {
        if(K4A_RESULT_SUCCEEDED == k4a_device_open(K4A_DEVICE_DEFAULT, &camera.device))
        {
            if(K4A_RESULT_SUCCEEDED == k4a_device_start_cameras(camera.device, config)) {}
            else
            {
                k4a_device_close(camera.device);
                camera.device = NULL;
            }
        }
    }
    
    return(camera);
}

void camera_release(tof_camera *camera)
{
    k4a_device_stop_cameras(camera->device);
    k4a_device_close(camera->device);
    camera->device = NULL;
}

bool camera_get_depth_map(tof_camera *camera, int timeout, uint16_t *depth_map, size_t depth_map_size)
{
    bool update = false;
    k4a_capture_t capture = NULL;
    k4a_wait_result_t wait_result = k4a_device_get_capture(camera->device, &capture, timeout);
    if(K4A_WAIT_RESULT_SUCCEEDED == wait_result)
    {
        k4a_image_t image = k4a_capture_get_depth_image(capture);
        if(image)
        {
            size_t size = k4a_image_get_size(image);
            assert(size == depth_map_size);
            uint8_t *buffer = k4a_image_get_buffer(image);
            
            memcpy(depth_map, buffer, size);

            update = true;
            
            k4a_image_release(image);
        }
        
        k4a_capture_release(capture);
    }
    return update;
}


void k4a_create_xy_table(const k4a_calibration_t *calibration, k4a_image_t xy_image)
{
    k4a_float2_t *table_data = (k4a_float2_t *)(void *)k4a_image_get_buffer(xy_image);
    
    int width = calibration->depth_camera_calibration.resolution_width;
    int height = calibration->depth_camera_calibration.resolution_height;
    
    k4a_float2_t p;
    k4a_float3_t ray;
    int valid;
    
    for (int y = 0, idx = 0; y < height; y++)
    {
        p.xy.y = (float)y;
        for (int x = 0; x < width; x++, idx++)
        {
            p.xy.x = (float)x;
            
            k4a_calibration_2d_to_3d(calibration, &p, 1.f, K4A_CALIBRATION_TYPE_DEPTH, K4A_CALIBRATION_TYPE_DEPTH, &ray, &valid);
            
            if (valid)
            {
                table_data[idx].xy.x = ray.xyz.x;
                table_data[idx].xy.y = ray.xyz.y;
            }
            else
            {
                table_data[idx].xy.x = nanf("");
                table_data[idx].xy.y = nanf("");
            }
        }
    }
}
