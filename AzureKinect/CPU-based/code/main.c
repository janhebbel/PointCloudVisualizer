#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "input.c"
#include "k4a.c"
#include "linalg.h"

typedef struct 
{
    BITMAPINFO Info;
    uint32_t *Memory;
    int Size;
    int Width;
    int Height;
    int Stride;
    int BytesPerPixel;
} framebuffer;

typedef struct
{
	float *Memory;
	uint32_t Width;
	uint32_t Height;
} depth_buffer;

typedef struct
{
	uint32_t w;
	uint32_t h;
} dimensions;

typedef struct
{
    mat4 model;
    v3f position;
    v3f forward;
    v3f up;
    float fov;
    float speed;
    float sensitivity;
} view_control;

typedef struct
{
    float xyz[3];
    float rgb[3];
} color_point;

typedef struct
{
	v4f Position;
	v3f Color;
} vertex_out;

typedef vertex_out vertex_program(color_point In, mat4 MVP);
typedef v3f        pixel_program(v3f HSV);

typedef struct
{
	dimensions ViewportDimensions;
	
	vertex_program *VertexProgram;
	pixel_program *PixelProgram;
} graphics_pipeline;

struct scroll_update { 
    double yoffset;
    int    updated;
};

static struct scroll_update global_scroll_update;
static bool GlobalRunning = false;

