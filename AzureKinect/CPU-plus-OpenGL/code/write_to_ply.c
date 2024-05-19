#if 0
#include <stdio.h>

struct color_point;

void write_point_cloud_to_ply(color_point *points, size_t size, char *filename)
{
    char properties[128];
    sprintf_s(properties, 128,
              "property float x\n"
              "property float y\n"
              "property float z\n"
              "property uchar red\n"
              "property uchar green\n"
              "property uchar blue\n");

    char *points_string = (char *)malloc(size * 72);
    if(points_string)
    {
        char *offset = points_string;
        for(int i = 0; i < size; ++i)
        {
            float x, y, z, r, g, b;
            x = points[i].xyz[0];
            y = points[i].xyz[1];
            z = points[i].xyz[2];
            r = points[i].rgb[0];
            g = points[i].rgb[1];
            b = points[i].rgb[2];
            int bytes_written = sprintf_s(offset, 72, "%f %f %f %f %f %f\n", x, y, z, r, g, b);
        
            offset += bytes_written;
        }

        FILE *file;
        if(!fopen_s(&file, filename, "w"))
        {
            fprintf(file, "ply\nformat ascii 1.0\nelement vertex %zu\n%send_header\n%s", size, properties, points_string);
        }
        fclose(file);
    
        free(points_string);
    }
}
#endif
