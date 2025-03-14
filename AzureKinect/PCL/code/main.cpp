#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <chrono>
#include <algorithm>

#include <pcl/common/common_headers.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <vtkOpenGLRenderer.h>

#include "k4a.c"

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
        fprintf(stdout, "%s: %f %s\n", Average->Msg, Avg, Average->Unit);
        Average->Acc = 0;
        Average->Count = 0;
    }
    else
    {
        Average->Acc += Value;
        Average->Count++;
    }
}

#define PROFILE
#ifdef PROFILE
#include <MinHook.h>
#include <GL/gl.h>

#define GL_TIME_ELAPSED                   0x88BF
#define GL_QUERY_RESULT                   0x8866
#define GL_QUERY_RESULT_AVAILABLE         0x8867

#define QUERY_COUNT 5

typedef uint64_t GLuint64;

typedef void (*type_glGenQueries)(GLsizei, GLuint *);
typedef void (*type_glBeginQuery)(GLenum, GLuint);
typedef void (*type_glEndQuery)(GLenum);
typedef void (*type_glGetQueryObjectiv)(GLuint, GLenum, GLint *);
typedef void (*type_glGetQueryObjectui64v)(GLuint, GLenum, GLuint64 *);

type_glGenQueries glGenQueries = NULL;
type_glBeginQuery glBeginQuery = NULL;
type_glEndQuery   glEndQuery   = NULL;
type_glGetQueryObjectiv glGetQueryObjectiv = NULL;
type_glGetQueryObjectui64v glGetQueryObjectui64v = NULL;

GLuint render_queries[QUERY_COUNT];
uint64_t frame_count = 0;
int clear_count_per_frame = 0;

typedef BOOL (WINAPI *wglSwapIntervalEXTFunc)(int);

wglSwapIntervalEXTFunc wglSwapIntervalEXT = 0;

int SwapBufferCount = 0;
double SwapBuffersTime = 0.0;
double FrameTime = 0.0;
double DrawTime = 0.0;
double DrawTimeCPU = 0.0;
std::chrono::steady_clock::time_point DrawTimeCPUBegin = {};
std::chrono::steady_clock::time_point Last = {};

typedef BOOL (WINAPI *SwapBuffersFunc)(HDC);

SwapBuffersFunc OriginalSwapBuffers = NULL;

BOOL WINAPI MySwapBuffers(HDC DeviceContext) 
{
    int query_index = frame_count % QUERY_COUNT;
    glEndQuery(GL_TIME_ELAPSED);

    int prev_query_index = (query_index + 1) % QUERY_COUNT;
    if (frame_count >= 4) {
        GLint prev_query_available;
        glGetQueryObjectiv(render_queries[prev_query_index], GL_QUERY_RESULT_AVAILABLE, &prev_query_available);
        if (prev_query_available) {
            GLuint64 time_elapsed;
            glGetQueryObjectui64v(render_queries[prev_query_index], GL_QUERY_RESULT, &time_elapsed);
            DrawTime += time_elapsed / 1000000.0;
            // fprintf(stdout, "time elapsed: %.3f ms\n", time_elapsed / 1000000.0);
        }
    }

    std::chrono::steady_clock::time_point DrawTimeCPUEnd = std::chrono::steady_clock::now();
    double DrawTimeCPUDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(DrawTimeCPUEnd - DrawTimeCPUBegin).count() / 1e6f;
    DrawTimeCPU += DrawTimeCPUDelta;

    std::chrono::steady_clock::time_point Begin = std::chrono::steady_clock::now();

    BOOL Result = OriginalSwapBuffers(DeviceContext);

    std::chrono::steady_clock::time_point End = std::chrono::steady_clock::now();

    double Delta = std::chrono::duration_cast<std::chrono::nanoseconds>(End - Begin).count() / 1000000.0;
    double FrameTimeDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(End - Last).count() / 1000000.0;

    SwapBuffersTime += Delta;
    SwapBufferCount += 1;

    FrameTime += FrameTimeDelta;
    Last = End;

    frame_count += 1;
    clear_count_per_frame = 0;

    return Result;
}

typedef void (*glClearFunc)(GLbitfield);

glClearFunc OriginalglClear = NULL;

