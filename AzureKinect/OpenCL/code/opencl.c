#include <CL/opencl.h>

typedef cl_int (*clGetGLContextInfoKHR_fn)(const cl_context_properties* properties, cl_gl_context_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);

#include "limits.h"

#define Size(Array) (sizeof(Array) / sizeof(Array[0]))

#define QUERY_COUNT 5

typedef struct
{
    cl_platform_id Platform;
    cl_device_id Device;
    
    cl_context Context;
    
    cl_command_queue CommandQueue;
    
    cl_kernel PointCloudComputeKernel;
    cl_kernel PipelineKernel;
    
    cl_program PointCloudComputeProgram;
    cl_program PipelineProgram;
    
    cl_mem Framebuffer;
    cl_mem DepthBuffer;
    cl_mem DepthMapImage;
    cl_mem XYMapImage;
    cl_mem PositionImage;
    cl_mem ColorImage;

    cl_event FirstAndLastEvent[2][QUERY_COUNT][2];
    
    bool SupportsGLContextSharing;
    
    uint32_t FramebufferWidth;
    uint32_t FramebufferHeight;
} open_cl;

typedef cl_int type_clIcdGetPlatformIDsKHR(cl_uint num_entries, cl_platform_id* platforms, cl_uint* num_platforms);

void CL_CALLBACK ContextCallback(const char *ErrorInfo, const void *PrivateInfo, size_t CB, void *UserData)
{
    fprintf(stderr, "CL CONTEXT ERROR: %s\n", ErrorInfo);
}

