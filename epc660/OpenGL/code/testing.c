#include <stdio.h>

void ReadDepthDataFromFile(const char *Name, unsigned char *Dst, size_t DstSize)
{
    FILE *File = fopen(Name, "rb");

    size_t BytesRead = fread(Dst, 1, DstSize, File);
    if(BytesRead != DstSize)
    {
        fprintf(stderr, "Something ain't right.");
    }
}
