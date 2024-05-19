#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include "k4a.c"

typedef struct
{
    float x;
    float y;
}
v2f;

typedef struct
{
    struct
    {
        float X;
        float Y;
        float Z;
    }
    Position;
}
point;

static void calculate_point_cloud(point *PointCloud, uint32_t *PointCount, v2f *XYMap, uint16_t *DepthMap, int DepthMapCount)
{
    uint32_t InsertIndex = 0;
    for(size_t i = 0; i < DepthMapCount; ++i)
    {
        float d = (float)DepthMap[i];
        
        point Point;
        Point.Position.X = XYMap[i].x * d / 1000.0f;
        Point.Position.Y = -XYMap[i].y * d / 1000.0f;
        Point.Position.Z = -d / 1000.0f;
        
        if(Point.Position.Z != 0.0f)
        {
            // interpolate
            float min_z = 0.5f;
            float max_z = 3.86f;

#define clamp(x, low, high) (x) < (low) ? (low) : ((x) > (high) ? (high) : (x))
            
            float hue = (-Point.Position.Z - min_z) / (max_z - min_z);
            hue = clamp(hue, 0.0f, 1.0f);

            // the hue of the hsv color goes from red to red so we want to scale with 2/3 which is blue
            float range = 2.0f / 3.0f;
            
            hue *= range;
            hue = range - hue;

            // Point.rgb[0] = hue;
            // Point.rgb[1] = 1.0f;
            // Point.rgb[2] = 1.0f;

            PointCloud[InsertIndex++] = Point;
        }
    }

    *PointCount = InsertIndex;
}

int main(void)
{
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

        point *PointCloud = (point *)malloc(DepthMapCount * sizeof(point));
        uint32_t PointCount;

        while(1)
        {
            camera_get_depth_map(Camera, 0, DepthMap, DepthMapSize);
            calculate_point_cloud(PointCloud, &PointCount, XYMap, DepthMap, DepthMapCount);

            // display using pcl

            break;
        }
    }

    return(0);
}