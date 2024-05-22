#include <winsock2.h>
#include <ws2tcpip.h>
#include <strsafe.h>
#include <windows.h>

typedef struct
{
    SOCKET Host;
    SOCKET Client;
} 
connection;

HANDLE EventBufferRead;
HANDLE EventBufferFull;
HANDLE ProducerThread;
DWORD ThreadID;

typedef struct
{
    SOCKET ClientSocket;
    uint8_t *Buffer;
    size_t BufferSize;
    int ImageSize;
}
get_depth_image_data;

get_depth_image_data *ThreadData;

DWORD WINAPI ThreadProc(LPVOID Param);

void CreateMyThread(get_depth_image_data *ThreadDataIn)
{
    EventBufferRead = CreateEvent(NULL, FALSE, TRUE, "EventBufferRead");
    EventBufferFull = CreateEvent(NULL, FALSE, FALSE, "EventBufferFull");

    ThreadData = (get_depth_image_data *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(get_depth_image_data));

    *ThreadData = *ThreadDataIn;

    ProducerThread = CreateThread(NULL,
                                  0,
                                  ThreadProc,
                                  ThreadData,
                                  0,
                                  &ThreadID);
}

int Connect(connection *Connection)
{
    int Status = 0;
	
	WSADATA WSAData;
	WORD Version = MAKEWORD(2, 2);
	
	Status = WSAStartup(Version, &WSAData);
	if(Status == 0)
	{	
		SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		
		if(Socket != INVALID_SOCKET)
		{
            Connection->Host = Socket;

			// 192.168.10.1			
			IN_ADDR InAddr;
			Status = inet_pton(AF_INET, "192.168.10.1", &InAddr);
			assert(Status == 1);
						
			SOCKADDR_IN Service = {0};
			Service.sin_family = AF_INET;
			Service.sin_addr = InAddr;
			Service.sin_port = htons(10002);
			
			Status = bind(Socket, (SOCKADDR *)&Service, sizeof(Service));
			if(Status == SOCKET_ERROR)
            {
                fprintf(stderr, "An error occurred when binding the socket. Did you change your network settings as per the GitHub README instructions? Error code: %d\n", WSAGetLastError());
                assert(0);
                return(-3);
            }
			
			Status = listen(Socket, 1);
			assert(Status == 0);
			
			SOCKET ConnectedSocket = accept(Socket, NULL, NULL);
			assert(ConnectedSocket != INVALID_SOCKET);

            Connection->Client = ConnectedSocket;
        }
        else
        {
            fprintf(stderr, "Failed to create the socket. Error: %d\n", WSAGetLastError());
            return(-2);
        }
    }
    else
    {
        fprintf(stderr, "WSAStartup failed.\n");
        return(-1);
    }

    return(0);
}

void GetDepthImageInfo(SOCKET ClientSocket, uint32_t *WidthOut, uint32_t *HeightOut, uint32_t *DataSizeOut)
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

void GetDepthImage(SOCKET ClientSocket, uint8_t *Buffer, size_t BufferSize, int ImageSize)
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

DWORD WINAPI ThreadProc(LPVOID Param)
{
    get_depth_image_data *DepthImageData = (get_depth_image_data *)Param;

    while(1)
    {
        WaitForSingleObject(EventBufferRead, INFINITE);
        {
            GetDepthImage(DepthImageData->ClientSocket, DepthImageData->Buffer, DepthImageData->BufferSize, DepthImageData->ImageSize);
        }
        SetEvent(EventBufferFull);
    }

    return(0);
}

void Disconnect(SOCKET Socket)
{
    int Status;

    Status = closesocket(Socket);
    assert(Status == 0);

    Status = WSACleanup();
    assert(Status == 0);
}