void BuildPointCloudComputeProgram(open_cl *OpenCL)
{
    // Compiling the compute program.
    char *ComputeSource = 
    "float3 HSVToRGB(float3 HSV)                                                         \n"
    "{                                                                                   \n"
    "    float3 RGB;                                                                     \n"
    "                                                                                    \n"
    "    int I;                                                                          \n"
    "    float F, P, Q, T;                                                               \n"
    "                                                                                    \n"
    "    float H = HSV.x;                                                                \n"
    "    float S = HSV.y;                                                                \n"
    "    float V = HSV.z;                                                                \n"
    "                                                                                    \n"
    "    if(S == 0)                                                                      \n"
    "    {                                                                               \n"
    "        RGB = (float3){V, V, V};                                                    \n"
    "    }                                                                               \n"
    "    else                                                                            \n"
    "    {                                                                               \n"
    "        H *= 6;                                                                     \n"
    "        I = (int)H;                                                                 \n"
    "        F = H - I;                                                                  \n"
    "        P = V * (1 - S);                                                            \n"
    "        Q = V * (1 - S * F);                                                        \n"
    "        T = V * (1 - S * (1 - F));                                                  \n"
    "        switch (I)                                                                  \n"
    "        {                                                                           \n"
    "            case 1:  RGB = (float3){Q, V, P}; break;                                \n"
    "            case 0:  RGB = (float3){V, T, P}; break;                                \n"
    "            case 2:  RGB = (float3){P, V, T}; break;                                \n"
    "            case 3:  RGB = (float3){P, Q, V}; break;                                \n"
    "            case 4:  RGB = (float3){T, P, V}; break;                                \n"
    "            default: RGB = (float3){V, P, Q}; break;                                \n"
    "        }                                                                           \n"
    "    }                                                                               \n"
    "                                                                                    \n"
    "    return(RGB);                                                                    \n"
    "}                                                                                   \n"
    "                                                                                    \n"
    "__kernel void ComputeKernel(__read_only  image2d_t DepthImage,                      \n"
    "                            __read_only  image2d_t XYMap,                           \n"
    "                            __write_only image2d_t PositionImage,                   \n"
    "                            __write_only image2d_t ColorImage,                      \n"
    "                            float MinDepth,                                         \n"
    "                            float MaxDepth)                                         \n"
    "{                                                                                   \n"
    "    int2 Pixel = { get_global_id(0), get_global_id(1) };                            \n"
    "                                                                                    \n"
    "    float Depth = (float)read_imageui(DepthImage, Pixel).x;                         \n"
    "    float2 XY = read_imagef(XYMap, Pixel).xy;                                       \n"
    "                                                                                    \n"
    "    float W = 1.0f;                                                                 \n"
    "                                                                                    \n"
    "    if(XY.x == 0.0f && XY.y == 0.0f)                                                \n"
    "    {                                                                               \n"
    "        W = 0.0f;                                                                   \n"
    "    }                                                                               \n"
    "                                                                                    \n"
    "    float3 Position = { XY.x * Depth, -XY.y * Depth, -Depth };                      \n"
    "                                                                                    \n"
    "    Position /= 1000.0f;                                                            \n"
    "                                                                                    \n"
    "    float Hue = (-Position.z - MinDepth) / (MaxDepth - MinDepth);                   \n"
    "    Hue = clamp(Hue, 0.0f, 1.0f);                                                   \n"
    "                                                                                    \n"
    "    float Range = 2.0f / 3.0f;                                                      \n"
    "                                                                                    \n"
    "    Hue *= Range;                                                                   \n"
    "    Hue = Range - Hue;                                                              \n"
    "                                                                                    \n"
    "    float3 Color = HSVToRGB((float3){ Hue, 1.0f, 1.0f });                           \n"
    "                                                                                    \n"
    "    write_imagef(PositionImage, Pixel, (float4){ Position, W });                    \n"
    "    write_imagef(ColorImage, Pixel, (float4){ Color, W });                          \n"
    "}                                                                                   \n";
    
    cl_int Result;
    cl_program Program = clCreateProgramWithSource(OpenCL->Context, 1, &ComputeSource, 0, &Result);
    assert(Result == CL_SUCCESS);
    
    #if defined(NDEBUG)
    char *Flags = "-cl-std=CL2.0";
    #else // DEBUG
    char *Flags = "-g -Werror -cl-std=CL2.0";
    #endif
    clBuildProgram(Program, 0, NULL, Flags, NULL, NULL);
    
    cl_build_status BuildStatus;
    clGetProgramBuildInfo(Program, OpenCL->Device, CL_PROGRAM_BUILD_STATUS, sizeof(BuildStatus), &BuildStatus, NULL);
    
    if(BuildStatus != CL_BUILD_SUCCESS)
    {
        size_t LogSize;
        clGetProgramBuildInfo(Program, OpenCL->Device, CL_PROGRAM_BUILD_LOG, 0, NULL, &LogSize);
        
        char *BuildLog = (char *)malloc(LogSize);
        
        clGetProgramBuildInfo(Program, OpenCL->Device, CL_PROGRAM_BUILD_LOG, LogSize, BuildLog, NULL);
        fprintf(stderr, "OPENCL BUILD ERROR\n%s\n", BuildLog);
        
        free(BuildLog);
        
        assert(!"Failed to build the point cloud compute shader.");
    }
    
    // Create Kernel
    OpenCL->PointCloudComputeKernel = clCreateKernel(Program, "ComputeKernel", &Result);
    assert(Result == CL_SUCCESS);
    
    OpenCL->PointCloudComputeProgram = Program;
}