static framebuffer *CreateFramebuffer(int Width, int Height, int BytesPerPixel)
{
    framebuffer *Framebuffer = (framebuffer *)VirtualAlloc(NULL, sizeof(framebuffer), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    Framebuffer->Info.bmiHeader.biSize = sizeof(Framebuffer->Info.bmiHeader);
    Framebuffer->Info.bmiHeader.biWidth = Width;
    Framebuffer->Info.bmiHeader.biHeight = -Height; // top down dib (origin at top left corner)
    Framebuffer->Info.bmiHeader.biPlanes = 1;
    Framebuffer->Info.bmiHeader.biBitCount = 32;
    Framebuffer->Info.bmiHeader.biCompression = BI_RGB;

    int Size = BytesPerPixel * Width * Height;
    Framebuffer->Memory = (uint32_t *)VirtualAlloc(NULL, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    Framebuffer->Width = Width;
    Framebuffer->Height = Height;
    Framebuffer->Size = Size;
    Framebuffer->Stride = Width * BytesPerPixel;

    return(Framebuffer);
}

static void DisplayFramebuffer(framebuffer *Framebuffer, HDC WindowDC, int Width, int Height)
{
	StretchDIBits(
		WindowDC,
		0, 0, Width, Height, // destination
        0, 0, Framebuffer->Width, Framebuffer->Height, // source
        Framebuffer->Memory,
        &Framebuffer->Info,
        DIB_RGB_COLORS, SRCCOPY);
}

static void ClearFramebuffer(framebuffer *Framebuffer, float Red, float Green, float Blue, float Alpha)
{
	uint32_t R = (uint32_t)(0xFF * Red);
	uint32_t G = (uint32_t)(0xFF * Green);
	uint32_t B = (uint32_t)(0xFF * Blue);
	uint32_t A = (uint32_t)(0xFF * Alpha);
	uint32_t Color = A << 24 | R << 16 | G << 8 | B << 0;
	
	uint32_t PixelCount = Framebuffer->Width * Framebuffer->Height;
	
	for(size_t Index = 0; Index < PixelCount; ++Index)
	{
		Framebuffer->Memory[Index] = Color;
	}
}

static depth_buffer *CreateDepthBuffer(uint32_t Width, uint32_t Height)
{
	depth_buffer *DepthBuffer = (depth_buffer *)VirtualAlloc(NULL, sizeof(depth_buffer), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	
	uint32_t PixelCount = Width * Height;
	DepthBuffer->Memory = (float *)VirtualAlloc(NULL, 1280 * 720 * sizeof(float), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	DepthBuffer->Width = Width;
	DepthBuffer->Height = Height;
	
	return(DepthBuffer);
}

static void ClearDepthBuffer(depth_buffer *DepthBuffer)
{
	uint32_t PixelCount = DepthBuffer->Width * DepthBuffer->Height;
	for(size_t Index = 0; Index < PixelCount; ++Index)
	{
		DepthBuffer->Memory[Index] = 0;
	}
}

static graphics_pipeline *CreateGraphicsPipeline(uint32_t ViewportWidth, uint32_t ViewportHeight, vertex_program *VertexProgram, pixel_program *PixelProgram)
{
	graphics_pipeline *Pipeline = (graphics_pipeline *)VirtualAlloc(NULL, sizeof(graphics_pipeline), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	
	dimensions Dimensions = { ViewportWidth, ViewportHeight };
	
	Pipeline->ViewportDimensions = Dimensions;
	Pipeline->VertexProgram = VertexProgram;
	Pipeline->PixelProgram = PixelProgram;
	
	return(Pipeline);
}

static void ToggleFullscreen(HWND Window)
{
    static DWORD PrevWindowStyle;
    static WINDOWPLACEMENT PrevWindowPlacement = { sizeof(PrevWindowPlacement) };
    
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    DWORD FullscreenStyle = WS_POPUP|WS_VISIBLE;
    
    RECT WindowRect;
    GetWindowRect(Window, &WindowRect);
    
    MONITORINFO MonitorInfo = { sizeof(MonitorInfo) };
    GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo);
    
    bool Windowed = !(MonitorInfo.rcMonitor.left == WindowRect.left && MonitorInfo.rcMonitor.right == WindowRect.right && 
                      MonitorInfo.rcMonitor.top == WindowRect.top && MonitorInfo.rcMonitor.bottom == WindowRect.bottom);
    
    if(Windowed)
    {
        PrevWindowStyle = Style;
        GetWindowPlacement(Window, &PrevWindowPlacement);
        
        SetWindowLong(Window, GWL_STYLE, FullscreenStyle);;
        SetWindowPos(Window, HWND_TOP, 
                     MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                     MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                     MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                     SWP_NOOWNERZORDER|SWP_FRAMECHANGED);
    }
    else
    {
        SetWindowLong(Window, GWL_STYLE, PrevWindowStyle);
        SetWindowPlacement(Window, &PrevWindowPlacement);
        SetWindowPos(Window, NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_FRAMECHANGED);
    }
}

static POINT GlobalInitialCursorPos;

LRESULT CALLBACK MainWndProc(HWND Window, UINT Message, WPARAM wParam, LPARAM lParam)
{
    LRESULT lResult = 1;
    
    switch(Message)
    {
        case WM_ACTIVATEAPP:
        {
            ClearKeyboardState();
            break;
        }
        
        case WM_CLOSE:
        {
            GlobalRunning = false;
            break;
        }
        
        case WM_DESTROY:
        {
            GlobalRunning = false;
            break;
        }
        
        case WM_QUIT:
        {
            GlobalRunning = false;
            break;
        }
        
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            int Key = LOWORD(wParam);
            
            b32 WasDown = (lParam & (1 << 30)) != 0;
            b32 Down    = (lParam & (1 << 31)) == 0;
            
            if(WasDown != Down)
            {
                b32 AltDown = (lParam & (1 << 29)) != 0;
                
                if(Key == VK_F11 && Down)
                {
                    ToggleFullscreen(Window);
                }
                
                if(Key == VK_F4 && AltDown && Down)
                {
                    GlobalRunning = false;
                }
                
                KeyEvent(Key, Down);
            }
            
            break;
        }
		
		case WM_RBUTTONDOWN:
		{
			ShowCursor(FALSE);
			GetCursorPos(&GlobalInitialCursorPos);
			break;
		}
		
		case WM_RBUTTONUP:
		{
			ShowCursor(TRUE);
			break;			
		}
		
		case WM_MOUSEWHEEL:
		{
			global_scroll_update.yoffset = -(GET_WHEEL_DELTA_WPARAM(wParam) / (double)WHEEL_DELTA);
			global_scroll_update.updated = 1;
			break;
		}
        
        default:
        {
            lResult = DefWindowProc(Window, Message, wParam, lParam);
            break;
        }
    }
    
    return(lResult);
}

static void ProcessWindowMessages(void)
{
    MSG Message;
    while(PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
}

static double GetTimeInSeconds(void)
{
    static int64_t PerformanceFrequency;
    if(!PerformanceFrequency)
    {
        LARGE_INTEGER PerformanceFrequencyResult;
        QueryPerformanceFrequency(&PerformanceFrequencyResult);
        PerformanceFrequency = PerformanceFrequencyResult.QuadPart;
    }
    
    LARGE_INTEGER PerformanceCounterResult;
    QueryPerformanceCounter(&PerformanceCounterResult);
    int64_t PerformanceCounter = PerformanceCounterResult.QuadPart;
    
    double TimeInSeconds = (double)PerformanceCounter / (double)PerformanceFrequency;
    
    return(TimeInSeconds);
}

static void HandleInput(HWND Window, view_control *Control, float DeltaTime)
{
	// left mouse button pressed
    if((GetKeyState(VK_RBUTTON) & 0x8000) != 0) 
    {
		double xpos, ypos;
				
		POINT CursorPosition;
		GetCursorPos(&CursorPosition);
		xpos = (double)CursorPosition.x;
		ypos = (double)CursorPosition.y;
		
		float dx = (float)(xpos - GlobalInitialCursorPos.x) * Control->sensitivity;
		float dy = -(float)(ypos - GlobalInitialCursorPos.y) * Control->sensitivity;
		
		if(dx || dy)
		{
			SetCursorPos(GlobalInitialCursorPos.x, GlobalInitialCursorPos.y);
		}
		
        static float yaw = -0.25f;
        static float pitch = 0.0f;

        yaw += dx;
        pitch += dy;
		
        if(pitch > 0.245f) pitch = 0.245f;
        else if(pitch < -0.245f) pitch = -0.245f;

        Control->forward.x = linalg_cos(yaw) * linalg_cos(pitch);
        Control->forward.y = linalg_sin(pitch);
        Control->forward.z = linalg_sin(yaw) * linalg_cos(pitch);
        Control->forward = v3f_normalize(Control->forward);

        v3f add = {0};
        
        if(IsDown('W'))
        {
            add = v3f_add(add, Control->forward);
        }
        if(IsDown('A'))
        {
            add = v3f_sub(add, v3f_normalize(v3f_cross(Control->forward, Control->up)));
        }
        if(IsDown('S'))
        {
            add = v3f_sub(add, Control->forward);
        }
        if(IsDown('D'))
        {
            add = v3f_add(add, v3f_normalize(v3f_cross(Control->forward, Control->up)));
        }

        Control->position = v3f_add(Control->position, v3f_scale(add, Control->speed * DeltaTime));
    }

    if(global_scroll_update.updated)
    {
        float dfov = (float)global_scroll_update.yoffset / 100.0f;
        float new_fov = Control->fov + dfov;
        if(new_fov > 0.0f && new_fov < 0.4f)
        {
            Control->fov = new_fov;
        }
        
        global_scroll_update.updated = 0;
    }
}

typedef struct s_timer {
    const int FramesToSumUp;
    int Count;
    float Acc;
} timer;

static void PrintAverageTime(timer *Timer, float DeltaTime)
{
    if(Timer->Count == Timer->FramesToSumUp)
    {
        float AvgFrameTime = Timer->Acc / (float)Timer->FramesToSumUp;
        printf("%f ms\n", AvgFrameTime * 1000.0f);
        Timer->Acc = 0;
        Timer->Count = 0;
    }
    else
    {
        Timer->Acc += DeltaTime;
        Timer->Count++;
    }
}

static void calculate_point_cloud(color_point *VertexArray, uint32_t *VertexCount, v2f *xy_map, uint16_t *depth_map, int depth_map_count)
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

            VertexArray[insert_index++] = point;
        }
    }

    *VertexCount = insert_index;
}

static bool ClipCondition(v4f P)
{
	bool X = -P.w < P.x && P.x < P.w;
	bool Y = -P.w < P.y && P.y < P.w;
	bool Z = -P.w < P.z && P.z < P.w;
	return(!(X && Y && Z));
}

static void ProcessVertices(color_point *VertexArray, uint32_t VertexCount, graphics_pipeline *Pipeline, framebuffer *Framebuffer, depth_buffer *DepthBuffer, mat4 MVP)
{
	for(uint32_t Index = 0; Index < VertexCount; ++Index)
	{
		color_point Vertex = VertexArray[Index];
		
		// Per Vertex Operations (LOCAL SPACE (=> WORLD SPACE => VIEW SPACE) => CLIP SPACE)
		vertex_out VertexOut = Pipeline->VertexProgram(Vertex, MVP); 
		
		// Clipping
		if(ClipCondition(VertexOut.Position))
		{
			continue;
		}
		
		// Perspective Division (CLIP SPACE => NORMALIZED DEVICE COORDINATES)
		v3f NDC;
		if(VertexOut.Position.w != 0.0f)
		{
			NDC.x = VertexOut.Position.x / VertexOut.Position.w;
			NDC.y = -VertexOut.Position.y / VertexOut.Position.w;
			NDC.z = VertexOut.Position.z / VertexOut.Position.w;
		}
		else
		{
			NDC.x = VertexOut.Position.x;
			NDC.y = VertexOut.Position.y;
			NDC.z = VertexOut.Position.z;
		}
		
		uint32_t Width = Pipeline->ViewportDimensions.w;
		uint32_t Height = Pipeline->ViewportDimensions.h;
		
		// Viewport Transform (NORMALIZED DEVICE COORDINATES => SCREEN COORDINATES)
		v2u ViewportPosition = 
		{ 
			(uint32_t)(floor(Width / 2 * NDC.x) + Width / 2),
			(uint32_t)(floor(Height / 2 * NDC.y) + Height / 2),
			/* (int)((Far - Near) / 2.0f * NDC.z + (Far + Near) / 2.0f) */
		};
		
		// Per Pixel Operations
		v3f Color = Pipeline->PixelProgram(VertexOut.Color);
		
		uint32_t Alpha = 0xFF;
		uint32_t Red   = (uint32_t)(0xFF * Color.x);
		uint32_t Green = (uint32_t)(0xFF * Color.y);
		uint32_t Blue  = (uint32_t)(0xFF * Color.z);
		
		size_t FramebufferIndex = ViewportPosition.y * Width + ViewportPosition.x;
		
		// Occlusion Culling
		float Depth = (NDC.z + 1) / 2;
		if(DepthBuffer->Memory[FramebufferIndex] != 0 && Depth >= DepthBuffer->Memory[FramebufferIndex])
		{
			continue;
		}
		
		Framebuffer->Memory[FramebufferIndex] = Alpha << 24 | Red << 16 | Green << 8 | Blue << 0;
		DepthBuffer->Memory[FramebufferIndex] = Depth; // Convert from range -1..1 to 0..1
	}
}

vertex_out VertexProgram(color_point In, mat4 MVP)
{
	vertex_out Out;
	
	Out.Position = mat4_mul_v4f(MVP, (v4f){In.xyz[0], In.xyz[1], In.xyz[2], 1.0f});
	Out.Color = (v3f){In.rgb[0], In.rgb[1], In.rgb[2]};
	
	return(Out);
}

v3f HSVToRGB(v3f HSV) 
{
	v3f RGB;
	
    int I;
    float F, P, Q, T;
	
	float H = HSV.x;
	float S = HSV.y;
	float V = HSV.z;
	
    if (S == 0) // No saturation => grayscale; V == lightness/darkness
	{
        RGB = (v3f){V, V, V};
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
			case 0:  RGB = (v3f){V, T, P}; break;
			case 1:  RGB = (v3f){Q, V, P}; break;
			case 2:  RGB = (v3f){P, V, T}; break;
			case 3:  RGB = (v3f){P, Q, V}; break;
			case 4:  RGB = (v3f){T, P, V}; break;
			default: RGB = (v3f){V, P, Q}; break;
		}
	}
	
	return(RGB);
}


