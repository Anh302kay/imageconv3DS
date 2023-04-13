#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <png.h>

typedef uint32_t u32;
typedef uint16_t u16;

int width, height;
png_byte colour_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep * row_pointers = NULL;

void read_png(char* filename)
{
    unsigned char header[8];
    FILE* fp = fopen(filename, "rb");
    fread(header, 1, 8, fp);
    if(png_sig_cmp(header, 0, 8))
    {
        printf("File: %s , is not considered a png", filename);
        abort();
    }

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if(!png_ptr)
    {
        printf("could not create png struct");
        abort();
    }

    info_ptr = png_create_info_struct(png_ptr);

    if(!info_ptr)
    {
        printf("could not create info struct");
        abort();
    }
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        printf("[read_png_file] Error during init_io");
        abort();        
    }
            
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);


    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    colour_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    if(bit_depth == 16)
        png_set_strip_16(png_ptr);

    if(colour_type == PNG_COLOR_TYPE_PALETTE)
      png_set_palette_to_rgb(png_ptr);

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if(colour_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
      png_set_expand_gray_1_2_4_to_8(png_ptr);

    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
      png_set_tRNS_to_alpha(png_ptr);

    // These color_type don't have an alpha channel then fill it with 0xff.
    if(colour_type == PNG_COLOR_TYPE_RGB ||
       colour_type == PNG_COLOR_TYPE_GRAY ||
       colour_type == PNG_COLOR_TYPE_PALETTE)
      png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);

    if(colour_type == PNG_COLOR_TYPE_GRAY ||
       colour_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      png_set_gray_to_rgb(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        printf("[read_png_file] Error during read image");
        abort();        
    }

    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for(int y=0;y<height; y++)
        row_pointers[y] = (png_byte*)malloc(png_get_rowbytes(png_ptr, info_ptr));

    png_read_image(png_ptr, row_pointers);

    fclose(fp);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
}

void PrintUsage()
{
    printf("usage: imageconv image.png -o image.h\n");
    printf("       imageconv -o image.h image.png\n");
}

int main(int argc, char* argv[])
{
    if(argc <= 2 || argc > 4)
    {
        PrintUsage();
        return 1;
    }
    char input[256];
    char output[256];
    if(!strcmp(argv[1],"-o") || !strcmp(argv[1],"-O"))
    {
        read_png(argv[3]);
        strncpy(input, argv[3], 256);
        strncpy(output, argv[2], 256);
    }
    else if(!strcmp(argv[2],"-o") || !strcmp(argv[2],"-O"))
    {
        read_png(argv[1]);
        strncpy(input, argv[1], 256);
        strncpy(output, argv[3], 256);
    }
    else
    {
        PrintUsage();
        return 1;
    }

    for(int i = 0; i < 256; i++)
    {
        if(input[i] == '.')
            input[i] = '_';
    }

    u32** img = (u32**)row_pointers;

    u16 convertedimg[width*height];
    memset(convertedimg, 0, width*height*2);

    int cpos = 0;
    for(int x = 0; x < width; x++)
    {
        for(int y = height-1; y >= 0; y--)
        {
            if((img[y][x] >> 24) == 255)
            {
                convertedimg[cpos] =  (((img[y][x]&0xf8)<<8) + ((img[y][x]&0xfc00)>>5) + ((img[y][x]&0xF80000)>>19)); // due to endianess or smth 'r' and 'b' are swapped
            }
            else
            {
                convertedimg[cpos] = 0;
            }
            cpos++;
        }
    }

    FILE* fp = fopen(output, "w");

    fprintf(fp, "#pragma once\n\n");

    fprintf(fp, "const unsigned int %s%s = %d;\n", input, "_size", width*height);

    fprintf(fp, "const u16 %s[%d] = { \n    %#06x", input, width*height, convertedimg[0]);
    for(int i = 1; i < width*height; i++)
    {
        fprintf(fp, ", ");
        if(i % 16 == 0)
        fprintf(fp, "\n    ");
        fprintf(fp, "%#06x", convertedimg[i]);
    }
    fprintf(fp, "\n};");

    fclose(fp);

    for (int y=0; y<height; y++)
        free(row_pointers[y]);

    free(row_pointers);
   
    return 0;
}