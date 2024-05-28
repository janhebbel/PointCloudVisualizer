#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <chrono>
#include <algorithm>
#include <cmath>

#include <pcl/common/common_headers.h>
#include <pcl/visualization/pcl_visualizer.h>

#include "network.c"

#define clamp(x, low, high) std::max(low, std::min(high, x))

typedef struct
{
    float x;
    float y;
}
v2f;

typedef struct
{
    float x;
    float y;
    float z;
}
v3f;

typedef struct
{
    struct
    {
        float X;
        float Y;
        float Z;
    }
    Position;

    struct
    {
        float R;
        float G;
        float B;
    }
    Color;
}
point;

v3f HSV2RGB(v3f HSV) 
{
	v3f RGB;
	
    int I;
    float F, P, Q, T;
	
	float H = HSV.x;
	float S = HSV.y;
	float V = HSV.z;
	
    if (S == 0) // No saturation => grayscale; V == lightness/darkness
	{
        RGB = { V, V, V };
    }
	else
	{
		H *= 6;
		I = (int)H;
		F = H - I;
		P = V * (1 - S);
		Q = V * (1 - S * F);
		T = V * (1 - S * (1 - F));
		switch (I) {
			case 0:  RGB = {V, T, P}; break;
			case 1:  RGB = {Q, V, P}; break;
			case 2:  RGB = {P, V, T}; break;
			case 3:  RGB = {P, Q, V}; break;
			case 4:  RGB = {T, P, V}; break;
			default: RGB = {V, P, Q}; break;
		}
	}
	
	return(RGB);
}

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
            
            float hue = (-Point.Position.Z - min_z) / (max_z - min_z);
            hue = clamp(hue, 0.0f, 1.0f);

            // the hue of the hsv color goes from red to red so we want to scale with 2/3 which is blue
            float range = 2.0f / 3.0f;
            
            hue *= range;
            hue = range - hue;

            v3f RGB = HSV2RGB({ hue, 1.0f, 1.0f });

            Point.Color.R = RGB.x;
            Point.Color.G = RGB.y;
            Point.Color.B = RGB.z;

            PointCloud[InsertIndex++] = Point;
        }
    }

    *PointCount = InsertIndex;
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

int main(void)
{
    connection Connection =
    {
        INVALID_SOCKET,
        INVALID_SOCKET
    };

    // This will create a socket, bind it, listen and accept when a connection comes in.
    int Connected = Connect(&Connection);
    if(0 == Connected)
    {
        int depth_map_width = 320;
        int depth_map_height = 240;
        int depth_image_size = 307200;

        size_t depth_map_size = depth_image_size * 4;

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

        boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer (new pcl::visualization::PCLVisualizer ("3D Viewer"));
        pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_ptr (new pcl::PointCloud<pcl::PointXYZRGB>);
        viewer->addPointCloud<pcl::PointXYZRGB>(cloud_ptr, "sample cloud");
        viewer->setBackgroundColor(0, 0, 0);
        viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1, "sample cloud");
        viewer->addCoordinateSystem(1.0);
        viewer->initCameraParameters();

        float DeltaTime = 0.0f;

        while(!viewer->wasStopped())
        {
            std::chrono::steady_clock::time_point Begin = std::chrono::steady_clock::now();

            // Here we are waiting for the producer thread to signal that the Buffer is full. We time out at 5ms which is ~200 Hz.
            if(WaitForOtherThread(5))
            {
                // to_proper_layout() lays the depth data out in 4 consecutive images. Here we use the extra memory we allocated earlier.
                to_proper_layout(depth_map, depth_map_size, depth_image_size, depth_map_width, depth_map_height, scratch_memory);

                // fill PCL point cloud with new data
                cloud_ptr->points.clear();
            
                uint32_t InsertIndex = 0;
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
                    
                    pcl::PointXYZRGB Point;
                    Point.x = x * z;
                    Point.y = y * z;
                    Point.z = -z;

                    float min_z = 0.0f;
                    float max_z = 12.5f;
                    
                    if(z != 0.0f || z >= min_z || z <= max_z)
                    {
                        // interpolate
                        
                        float hue = (z - min_z) / (max_z - min_z);
                        hue = clamp(hue, 0.0f, 1.0f);

                        // the hue of the hsv color goes from red to red so we want to scale with 2/3 which is blue
                        float range = 2.0f / 3.0f;
                        
                        hue *= range;
                        hue = range - hue;

                        v3f RGB = HSV2RGB({ hue, 1.0f, 1.0f });

                        Point.r = RGB.x * 255;
                        Point.g = RGB.y * 255;
                        Point.b = RGB.z * 255;

                        cloud_ptr->points.push_back(Point);
                    }
                }

                cloud_ptr->width = (int)cloud_ptr->points.size();
                cloud_ptr->height = 1;
                
                // Signal that the buffer was read so that the producer thread can start filling in the depth buffer.
                SignalOtherThread();
            }

            // display using pcl
            viewer->updatePointCloud(cloud_ptr, "sample cloud");
            viewer->spinOnce(0);

            std::chrono::steady_clock::time_point End = std::chrono::steady_clock::now();
            DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(End - Begin).count() / 1000000.0;
            PrintFPS(DeltaTime);
        }

        free(scratch_memory);
        free(depth_map);

        TerminateMyThread();

        Disconnect(Connection.Host);
    }

    return(0);
}