void MyglClear(GLbitfield mask) {
    // Only start the GPU timer once per frame on the first call to glClear.
    if (clear_count_per_frame == 0) {
        DrawTimeCPUBegin = std::chrono::steady_clock::now();

        clear_count_per_frame += 1;
        int query_index = frame_count % QUERY_COUNT;
        glBeginQuery(GL_TIME_ELAPSED, render_queries[query_index]);
    }

    OriginalglClear(mask);
}

#endif

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

int main(void)
{
#ifdef PROFILE
    if (MH_Initialize() != MH_OK) {
        return 1;
    }

    if (MH_CreateHook(&SwapBuffers, &MySwapBuffers, (LPVOID *)&OriginalSwapBuffers) != MH_OK) {
        return 1;
    }

    if (MH_CreateHook(&glClear, &MyglClear, (LPVOID *)&OriginalglClear) != MH_OK) {
        return 1;
    }

    if (MH_EnableHook(&SwapBuffers) != MH_OK) {
        return 1;
    }

    if (MH_EnableHook(&glClear) != MH_OK) {
        return 1;
    }
#endif

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

        boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer (new pcl::visualization::PCLVisualizer ("3D Viewer"));
        pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_ptr (new pcl::PointCloud<pcl::PointXYZRGB>);
        viewer->addPointCloud<pcl::PointXYZRGB>(cloud_ptr, "sample cloud");
        viewer->setBackgroundColor(0, 0, 0);
        viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 1, "sample cloud");
        // viewer->addCoordinateSystem(1.0);
        viewer->initCameraParameters();

#ifdef PROFILE
        HDC DeviceContext = wglGetCurrentDC();
        HGLRC GLDeviceContext = wglGetCurrentContext();
        if (wglSwapIntervalEXT == NULL) {
            if (wglMakeCurrent(DeviceContext, GLDeviceContext)) {
                wglSwapIntervalEXT = (wglSwapIntervalEXTFunc)wglGetProcAddress("wglSwapIntervalEXT");
                if (wglSwapIntervalEXT) {
                    wglSwapIntervalEXT(0);
                }

                glGenQueries = (type_glGenQueries)wglGetProcAddress("glGenQueries");
                glBeginQuery = (type_glBeginQuery)wglGetProcAddress("glBeginQuery");
                glEndQuery = (type_glEndQuery)wglGetProcAddress("glEndQuery");
                glGetQueryObjectiv = (type_glGetQueryObjectiv)wglGetProcAddress("glGetQueryObjectiv");
                glGetQueryObjectui64v = (type_glGetQueryObjectui64v)wglGetProcAddress("glGetQueryObjectui64v");

                //wglMakeCurrent(0, 0);
            }
        }

        glGenQueries(QUERY_COUNT, (GLuint *)render_queries);

        Last = std::chrono::steady_clock::now();