v3f PixelProgram(v3f HSV)
{
	v3f RGB;
	
	RGB = HSVToRGB(HSV);
	
	return(RGB);
}

int main(void)
{
    WNDCLASS WindowClass = {0};
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc = MainWndProc;
    WindowClass.hInstance = GetModuleHandle(NULL);
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    // WindowClass.hIcon;
    WindowClass.lpszClassName = L"CPU-based";

    if(RegisterClass(&WindowClass))
    {
        DWORD WindowStyle = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;
        DWORD ExWindowStyle = 0;
        
        RECT Rect = {0};
        Rect.left = 0;
        Rect.right = 1280;
        Rect.top = 0;
        Rect.bottom = 720;
        AdjustWindowRectEx(&Rect, WindowStyle, 0, ExWindowStyle);
        
        HWND Window = CreateWindowEx(0,
                                     WindowClass.lpszClassName,
                                     L"CPU-based",
                                     WindowStyle,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     Rect.right - Rect.left,
                                     Rect.bottom - Rect.top,
                                     NULL,
                                     NULL,
                                     WindowClass.hInstance,
                                     NULL);
        
        if(Window)
        {
            HDC WindowDC = GetDC(Window);

            framebuffer  *Framebuffer = CreateFramebuffer(1280, 720, 4);
			depth_buffer *DepthBuffer = CreateDepthBuffer(1280, 720);
			graphics_pipeline *Pipeline = CreateGraphicsPipeline(1280, 720, VertexProgram, PixelProgram);
			
			camera_config Config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
			Config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
			Config.camera_fps = K4A_FRAMES_PER_SECOND_30;
            Config.synchronized_images_only = false;
			
			tof_camera Camera_ = camera_init(&Config);
			tof_camera *Camera = &Camera_;
			
			if(Camera->device)
			{
				int DepthMapWidth = Camera->max_capture_width;
                int DepthMapHeight = Camera->max_capture_height;
                int DepthMapCount = DepthMapWidth * DepthMapHeight;

                k4a_calibration_t calibration;
                k4a_device_get_calibration(Camera->device, Config.depth_mode, Config.color_resolution, &calibration);
                
                k4a_image_t xy_image = NULL;
                k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM,
                                 calibration.depth_camera_calibration.resolution_width,
                                 calibration.depth_camera_calibration.resolution_height,
                                 calibration.depth_camera_calibration.resolution_width * (int)sizeof(k4a_float2_t),
                                 &xy_image);
                
                k4a_create_xy_table(&calibration, xy_image);
                v2f *xy_map = (v2f *)k4a_image_get_buffer(xy_image);
                
                dimensions depth_image_dimensions = {DepthMapWidth, DepthMapHeight};
				
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
								
				size_t DepthMapSize = DepthMapCount * sizeof(uint16_t);
                uint16_t *DepthMap = (uint16_t *)VirtualAlloc(NULL, DepthMapSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
				
				color_point *VertexArray = (color_point *)VirtualAlloc(NULL, sizeof(color_point) * DepthMapCount, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
				uint32_t VertexCount = 0;
				
				float DeltaTime = 0.0f;
                float TotalTime = 0.0f;
                
                timer PointCloudComputeTimer = {.FramesToSumUp = 1000};
                timer RenderTimer = {.FramesToSumUp = 1000};
                timer FrameTimer = {.FramesToSumUp = 1000};

				ShowWindow(Window, SW_SHOWNORMAL);
				GlobalRunning = true;
				while(GlobalRunning)
				{
					double FrameTimeStart = GetTimeInSeconds();
					
					ProcessWindowMessages();
					//HandleInput(Window, Control, DeltaTime);
                    
                    Control->position = (v3f){.x = 5 * linalg_sin(TotalTime / 2), .z = 5 * linalg_cos(TotalTime / 2)};
                    Control->forward = (v3f){.x = -Control->position.x, .y = -Control->position.y, .z = -Control->position.z};
					
					RECT ClientRect;
					GetClientRect(Window, &ClientRect);
					dimensions RenderDimensions = {(uint32_t)ClientRect.right, (uint32_t)ClientRect.bottom};
					
                    // Depth Data Acquisition
					camera_get_depth_map(Camera, 0, DepthMap, DepthMapSize);
                    
                    // Point Cloud Computation
                    double TimeBegin = GetTimeInSeconds();
					calculate_point_cloud(VertexArray, &VertexCount, xy_map, DepthMap, DepthMapCount);
                    double TimeEnd = GetTimeInSeconds();
                    PrintAverageTime(&PointCloudComputeTimer, (float)(TimeEnd - TimeBegin));

                    // Rendering
                    TimeBegin = GetTimeInSeconds();
                    
					ClearFramebuffer(Framebuffer, 0.0f, 0.0f, 0.0f, 1.0f);
					ClearDepthBuffer(DepthBuffer);
					
					mat4 Model = Control->model;
					mat4 View = look_at(Control->position, v3f_add(Control->position, Control->forward), Control->up);
					mat4 Proj = perspective(Control->fov, (float)RenderDimensions.w / (float)RenderDimensions.h, 0.1f, 100.0f);
					mat4 MVP = mat4_mul(Proj, mat4_mul(View, Model));
					ProcessVertices(VertexArray, VertexCount, Pipeline, Framebuffer, DepthBuffer, MVP);
					
					DisplayFramebuffer(Framebuffer, WindowDC, RenderDimensions.w, RenderDimensions.h);
                    
                    TimeEnd = GetTimeInSeconds();
                    PrintAverageTime(&RenderTimer, (float)(TimeEnd - TimeBegin));
                    
					// DeltaTime
					double FrameTimeEnd = GetTimeInSeconds();
					DeltaTime = (float)(FrameTimeEnd - FrameTimeStart);
					PrintAverageTime(&FrameTimer, DeltaTime);
                    
                    TotalTime += DeltaTime;
				}
				
				//camera_release(Camera);
			}
			else
			{
				fprintf(stderr, "Could not initialize camera.\n");
			}
        }
		else
		{
			fprintf(stderr, "Could not create the window.\n");
		}
    }
	else
	{
		fprintf(stderr, "Could not register window class.\n");
	}
}
