#ifndef IMGFILE_H_
#define IMGFILE_H_

#include "glheaders.h"
#include "texture.h"

#define O3_TC_RGB_S3TC_DXT1 1
#define O3_TC_RGBA_S3TC_DXT5 4
#define u8 GLubyte
#define u32 GLuint

struct O3TC_Header
{
	// Magic number: Must be O3TC.
	char magic_number[4];
	
	// Must be filled with sizeof(O3TC_Header).
	u32 header_size;
	
	// Version. Currently: 0x00000001.
	u32 version;
};

struct O3TC_Chunk_Header
{
	// Must be filled with sizeof(O3TC_Chunk_Header):
	u32 chunk_header_size;
	
	// Reserved
	u32 reserved1; 

	// The size of the data chunk that follows this one. 
	u32 size; 

	// Reserved
	u32 reserved2; 

	// Pixel format:
	// - O3_TC_RGB_S3TC_DXT1 = 1
	// - O3_TC_RGBA_S3TC_DXT5 = 4
	// - O3_TC_ATI3DC_ATI2N = 16
	u32 internal_pixel_format;

	// Texture width.
	u32 width;
	
	// Texture height.
	u32 height;
	
	// Texture depth.
	u32 depth;

	// Number of mipmaps.
	u32 num_mipmaps;
	
	// The texture name (optional).
	char texture_name[128];
	
	// The texture id (optional).
	u32 texture_id;
};

class GenericImage
{
public:
	int width;
	int height;
	int components;
	int pix_type;
	int mipmaps;
	unsigned char* pixels;


public:
	GenericImage();
	~GenericImage();
	void init();
	void init(unsigned char* p, int w, int h, int comp, int);
	
	//read and write
	int load(char* filename);
	void print();
	void unload();
	bool save(char* filename);
	
	//image processing
	void gen_normalMap(GenericImage* dst);
	void setup_texture(tex_unit* tex, int mipmaps, bool comp);
};

#endif /*IMGFILE_H_*/