#endif

        float DeltaTime = 0.0f;
        float TotalTime = 0.0f;

        average AvgCompute = {1000, "Compute", "ms"};
        average AvgRender = {1000, "Spin Once Duration", "ms"};
        average AvgLoop = {1000, "Loop", "ms"};
        average SwapBufferCountAvg = {1000, "SwapBuffers Calls", ""};
        average SwapBufferTimeAvg = {1000, "SwapBuffers Time", "ms"};
        average DrawTimeCPUAvg = {1000, "Draw Time CPU", "ms"};
        average AvgRenderGPU = {1000, "Draw Time GPU", "ms"};
        average AvgFullConversion = {1000, "Full Conversion", "ms"};
        average FrameTimeAvg = {1000, "Frame Time", "ms"};

        viewer->setCameraPosition(0, 0, -3, 0, 0, 0, 0, 1, 0);

        int FrameCount = 0;
        int DepthImageCount = 0;

        while(!viewer->wasStopped())
        {
            // measure whole frame time start
            std::chrono::steady_clock::time_point Begin = std::chrono::steady_clock::now();

            FrameCount++;

            if (FrameCount % 1000 == 0)
            {
                printf("DepthImageCount: %d\n", DepthImageCount);
                DepthImageCount = 0;
            }

            if (FrameCount == INT_MAX)
            {
                FrameCount = 0;
            }

            // fprintf(stdout, "New Frame.\n");
#ifdef PROFILE
            HWND Window = GetActiveWindow();
            for (int i = 0; i < 100; ++i) {
                RedrawWindow(Window, NULL, NULL, RDW_INVALIDATE);
            }
#endif

            bool DepthMapUpdate = camera_get_depth_map(Camera, 0, DepthMap, DepthMapSize);
            DepthImageCount += DepthMapUpdate;

            // measure start
            std::chrono::steady_clock::time_point TimeBegin = std::chrono::steady_clock::now();

            // computing point cloud
            if (DepthMapUpdate)
            {
                std::chrono::steady_clock::time_point FullConversionTimeBegin = std::chrono::steady_clock::now();

                cloud_ptr->points.clear();

                uint32_t InsertIndex = 0;
                for(size_t i = 0; i < DepthMapCount; ++i)
                {
                    float d = (float)DepthMap[i];

                    pcl::PointXYZRGB Point;
                    Point.x = -XYMap[i].x * d / 1000.0f;
                    Point.y = -XYMap[i].y * d / 1000.0f;
                    Point.z = d / 1000.0f;

                    if(Point.z != 0.0f)
                    {
                        // interpolate
                        float min_z = 0.5f;
                        float max_z = 3.86f;

                        float hue = (Point.z - min_z) / (max_z - min_z);
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

                std::chrono::steady_clock::time_point FullConversionTimeEnd = std::chrono::steady_clock::now();
                float ConversionTimeElapsed = std::chrono::duration_cast<std::chrono::microseconds>(FullConversionTimeEnd - FullConversionTimeBegin).count() / 1000.0;
                PrintAverage(&AvgFullConversion, ConversionTimeElapsed);
            }

            // measure compute
            std::chrono::steady_clock::time_point TimeEnd = std::chrono::steady_clock::now();
            float Elapsed = std::chrono::duration_cast<std::chrono::microseconds>(TimeEnd - TimeBegin).count() / 1000.0;
            PrintAverage(&AvgCompute, Elapsed);

            // measure draw time start
            TimeBegin = std::chrono::steady_clock::now();

            // draw point cloud
#define DYNAMIC_TEST 0
#if DYNAMIC_TEST
            viewer->setCameraPosition(3.0f * sinf(TotalTime), 3.0f * cosf(TotalTime), -3.0f, 0.0f, 0.0f, 3.0f, 0.0f, 1.0f, 0.0f);
#endif
            viewer->updatePointCloud(cloud_ptr, "sample cloud");
            viewer->spinOnce(0, true);

            TimeEnd = std::chrono::steady_clock::now();

            Elapsed = std::chrono::duration_cast<std::chrono::microseconds>(TimeEnd - TimeBegin).count() / 1000.0;
            PrintAverage(&AvgRender, Elapsed);

#ifdef PROFILE
            // Dividing by SwapBufferCount is required as SwapBuffers() is called multiple times per main loop
            // iteration. However, RenderDoc (correctly) shows that every frame calls SwapBuffers once, since 1
            // SwapBuffers call is per definition making up one frame.
            SwapBuffersTime /= SwapBufferCount;
            FrameTime /= SwapBufferCount;
            DrawTime /= SwapBufferCount;
            DrawTimeCPU /= SwapBufferCount;
            // fprintf(stdout, "%d, %.3f ms\n", SwapBufferCount, SwapBuffersTime);
            PrintAverage(&SwapBufferCountAvg, SwapBufferCount);
            PrintAverage(&SwapBufferTimeAvg, SwapBuffersTime);
            PrintAverage(&FrameTimeAvg, FrameTime);
            PrintAverage(&AvgRenderGPU, DrawTime);
            PrintAverage(&DrawTimeCPUAvg, DrawTimeCPU);
            FrameTime = 0.0;
            SwapBuffersTime = 0.0;
            DrawTime = 0.0;
            DrawTimeCPU = 0.0;
            SwapBufferCount = 0;
#endif

            // measure whole frame time end
            std::chrono::steady_clock::time_point End = std::chrono::steady_clock::now();
            DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(End - Begin).count() / 1e6f;
            PrintAverage(&AvgLoop, DeltaTime * 1000.0f);

            TotalTime += DeltaTime;
        }
    }

#ifdef PROFILE
    if (MH_DisableHook(&SwapBuffers) != MH_OK) {
        return 1;
    }

    if (MH_Uninitialize() != MH_OK) {
        return 1;
    }
#endif

    return(0);
}