void BuildPipelineProgram(open_cl *OpenCL)
{
    char *PipelineSource = 
    "float4 Mat4Vec4Mul(const float16 Matrix,                                            \n"
    "                   const float4  Vector)                                            \n"
    "{                                                                                   \n"
    "    float4 Result = (float4)0;                                                      \n"
    "                                                                                    \n"
    "    for(int i = 0; i < 4; ++i)                                                      \n"
    "    {                                                                               \n"
    "        for(int j = 0; j < 4; ++j)                                                  \n"
    "        {                                                                           \n"
    "            Result[i] += Matrix[i * 4 + j] * Vector[j];                             \n"
    "        }                                                                           \n"
    "    }                                                                               \n"
    "                                                                                    \n"
    "    return(Result);                                                                 \n"
    "}                                                                                   \n"
    "                                                                                    \n"
    "__kernel void Pipeline(__read_only  image2d_t PositionImage,                        \n"
    "                       __read_only  image2d_t ColorImage,                           \n"
    "                       __write_only image2d_t Framebuffer,                          \n"
    "                       __global uint *DepthBuffer,                                  \n"
    "                       float16 MVP)                                                 \n"
    "{                                                                                   \n"
    "    int2 Pixel = { get_global_id(0), get_global_id(1) };                            \n"
    "                                                                                    \n" 
    "    float4 Position;                                                                \n"
    "    float4 Color = read_imagef(ColorImage, Pixel);                                  \n"
    "    float4 VertexPosition = read_imagef(PositionImage, Pixel);                      \n"
    "                                                                                    \n"
    "    Position = Mat4Vec4Mul(MVP, VertexPosition);                                    \n"
    "                                                                                    \n"
    "    //                                                                              \n"
    "    // Clipping                                                                     \n"
    "    bool X = -Position.w < Position.x && Position.x < Position.w;                   \n"
    "    bool Y = -Position.w < Position.y && Position.y < Position.w;                   \n"
    "    bool Z = -Position.w < Position.z && Position.z < Position.w;                   \n"
    "    if(!X || !Y || !Z) return;                                                      \n"
    "                                                                                    \n"
    "    //                                                                              \n"
    "    // Perspective Division                                                         \n"
    "    float3 NDC;                                                                     \n"
    "    if(Position.w != 0.0f)                                                          \n"
    "    {                                                                               \n"
    "        NDC.x = Position.x / Position.w;                                            \n"
    "        NDC.y = Position.y / Position.w;                                            \n"
    "        NDC.z = Position.z / Position.w;                                            \n"
    "    }                                                                               \n"
    "                                                                                    \n"
    "    //                                                                              \n"
    "    // Viewport Transform                                                           \n"
    "    int Width, Height;                                                              \n"
    "    Width = get_image_width(Framebuffer);                                           \n"
    "    Height = get_image_height(Framebuffer);                                         \n"
    "    int2 ScreenPixel =                                                              \n"
    "    {                                                                               \n"
    "        (int)((NDC.x + 1) * ((Width - 1) / 2)),                                     \n"
    "        (int)((NDC.y + 1) * ((Height - 1) / 2))                                     \n"
    "    };                                                                              \n"
    "    int Index = (ScreenPixel.y * Width + ScreenPixel.x) * 2;                        \n"
    "                                                                                    \n"
    "    float DepthFloat = (NDC.z + 1) / 2;                                             \n"
    "    uint Depth = UINT_MAX * DepthFloat;                                             \n"
    "                                                                                    \n"
    "    bool Processed = false;                                                         \n"
    "    while(!Processed)                                                               \n"
    "    {                                                                               \n"
    "        if(Depth >= DepthBuffer[Index])                                             \n"
    "        {                                                                           \n"
    "            Processed = true;                                                       \n"
    "        }                                                                           \n"
    "        else                                                                        \n"
    "        {                                                                           \n"
    "            uint Acquired = atomic_cmpxchg(&DepthBuffer[Index+1], 0, 1);            \n"
    "            if(Acquired == 0)                                                       \n"
    "            {                                                                       \n"
    "                if(Depth < DepthBuffer[Index])                                      \n"
    "                {                                                                   \n"
    "                    DepthBuffer[Index] = Depth;                                     \n"
    "                    write_imagef(Framebuffer, ScreenPixel, Color);                  \n"
    "                }                                                                   \n"
    "                Processed = true;                                                   \n"
    "                atomic_xchg(&DepthBuffer[Index+1], 0);                              \n"
    "            }                                                                       \n"
    "            barrier(CLK_LOCAL_MEM_FENCE);                                           \n"
    "        }                                                                           \n"
    "    }                                                                               \n"
    "}                                                                                   \n";
    
    cl_int Result;
    cl_program Program = clCreateProgramWithSource(OpenCL->Context, 1, &PipelineSource, 0, &Result);
    assert(Result == CL_SUCCESS);
    
    #if defined(NDEBUG)
    char *Flags = "-cl-std=CL2.0";
    #else // DEBUG
    char *Flags = "-g -Werror -cl-std=CL2.0";
    #endif
    clBuildProgram(Program, 0, NULL, Flags, NULL, NULL);
    
    cl_build_status BuildStatus;
    clGetProgramBuildInfo(Program, OpenCL->Device, CL_PROGRAM_BUILD_STATUS, sizeof(BuildStatus), &BuildStatus, NULL);
    
    if(BuildStatus != CL_BUILD_SUCCESS)
    {
        size_t LogSize;
        clGetProgramBuildInfo(Program, OpenCL->Device, CL_PROGRAM_BUILD_LOG, 0, NULL, &LogSize);
        
        char *BuildLog = (char *)malloc(LogSize);
        
        clGetProgramBuildInfo(Program, OpenCL->Device, CL_PROGRAM_BUILD_LOG, LogSize, BuildLog, NULL);
        fprintf(stderr, "OPENCL BUILD ERROR\n%s\n", BuildLog);
        
        free(BuildLog);
        
        assert(!"Failed to build the point cloud compute shader.");
    }
    
    // Create Kernel
    OpenCL->PipelineKernel = clCreateKernel(Program, "Pipeline", &Result);
    assert(Result == CL_SUCCESS);
    
    OpenCL->PipelineProgram = Program;
}

