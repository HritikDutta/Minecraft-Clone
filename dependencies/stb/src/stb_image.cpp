#define STB_IMAGE_IMPLEMENTATION

#include "platform/platform.h"
#define STBI_MALLOC PlatformAllocate
#define STBI_REALLOC PlatformReallocate
#define STBI_FREE PlatformFree

#include "../include/stb_image.h"