//#define STBI_NO_PNG
//#define STBI_NO_BMP
//#define STBI_NO_PSD
//#define STBI_NO_TGA
//#define STBI_NO_GIF
//#define STBI_NO_HDR
//#define STBI_NO_PIC
//#define STBI_NO_PNM 
//
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "helpers.hpp"


unsigned char* load_image(char const* img, int* width, int* height, int* nrChannels, int desiredChannels)
{
    //return stbi_load("metu_flag.jpg", width, height, nrChannels, 0);
    return stbi_load(img, width, height, nrChannels, desiredChannels);
}

int factorial(int x){
    if(x <2) return 1;
    return x * factorial(x-1);
}