bool StringsAreEqual(size_t ALength, char *A, char *B)
{
    bool Result = false;
    
    if(B)
    {
        char *At = B;
        for(size_t Index = 0; Index < ALength; ++Index, ++At)
        {
            if(B[Index] == '\0' || (A[Index] != B[Index]))
            {
                return(false);
            }
        }
        
        Result = (*At == 0);
    }
    else
    {
        Result = (ALength == 0);
    }
    
    return(Result);
}

open_cl *OpenCLInit(uint32_t DepthMapWidth, uint32_t DepthMapHeight, uint32_t WindowWidth, uint32_t WindowHeight, uint16_t *DepthMap, v2f *XYMap, os_specifics *OS, cl_GLuint GLFramebuffer)
{
    open_cl *OpenCL = (open_cl *)malloc(sizeof(open_cl));
    
    cl_int Result;
    
    cl_uint NumPlatforms;
    cl_platform_id Platforms[16];
    
    clGetPlatformIDs(16, Platforms, &NumPlatforms);
    assert(16 >= NumPlatforms && NumPlatforms > 0);

    for(cl_uint PlatformIndex = 0; PlatformIndex < NumPlatforms; ++PlatformIndex)
    {   
        cl_platform_id Platform = Platforms[PlatformIndex];
        
        // Checking for extension support
        size_t ExtensionStringLength;
        clGetPlatformInfo(Platform, CL_PLATFORM_EXTENSIONS, 0, NULL, &ExtensionStringLength);
        
        char *Extensions = (char *)malloc(ExtensionStringLength);
        
        clGetPlatformInfo(Platform, CL_PLATFORM_EXTENSIONS, ExtensionStringLength, Extensions, NULL);
        
        char *Start = Extensions;
        while(*Start)
        {
            while(*Start == ' ') ++Start;
            char *End = Start;
            while(*End && *End != ' ') ++End;
            
            size_t Count = End - Start;
            
            if(0) {}
            else if(StringsAreEqual(Count, Start, "cl_khr_gl_sharing")) 
            {
                OpenCL->SupportsGLContextSharing = true;
                
                clGetGLContextInfoKHR_fn clGetGLContextInfoKHR = (clGetGLContextInfoKHR_fn)clGetExtensionFunctionAddressForPlatform(Platform, "clGetGLContextInfoKHR");
                
                cl_context_properties ContextProperties[] = 
                {
                    #if defined(_WIN32)
                        CL_GL_CONTEXT_KHR,   (cl_context_properties)OS->GLContext,
                        CL_WGL_HDC_KHR,      (cl_context_properties)OS->DeviceContext,
                        CL_CONTEXT_PLATFORM, (cl_context_properties)Platform,
                        0
                    #elif defined(__linux__)
                        CL_GL_CONTEXT_KHR,   (cl_context_properties)OS->GLContext,
                        CL_GLX_DISPLAY_KHR,  (cl_context_properties)OS->X11Display,
                        CL_CONTEXT_PLATFORM, (cl_context_properties)Platform,
                        0
                    #endif
                };
                
                cl_device_id Device;
                clGetGLContextInfoKHR(ContextProperties, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, sizeof(cl_device_id), &Device, NULL);
                
                OpenCL->Platform = Platform;
                OpenCL->Device = Device;
                
                goto PlatformFound;
            }
            
            Start = End;
        }
        
        free(Extensions);
    }
    
PlatformFound:
    
    if(OpenCL->SupportsGLContextSharing == false)
    {
        free(OpenCL);
        exit(-1);
    }
    
#if 1
    // Outputting name and vendor of the chosen platform and device to the console.
    char PlatformName[256], PlatformVendor[256];
    clGetPlatformInfo(OpenCL->Platform, CL_PLATFORM_NAME, 256, PlatformName, NULL);
    clGetPlatformInfo(OpenCL->Platform, CL_PLATFORM_VENDOR, 256, PlatformVendor, NULL);
    
    char DeviceName[256], DeviceVendor[256], DeviceVersion[256], DeviceOpenCLCVersion[256];
    clGetDeviceInfo(OpenCL->Device, CL_DEVICE_NAME, 256, DeviceName, NULL);
    clGetDeviceInfo(OpenCL->Device, CL_DEVICE_NAME, 256, DeviceVendor, NULL);
    clGetDeviceInfo(OpenCL->Device, CL_DEVICE_VERSION, 256, DeviceVersion, NULL);
    clGetDeviceInfo(OpenCL->Device, CL_DEVICE_OPENCL_C_VERSION, 256, DeviceOpenCLCVersion, NULL);
    
    printf("Platform Name    : %s\n", PlatformName);
    printf("Platform Vendor  : %s\n", PlatformVendor);
    printf("Device Name      : %s\n", DeviceName);
    printf("Device Vendor    : %s\n", DeviceVendor);
    printf("Driver Version   : %s\n", DeviceVersion);
    printf("OpenCL C Version : %s\n", DeviceOpenCLCVersion);
    printf("\n");
#endif
        
    // Creating an OpenCL context.
    cl_context_properties ContextProperties[] = 
    {
        #if defined(_WIN32)
            CL_GL_CONTEXT_KHR,   (cl_context_properties)OS->GLContext,
            CL_WGL_HDC_KHR,      (cl_context_properties)OS->DeviceContext,
            CL_CONTEXT_PLATFORM, (cl_context_properties)OpenCL->Platform,
            0
        #elif defined(__linux__)
            CL_GL_CONTEXT_KHR,   (cl_context_properties)OS->GLContext,
            CL_GLX_DISPLAY_KHR,  (cl_context_properties)OS->X11Display,
            CL_CONTEXT_PLATFORM, (cl_context_properties)OpenCL->Platform,
            0
        #endif
    };
    
    OpenCL->Context = clCreateContext(ContextProperties, 1, &OpenCL->Device, ContextCallback, NULL, &Result);
    if(Result == CL_SUCCESS)
    {
        cl_queue_properties CommandQueueProperties[] = 
        {
            CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE,
            0
        };
        
        OpenCL->CommandQueue = clCreateCommandQueueWithProperties(OpenCL->Context, OpenCL->Device, CommandQueueProperties, &Result);
        if(Result == CL_SUCCESS)
        {
            BuildPointCloudComputeProgram(OpenCL);
            BuildPipelineProgram(OpenCL);
            
            // Creating the framebuffer from the OpenGL texture.
            OpenCL->Framebuffer = clCreateFromGLTexture(OpenCL->Context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, GLFramebuffer, &Result);
            assert(Result == CL_SUCCESS);

            OpenCL->FramebufferWidth = WindowWidth;
            OpenCL->FramebufferHeight = WindowHeight;

            // Creating the depth buffer.
            OpenCL->DepthBuffer = clCreateBuffer(OpenCL->Context, CL_MEM_READ_WRITE, WindowWidth * WindowHeight * sizeof(unsigned int) * 2, NULL, &Result);
            assert(Result == CL_SUCCESS);
            
            // Creating the depth map image.
            cl_image_desc DepthMapImageDescriptor = {0};
            DepthMapImageDescriptor.image_type = CL_MEM_OBJECT_IMAGE2D;
            DepthMapImageDescriptor.image_width = DepthMapWidth;
            DepthMapImageDescriptor.image_height = DepthMapHeight;
            
            cl_image_format DepthMapImageFormat = { CL_R, CL_UNSIGNED_INT16 };
            
            OpenCL->DepthMapImage = clCreateImage(OpenCL->Context, CL_MEM_READ_WRITE, &DepthMapImageFormat, &DepthMapImageDescriptor, NULL, &Result);
            assert(Result == CL_SUCCESS);
            
            // Creating the xy map image.
            cl_image_desc XYMapImageDescriptor = {0};
            XYMapImageDescriptor.image_type = CL_MEM_OBJECT_IMAGE2D;
            XYMapImageDescriptor.image_width = DepthMapWidth;
            XYMapImageDescriptor.image_height = DepthMapHeight;
            XYMapImageDescriptor.image_row_pitch = 2 * sizeof(float) * DepthMapWidth;
            
            cl_image_format XYMapImageFormat = { CL_RG, CL_FLOAT };
            
            OpenCL->XYMapImage = clCreateImage(OpenCL->Context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, &XYMapImageFormat, &XYMapImageDescriptor, XYMap, &Result);
            assert(Result == CL_SUCCESS);
            
            // Creating the position image/texture.
            cl_image_desc PositionImageDescriptor = {0};
            PositionImageDescriptor.image_type = CL_MEM_OBJECT_IMAGE2D;
            PositionImageDescriptor.image_width = DepthMapWidth;
            PositionImageDescriptor.image_height = DepthMapHeight;
            
            cl_image_format PositionImageFormat = { CL_RGBA, CL_FLOAT };
            
            OpenCL->PositionImage = clCreateImage(OpenCL->Context, CL_MEM_READ_WRITE, &PositionImageFormat, &PositionImageDescriptor, NULL, &Result);
            assert(Result == CL_SUCCESS);
            
            // Creating the color image/texture.
            cl_image_desc ColorImageDescriptor = {0};
            ColorImageDescriptor.image_type = CL_MEM_OBJECT_IMAGE2D;
            ColorImageDescriptor.image_width = DepthMapWidth;
            ColorImageDescriptor.image_height = DepthMapHeight;
            
            cl_image_format ColorImageFormat = { CL_RGBA, CL_FLOAT };
            
            OpenCL->ColorImage = clCreateImage(OpenCL->Context, CL_MEM_READ_WRITE, &ColorImageFormat, &ColorImageDescriptor, NULL, &Result);
            assert(Result == CL_SUCCESS);
        }
    }
    
    return(OpenCL);
}

