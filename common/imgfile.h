#ifndef IMGFILE_H_
#define IMGFILE_H_

#include "glheaders.h"
#include "texture.h"
#include "fbo.h"
#include "buffer_object.h"

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
	int pix_depth;
	int mipmaps;
	void* pixels;


public:
        GenericImage();
	GenericImage(const char*);
	~GenericImage();

        enum{COLOR, NORMAL, HEIGHT, ELEVATION};
	
	void init(void* p, int w, int h, int c, int b);
	
	//read and write
	void load(const char* filename);
	void print();
	void unload();
	bool save(const char* filename);
	
	//image processing
        char* gen_normal();
	void gen_normalMap(const char*);

        //setup texture
        static GLuint resolve_tex_target(int, int);
        static GLuint resolve_internal_format(int, int);
        static GLuint resolve_external_format(int);
        static GLuint resolve_external_type(int);

        void transform(int, GLuint, float, float);
	tex_unit* setup_texture(GLuint target);
        fbo_unit* setup_fbo(GLuint target);
        buffer_object* setup_vbo();
        void update_texture_subimage(tex_unit* tex,
                                     GLuint extformat,
                                     GLuint datatype);
private:
        void read_png(const char*);
        void write_png(const char*);
        void read_jpg(const char*);
        void write_jpg(const char*, int);
        //size_t size_raw(const char*);
        //int read_raw(const char*);
        //int write_raw(const char*);
};

#endif /*IMGFILE_H_*/
