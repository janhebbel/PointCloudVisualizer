#if defined(_WIN32)

//
// Windows

#include <winsock2.h>
#include <ws2tcpip.h>
#include <strsafe.h>
#include <windows.h>

typedef SOCKET      socket_t;
typedef IN_ADDR     my_in_addr_t;
typedef SOCKADDR_IN sockaddr_in_t;
typedef SOCKADDR    sockaddr_t;

#define valid_socket(socket) ((socket) != INVALID_SOCKET)
#define close_socket(socket) closesocket(socket)
#define get_last_error(X) WSAGetLastError()

HANDLE EventBufferRead;
HANDLE EventBufferFull;
HANDLE EndThread;
HANDLE ProducerThread;
DWORD ThreadID;

#elif defined(__linux__)

//
// Linux

#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

typedef int                socket_t;
typedef struct in_addr     my_in_addr_t;
typedef struct sockaddr_in sockaddr_in_t;
typedef struct sockaddr    sockaddr_t;

#define valid_socket(socket) ((socket) >= 0)
#define close_socket(socket) close(socket)
#define get_last_error() errno
#define INVALID_SOCKET -1

pthread_t ProducerThread;
pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ProducerCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t ConsumerCond = PTHREAD_COND_INITIALIZER;

volatile int BufferFull = 0;

#endif

// Platform Agnostic

typedef struct
{
    socket_t Host;
    socket_t Client;
} 
connection;

typedef struct
{
    socket_t ClientSocket;
    uint8_t *Buffer;
    size_t BufferSize;
    int ImageSize;
}
get_depth_image_data;

get_depth_image_data *ThreadData;

int Connect(connection *Connection)
{
    int status = 0;	
    int Domain = AF_INET;
    int Protocol;

#if defined(_WIN32)

    Protocol = IPPROTO_TCP;

	WSADATA WSAData;
	WORD Version = MAKEWORD(2, 2);
	
	status = WSAStartup(Version, &WSAData);
	if(status != 0)
    {
        fprintf(stderr, "WSAStartup failed.\n");
        return(-1);
    }

#elif defined(__linux__)

    Protocol = 0;

#endif

    socket_t Socket = socket(Domain, SOCK_STREAM, Protocol);
    
    if(valid_socket(Socket))
    {
        Connection->Host = Socket;

        // 192.168.10.1			
        my_in_addr_t InAddr;
        status = inet_pton(Domain, "192.168.10.1", &InAddr);
        assert(status == 1);
                    
        sockaddr_in_t Service = {0};
        Service.sin_family = (short)Domain;
        Service.sin_addr = InAddr;
        Service.sin_port = htons(10002);
        
        status = bind(Socket, (sockaddr_t *)&Service, sizeof(Service));
        if(0 != status)
        {
            fprintf(stderr, "Failed to bind the socket. Error: %d\n", get_last_error());
            return(-3);
        }
        
        status = listen(Socket, 1);
        if(0 != status)
        {
            fprintf(stderr, "'listen()' failed. Error: %d\n", get_last_error());
            return(-4);
        }
        
        socket_t ConnectedSocket = accept(Socket, NULL, NULL);
        if(!valid_socket(ConnectedSocket))
        {
            fprintf(stderr, "'accept()' failed. Error: %d\n", get_last_error());
            return(-5);
        }

        Connection->Client = ConnectedSocket;
    }
    else
    {
        fprintf(stderr, "Failed to create the socket. Error: %d\n", get_last_error());
        return(-2);
    }

    return(0);
}

void Disconnect(socket_t Socket)
{
    int status;

    status = close_socket(Socket);
    assert(status == 0);

#if defined(_WIN32)

    status = WSACleanup();
    assert(status == 0);
    
#endif
}

void GetDepthImageInfo(socket_t ClientSocket, uint32_t *WidthOut, uint32_t *HeightOut, uint32_t *DataSizeOut)
{
    // Read out the first 4 * 4 bytes meta data
    uint32_t DataSize, Width, Height, QuadCounterOld;
    
    int BytesReceived = 0;
    BytesReceived += recv(ClientSocket, (char *)&DataSize, 4, 0);
    BytesReceived += recv(ClientSocket, (char *)&Width, 4, 0);
    BytesReceived += recv(ClientSocket, (char *)&Height, 4, 0);
    BytesReceived += recv(ClientSocket, (char *)&QuadCounterOld, 4, 0);
    
    if(WidthOut)  *WidthOut  = Width;
    if(HeightOut) *HeightOut = Height;
    if(DataSizeOut) *DataSizeOut = DataSize;
}