void OpenCLRelease(open_cl *OpenCL)
{
    clReleaseKernel(OpenCL->PointCloudComputeKernel);

    clReleaseProgram(OpenCL->PointCloudComputeProgram);
    
    clReleaseMemObject(OpenCL->ColorImage);
    clReleaseMemObject(OpenCL->PositionImage);
    clReleaseMemObject(OpenCL->XYMapImage);
    clReleaseMemObject(OpenCL->DepthMapImage);
    clReleaseMemObject(OpenCL->DepthBuffer);
    clReleaseMemObject(OpenCL->Framebuffer);
    
    clReleaseCommandQueue(OpenCL->CommandQueue);
    
    clReleaseContext(OpenCL->Context);
}

double GetTimeElapsed(cl_event First, cl_event Last) 
{
    cl_ulong TimeStart, TimeEnd; // Nanoseconds
    cl_int Result = 0;
    Result |= clGetEventProfilingInfo(First, CL_PROFILING_COMMAND_START, sizeof(TimeStart), &TimeStart, NULL);
    Result |= clGetEventProfilingInfo(Last, CL_PROFILING_COMMAND_COMPLETE, sizeof(TimeEnd), &TimeEnd, NULL);
    assert(Result == CL_SUCCESS);
    return (TimeEnd - TimeStart) / 1e6; // Milliseconds
}

