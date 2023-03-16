#ifndef LINCE_TEXTURE_H
#define LINCE_TEXTURE_H

#include "lince/core/core.h"

typedef enum LinceTextureFlags {
	LinceTexture_Default = 0x0,
	LinceTexture_FlipY = 0x1,   // flips texture vertically on load
	// LinceTexture_DropAlpha, // 
	// LinceTexture_ForceAlpha // allows to load RGB format but adds alpha of 1
} LinceTextureFlags;

typedef struct LinceTexture {
	uint32_t id;               // OpenGL ID
	uint32_t width, height;    // dimension of 2D texture
	char name[LINCE_NAME_MAX]; // unique string identifier
	int32_t data_format;     // format of texture file, e.g. RGBA
	int32_t internal_format; // format of data in OpenGL buffer
} LinceTexture;

/* Loads a texture from a filename */
LinceTexture* LinceLoadTexture(const char* name, const char* path, uint32_t flags);

/* Creates texture from file */
LinceTexture* LinceCreateTexture(const char* name, const char* path);

/* Creates empty buffer with given dimensions */
LinceTexture* LinceCreateEmptyTexture(const char* name, 
	uint32_t width, uint32_t height);

/* Provides custom data to an existing texture buffer */
void LinceSetTextureData(LinceTexture* texture, unsigned char* data);

/* Deallocates texture memory and destroys OpenGL texture object */
void LinceDeleteTexture(LinceTexture* texture);

/* Binds the given texture to a slot (there are at least 16 slots) */
void LinceBindTexture(LinceTexture* texture, uint32_t slot);

// void LinceUnbindTexture(); // not necessary

#endif /* LINCE_TEXTURE_H */