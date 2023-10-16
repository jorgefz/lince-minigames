#ifndef LINCE_TEXTURE_H
#define LINCE_TEXTURE_H

#include "lince/core/core.h"

/** @enum LinceTextureFlags
* @brief Settings for loading a texture
*/
typedef enum LinceTextureFlags {
	LinceTexture_Default = 0x0,	///< Explicit default settings
	LinceTexture_FlipY = 0x1,   ///< Flips texture vertically on load
	LinceTexture_DropAlpha, 	///< (unused) Loads RGBA data as RGB ignoring alpha channel.
	LinceTexture_ForceAlpha 	///< (unused) Allows to load RGB format but adds alpha of 1.
} LinceTextureFlags;

/** @struct LinceTexture */
typedef struct LinceTexture {
	uint32_t id;               	///< OpenGL ID
	uint32_t width, height;    	///< 2D size
	int32_t data_format;     	///< Input format of texture file, e.g. RGBA
	int32_t internal_format; 	///< Output format of data in OpenGL buffer
} LinceTexture;

/** @brief Loads a texture from file
* @param path Path to texture file
* @param flags Settings.
*/
LinceTexture* LinceLoadTexture(const char* path, uint32_t flags);

/** @brief Creates empty texture buffer with given dimensions
* @param width  Width in pixels
* @param height Height in pixels
*/
LinceTexture* LinceCreateEmptyTexture(uint32_t width, uint32_t height);

/** @brief Provides custom data to an existing texture buffer */
void LinceSetTextureData(LinceTexture* texture, unsigned char* data);

/** @brief Deallocates texture memory and destroys OpenGL texture object */
void LinceDeleteTexture(LinceTexture* texture);

/**@brief Binds the given texture to a slot (there are at least 16 slots) */
void LinceBindTexture(LinceTexture* texture, uint32_t slot);

// void LinceUnbindTexture(); // not necessary

#endif /* LINCE_TEXTURE_H */