// DONT USE CALLBACK DO IT IN THE FUNCTION USE THE FIRST AND LAST EVENT OF BOTH COMPUTE AND TEXTURE 
// START OF FIRST AND COMPLETE OF LAST EVENT, THEN SUBTRACT; SHOULDNT BE MUCH CPU WAIT TIME

void OpenCLRenderToTexture(open_cl *OpenCL, float MinDepth, float MaxDepth, uint16_t *DepthMap, uint32_t DepthMapWidth, uint32_t DepthMapHeight, view_control *Control, bool DepthMapUpdate)
{
    cl_int Result = 0;

    static unsigned int FrameCount = 0;
    unsigned int QueryIndex = FrameCount % QUERY_COUNT;

    static average AvgComputeTimeGPU = {.CountTo = 1000, .Msg = "Compute GPU", "ms"};
    static average AvgRenderTimeGPU = {.CountTo = 1000, .Msg = "Render GPU", "ms"};

    size_t GlobalWorkSize[] = { DepthMapWidth, DepthMapHeight };
    size_t *LocalWorkSize = NULL;

    cl_event WroteToDepthMapImageEvent = 0;
    cl_event ComputedPointCloud = 0;

    // Writing the depth map data to the opencl image.
    size_t Origin[] = { 0, 0, 0 };
    size_t DepthMapRegion[] = { DepthMapWidth, DepthMapHeight, 1 };

    Result = clEnqueueWriteImage(
        OpenCL->CommandQueue, 
        OpenCL->DepthMapImage, 
        CL_FALSE, 
        Origin, DepthMapRegion, 
        DepthMapWidth * sizeof(DepthMap[0]), 0, 
        DepthMap, 
        0, NULL, &WroteToDepthMapImageEvent);
    assert(Result == CL_SUCCESS);
    
    // Set Kernel Arguments and Enqueue the Kernel in the command queue.
    Result = 0;
    Result |= clSetKernelArg(OpenCL->PointCloudComputeKernel, 0, sizeof(cl_mem), &OpenCL->DepthMapImage);
    Result |= clSetKernelArg(OpenCL->PointCloudComputeKernel, 1, sizeof(cl_mem), &OpenCL->XYMapImage);
    Result |= clSetKernelArg(OpenCL->PointCloudComputeKernel, 2, sizeof(cl_mem), &OpenCL->PositionImage);
    Result |= clSetKernelArg(OpenCL->PointCloudComputeKernel, 3, sizeof(cl_mem), &OpenCL->ColorImage);
    Result |= clSetKernelArg(OpenCL->PointCloudComputeKernel, 4, sizeof(float), &MinDepth);
    Result |= clSetKernelArg(OpenCL->PointCloudComputeKernel, 5, sizeof(float), &MaxDepth);
    assert(Result == CL_SUCCESS);
    
    //
    // COMPUTING POINT CLOUD
    Result = clEnqueueNDRangeKernel(
        OpenCL->CommandQueue, 
        OpenCL->PointCloudComputeKernel, 
        2, 
        NULL, GlobalWorkSize, LocalWorkSize, 
        1, &WroteToDepthMapImageEvent, 
        &ComputedPointCloud);
    assert(Result == CL_SUCCESS);
    
    // Acquire GL Objects
    cl_event AcquiredGLFramebuffer;
    Result = clEnqueueAcquireGLObjects(OpenCL->CommandQueue, 1, &OpenCL->Framebuffer, 0, NULL, &AcquiredGLFramebuffer);
    assert(Result == CL_SUCCESS);
        
    float Color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    unsigned int DepthPlusLock[] = { UINT_MAX, 0 };

    size_t FramebufferOrigin[] = { 0, 0, 0 };
    size_t FramebufferRegion[] = { OpenCL->FramebufferWidth, OpenCL->FramebufferHeight, 1 };
    
    cl_event FilledFramebuffer;
    cl_event FilledDepthBuffer;

    Result = 0;
    Result |= clEnqueueFillImage(OpenCL->CommandQueue, OpenCL->Framebuffer, Color, FramebufferOrigin, FramebufferRegion, 1, &AcquiredGLFramebuffer, &FilledFramebuffer);
    Result |= clEnqueueFillBuffer(OpenCL->CommandQueue, OpenCL->DepthBuffer, DepthPlusLock, 2 * sizeof(unsigned int), 0, OpenCL->FramebufferWidth * OpenCL->FramebufferHeight * sizeof(unsigned int) * 2, 0, NULL, &FilledDepthBuffer);
    assert(Result == CL_SUCCESS);
    
    Result = 0;
    Result |= clSetKernelArg(OpenCL->PipelineKernel, 0, sizeof(cl_mem), &OpenCL->PositionImage);
    Result |= clSetKernelArg(OpenCL->PipelineKernel, 1, sizeof(cl_mem), &OpenCL->ColorImage);
    Result |= clSetKernelArg(OpenCL->PipelineKernel, 2, sizeof(cl_mem), &OpenCL->Framebuffer);
    Result |= clSetKernelArg(OpenCL->PipelineKernel, 3, sizeof(cl_mem), &OpenCL->DepthBuffer);
    assert(Result == CL_SUCCESS);
    
    mat4 Model = Control->model;
    mat4 View  = look_at(Control->position, v3f_add(Control->position, Control->forward), Control->up);
    mat4 Proj  = perspective(Control->fov, (float)OpenCL->FramebufferWidth / (float)OpenCL->FramebufferHeight, 0.1f, 100.0f);
    mat4 MVP   = mat4_mul(Proj, mat4_mul(View, Model));
    Result = clSetKernelArg(OpenCL->PipelineKernel, 4, sizeof(float) * 16, (void *)MVP.p);
    assert(Result == CL_SUCCESS);
        
    cl_event Events[] = { FilledFramebuffer, FilledDepthBuffer, ComputedPointCloud };

    //
    // RENDERING TO TEXTURE
    cl_event PipelineDoneEvent;
    Result = clEnqueueNDRangeKernel(
        OpenCL->CommandQueue, 
        OpenCL->PipelineKernel, 
        2, 
        NULL, GlobalWorkSize, LocalWorkSize, 
        Size(Events), Events, 
        &PipelineDoneEvent);
    assert(Result == CL_SUCCESS);
    
    // release OpenGL objects
    cl_event GLObjectsReleasedEvent;
    Result = clEnqueueReleaseGLObjects(OpenCL->CommandQueue, 1, &OpenCL->Framebuffer, 1, &PipelineDoneEvent, &GLObjectsReleasedEvent);
    assert(Result == CL_SUCCESS);

    OpenCL->FirstAndLastEvent[0][QueryIndex][0] = WroteToDepthMapImageEvent;
    OpenCL->FirstAndLastEvent[0][QueryIndex][1] = ComputedPointCloud;

    OpenCL->FirstAndLastEvent[1][QueryIndex][0] = FilledFramebuffer;
    OpenCL->FirstAndLastEvent[1][QueryIndex][1] = PipelineDoneEvent;

    unsigned int PrevQueryIndex = (QueryIndex + 1) % QUERY_COUNT;
    if (FrameCount >= 4)
    {
        cl_int EventStatus;

        Result = clGetEventInfo(OpenCL->FirstAndLastEvent[0][PrevQueryIndex][1], CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &EventStatus, NULL);
        assert(Result == CL_SUCCESS);
        if (EventStatus == CL_COMPLETE)
        {
            double TimeElapsed = GetTimeElapsed(OpenCL->FirstAndLastEvent[0][PrevQueryIndex][0], OpenCL->FirstAndLastEvent[0][PrevQueryIndex][1]);
            PrintAverage(&AvgComputeTimeGPU, TimeElapsed);
        }
        else
        {
            assert(false);
        }

        clReleaseEvent(OpenCL->FirstAndLastEvent[0][PrevQueryIndex][0]);
        clReleaseEvent(OpenCL->FirstAndLastEvent[0][PrevQueryIndex][1]);

        Result = clGetEventInfo(OpenCL->FirstAndLastEvent[1][PrevQueryIndex][1], CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(cl_int), &EventStatus, NULL);
        assert(Result == CL_SUCCESS);
        if (EventStatus == CL_COMPLETE)
        {
            double TimeElapsed = GetTimeElapsed(OpenCL->FirstAndLastEvent[1][PrevQueryIndex][0], OpenCL->FirstAndLastEvent[1][PrevQueryIndex][1]);
            PrintAverage(&AvgRenderTimeGPU, TimeElapsed);
        }
        else
        {
            assert(false);
        }

        clReleaseEvent(OpenCL->FirstAndLastEvent[1][PrevQueryIndex][0]);
        clReleaseEvent(OpenCL->FirstAndLastEvent[1][PrevQueryIndex][1]);
    }
        
    // release events
    clReleaseEvent(GLObjectsReleasedEvent);
    clReleaseEvent(FilledDepthBuffer);
    clReleaseEvent(AcquiredGLFramebuffer);

    FrameCount++;
}