void GetDepthImage(socket_t ClientSocket, uint8_t *Buffer, size_t BufferSize, int ImageSize)
{
    int BytesReceived = 0;
    
    if(Buffer)
    {
        int AllBytesReceived = 0;
        
        while(1)
        {
            int BytesReceivedTotal = 0;
            
            uint32_t Discard[4];
            BytesReceived = recv(ClientSocket, (char *)Discard, 16, 0);
            assert(BytesReceived == 16);
            
            uint8_t ImageDataInformation[8];
            BytesReceived = recv(ClientSocket, (char *)ImageDataInformation, 8, 0);
            assert(BytesReceived == 8);
            
            uint8_t CaptureMode = ImageDataInformation[2] >> 4;
            uint8_t QuadCount = CaptureMode;
            uint8_t QuadCounter = (ImageDataInformation[7] >> 4);
            uint8_t MagicByte = ImageDataInformation[3];
            assert(MagicByte == 0x4a);
            
            if(CaptureMode >= 8)
            {
                QuadCount = CaptureMode - 8;
            }
            
            char *Pointer = (char *)(Buffer + QuadCounter * ImageSize);
            
            memcpy(Pointer, ImageDataInformation, 8);
            Pointer += 8;
            
            do
            {
                BytesReceived = recv(ClientSocket, Pointer, ImageSize - BytesReceivedTotal - 8, 0);
                
                Pointer += BytesReceived;
                BytesReceivedTotal += BytesReceived;
                AllBytesReceived += BytesReceived;
            }
            while(BytesReceivedTotal != ImageSize - 8);
            
            if(QuadCounter == 3)
                break;
        }
    }
}

#if defined(_WIN32)
DWORD WINAPI ThreadProc(LPVOID Param)
{
    get_depth_image_data *DepthImageData = (get_depth_image_data *)Param;

    while(1)
    {
        DWORD WaitResult = WaitForSingleObject(EndThread, 0);
        if(WaitResult == WAIT_OBJECT_0)
        {
            break;
        }

        WaitForSingleObject(EventBufferRead, INFINITE);
        {
            GetDepthImage(DepthImageData->ClientSocket, DepthImageData->Buffer, DepthImageData->BufferSize, DepthImageData->ImageSize);
        }
        SetEvent(EventBufferFull);
    }

    return(0);
}
#endif

#if defined(__linux__)
void *ThreadProc(void *Param)
{
    get_depth_image_data *DepthImageData = (get_depth_image_data *)Param;

    while(1)
    {
        pthread_mutex_lock(&Mutex);
        {
            while(BufferFull)
            {
                pthread_cond_wait(&ProducerCond, &Mutex);
            }

            GetDepthImage(DepthImageData->ClientSocket, DepthImageData->Buffer, DepthImageData->BufferSize, DepthImageData->ImageSize);
            BufferFull = 1;
            
        }
        pthread_mutex_unlock(&Mutex);

        pthread_cond_signal(&ConsumerCond);
    }

    return(NULL);
}
#endif

void CreateMyThread(get_depth_image_data *ThreadDataIn)
{
#if defined(_WIN32)

    EventBufferRead = CreateEvent(NULL, FALSE, TRUE, L"EventBufferRead");
    EventBufferFull = CreateEvent(NULL, FALSE, FALSE, L"EventBufferFull");
    EndThread = CreateEvent(NULL, FALSE, FALSE, L"EndThread");

    ThreadData = (get_depth_image_data *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(get_depth_image_data));

    *ThreadData = *ThreadDataIn;

    ProducerThread = CreateThread(NULL,
                                  0,
                                  ThreadProc,
                                  ThreadData,
                                  0,
                                  &ThreadID);

#elif defined(__linux__)

    pthread_create(&ProducerThread, NULL, ThreadProc, NULL);

#endif
}

void TerminateMyThread(void)
{
#if defined(_WIN32)

    SetEvent(EndThread);

#elif defined(__linux)

    pthread_cancel(ProducerThread);

#endif
}

int WaitForOtherThread(int TimeoutInMilliseconds)
{
#if defined(_WIN32)

    DWORD Result = WaitForSingleObject(EventBufferFull, TimeoutInMilliseconds);
    return(Result == WAIT_OBJECT_0); // WAIT_OBJECT_0 means EventBufferFull was signaled

#elif defined(__linux__)

    int Result = 0;

    pthread_mutex_lock(&Mutex);
    {
        struct timespec Timeout;
        clock_gettime(CLOCK_REALTIME, &Timeout);
        Timeout.tv_nsec += TimeoutInMilliseconds * 1000000;

        while(!BufferFull && Result == 0)
        {
            Result = pthread_cond_timedwait(&ConsumerCond, &Mutex, &Timeout);
        }
    }
    pthread_mutex_unlock(&Mutex);

    return(Result == 0);

#endif
}

void SignalOtherThread(void)
{
#if defined(_WIN32)

    SetEvent(EventBufferRead);
    WaitForSingleObject(ProducerThread, INFINITE);

#elif defined(__linux__)

    pthread_cond_signal(&ProducerCond);

#endif
}
