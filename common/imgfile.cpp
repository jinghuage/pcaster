//imgfile.cpp
#include <math.h>
#include <string.h>
#include <assert.h>
#include <png.h>

extern "C"{
#include <jpeglib.h>
}

#include <errno.h>
//#include <regex.h>
//#include <stdint.h>
#include <iostream>

#include "misc.h"
#include "imgfile.h"

//-----------------------------------------------------------------------------

// static const GLuint data_type[][2] = {
// 	{GL_UNSIGNED_BYTE,		1},
// 	{GL_UNSIGNED_SHORT,		2},
// 	{GL_FLOAT,			4},
// 	{GL_INT,			4},
// 	{O3_TC_RGBA_S3TC_DXT5, 	        2},
// 	{O3_TC_RGB_S3TC_DXT1, 	        4}
// };
    


//-----------------------------------------------------------------------------

static void fail(const char *error)
{
    fprintf(stderr, "Error: %s\n", error);
    exit(EXIT_FAILURE);
}

static void success(const char *msg)
{
    fprintf(stderr, "Success: %s\n", msg);
}


//-----------------------------------------------------------------------------

GenericImage::GenericImage()
{
	width = 0;
	height = 0;
	components = 0;
	pix_depth = 0;
	pixels = NULL;
}


GenericImage::GenericImage(const char* filename)
{
    load(filename);
}

GenericImage::~GenericImage()
{
    if(pixels)
    {
        free(pixels);
        pixels = 0;
    }
}


void GenericImage::init(void* p, int w, int h, int c, int b)
{
	if(w) width = w;
	if(h) height = h;
	if(c) components = c;
	if(b) pix_depth = b;
	
	if(p){
            //int mapsize = width * height * components * data_type[pix_type][1];
		//pixels = (unsigned char*)malloc(mapsize);
		//memcpy(pixels, p, mapsize);
               
                pixels = p;
	}

}

//-----------------------------------------------------------------------------

void GenericImage::read_png(const char *filename)
{
    int w=0, h=0, c=0, b=0;
	
	
    FILE       *filep = NULL;
    png_structp readp = NULL;
    png_infop   infop = NULL;
    png_bytep  *bytep = NULL;

    void *p = NULL;

    /* Initialize all PNG import data structures. */

    if (!(filep = fopen(filename, "rb")))
    {
        fail("can't open file\n");
    }

    
    if (!(readp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        fail("can't create png read struct\n");
    if (!(infop = png_create_info_struct(readp)))
        fail("can't create png info struct\n");

    /* Enable the default PNG error handler. */

    if (setjmp(png_jmpbuf(readp)) == 0)
    {
        /* Read the PNG header. */

        png_init_io (readp, filep);

        //STRIP_16 enforces 8 bit readout       
//         png_read_png(readp, infop, PNG_TRANSFORM_STRIP_16 |
//                                    PNG_TRANSFORM_PACKING, NULL);
        //SWAP_ENDIAN swap byte order
        png_read_png(readp, infop, PNG_TRANSFORM_SWAP_ENDIAN |
                     PNG_TRANSFORM_PACKING, NULL);
        
        /* Extract image properties. */

        w = (int) png_get_image_width (readp, infop);
        h = (int) png_get_image_height(readp, infop);
        c = (int) png_get_channels(readp, infop);
        b = (int) png_get_bit_depth(readp, infop)/8;

        /* Read the pixel data. */

        if ((bytep = png_get_rows(readp, infop)))
        {
            int i, j, s = w*c*b;

            /* Allocate the final pixel buffer and copy pixels there. */

            if ((p = malloc(w * h * c * b)))
            {
                for (i = 0, j = h - 1; j >= 0; ++i, --j)
                    memcpy((png_bytep) p + s * i, bytep[j], s);
            }
            else
            {
                fail("can't allocate image buffer\n");
            }
        }
    } 

    /* Release all resources. */

    png_destroy_read_struct(&readp, &infop, NULL);
    fclose(filep);
    
    //setup the generic image object 

    width = w;
    height = h;
    components = c;
    pix_depth = b;
    pixels = p;

    print();

    success("read png file\n");
}

//-----------------------------------------------------------------------------

void GenericImage::write_png(const char *filename)
{	
    int w = width;
    int h = height;
    int c = components;
    int b = pix_depth;
    void* p = pixels;

    FILE       *filep = NULL;
    png_structp writep = NULL;
    png_infop   infop = NULL;
    png_bytep  *bytep = NULL;

    /* Initialize all PNG import data structures. */

    if (!(filep = fopen(filename, "wb")))
        fail("can't open file\n");
    if (!(writep = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        fail("can't create png write struct\n");
    if (!(infop = png_create_info_struct(writep)))
        fail("can't create png info struct\n");

    /* Enable the default PNG error handler. */

    if (setjmp(png_jmpbuf(writep)) == 0)
    {
        png_init_io (writep, filep);
        
        /* Set the image information here.  Width and height are up to 2^31,
         * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
         * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
         * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
         * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
         * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
         * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
         */
        int color_type;
        switch (c)
        {
        case 1: color_type = PNG_COLOR_TYPE_GRAY; break;
        case 2: color_type = PNG_COLOR_TYPE_GRAY_ALPHA; break;
        case 3: color_type = PNG_COLOR_TYPE_RGB; break;
        case 4: color_type = PNG_COLOR_TYPE_RGB_ALPHA; break;
        default: color_type = PNG_COLOR_TYPE_RGB;
        }
        png_set_compression_level(writep, 9);
        png_set_IHDR(writep, infop, w, h, b*8, color_type,
                     PNG_INTERLACE_NONE, 
                     PNG_COMPRESSION_TYPE_DEFAULT, 
                     PNG_FILTER_TYPE_DEFAULT);
    	png_write_info(writep, infop);
    		
    	/* pack pixels into bytes */
        png_set_packing(writep);
    		      
    	
        bytep = (png_bytep *) png_malloc(writep, 
                                         h * sizeof (png_bytep));
    	for (int k = 0; k < h; k++)
            bytep[k] = (png_bytep)p + k*w*c*b;

        /* write out the entire image data in one call */
        //png_write_image(writep, bytep);
      	/* Write a few rows at a time. */
      	//png_write_rows(writep, &bytep[first_row], number_of_rows);

      	/* If you are only writing one row at a time, this works */
//       	for (int y = 0; y < h; y++)
//       	{
//             png_write_rows(writep, &row_pointers[y], 1);
//       	}

//         png_write_end(writep, infop);
	   	
        //set rows and write png
        png_set_rows(writep, infop, bytep);
        //png_write_png(writep, infop, PNG_TRANSFORM_STRIP_16 |
        //                           PNG_TRANSFORM_PACKING, NULL);   
        png_write_png (writep, infop, PNG_TRANSFORM_SWAP_ENDIAN, NULL);
        free(bytep);
    }
    else fail("can't write png data\n");

    /* Release all resources. */

    png_destroy_write_struct(&writep, &infop);
    fclose(filep);

    success("write png file\n");
}


/*---------------------------------------------------------------------------*/

void GenericImage::read_jpg(const char *filename)
{
    unsigned char *p = NULL;
    int w, h, c;

    FILE *fin;

    //fp = fopen(filename, "rb");
    //if(fp == NULL){ fprintf(stderr, "test: fopen() file %s error!\n", filename); exit(0); }
    //else { fprintf(stderr, "test: fopen() file %s ok!\n", filename); fclose(fp); }
    
    if ((fin = fopen(filename, "rb")))
    {
    	//fprintf(stderr, "begin to read in jpg header info.\n");
    	fprintf(stderr, "fopen() file %s ok!\n", filename);
    	
        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr         jerr;

        /* Initialize the JPG decompressor. */

        cinfo.err = jpeg_std_error(&jerr);

        jpeg_create_decompress(&cinfo);
        jpeg_stdio_src(&cinfo, fin);

        /* Grab the JPG header info. */

        jpeg_read_header(&cinfo, TRUE);
        jpeg_start_decompress(&cinfo);

        w = cinfo.output_width;
        h = cinfo.output_height;
        c = cinfo.output_components;

        /* Allocate the pixel buffer and copy pixels there. */

        if ((p = (unsigned char *) malloc ((w) * (h) * (c))))
        {
            int i = h - 1;

            while (cinfo.output_scanline < cinfo.output_height)
            {
                unsigned char *s = p + i * (w) * (c);
                i -= jpeg_read_scanlines(&cinfo, &s, 1);
            }
        }

        /* Finalize the decompression. */

        jpeg_finish_decompress (&cinfo);
        jpeg_destroy_decompress(&cinfo);

        fclose(fin);
  
        //set the generic iimage object
        width = w;
        height = h;
        pixels = p;
        components = c;
        pix_depth = 1;
	    
        print();
        success("file opened\n");
    }
    else
    {
        fail("can't open file\n");
    }
}

//-----------------------------------------------------------------------------

void GenericImage::write_jpg(const char *filename, int quality)
{
    FILE * fout;
    int w = width;
    int h = height;
    int b = components;
    unsigned char* p = (unsigned char*)pixels;
	
    if ((fout = fopen(filename, "wb")))
    {
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
  
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);

        jpeg_stdio_dest(&cinfo, fout);


        cinfo.image_width = w; 				
        cinfo.image_height = h;
        cinfo.input_components = b;			
        cinfo.in_color_space = JCS_RGB; 	
		
        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, quality, TRUE );
		
        jpeg_start_compress(&cinfo, TRUE);
		
		
        int i = 0;
        while (cinfo.next_scanline < cinfo.image_height) 
        {
            unsigned char *s = p + i * w * b;
            i += jpeg_write_scanlines(&cinfo, &s, 1);
        }

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);
  		
    	fclose(fout);
        success("file written\n");
    }
    fail("file not written\n");
}


//-----------------------------------------------------------------------------
/*
#define K_ERR -1
#define K_OK 0

static int read_O3TC( const char *filename, GenericImage* img )
{
	if( !filename )
		return( K_ERR );

	FILE *fp = fopen( filename, "rb" );

	if( !fp )
		return( K_ERR );

	//------ Read the whole file once.
	//
	fseek( fp, 0, SEEK_END );
	long o3tc_file_size = ftell( fp );
	fseek( fp, 0, SEEK_SET );


	u8 *buffer = new u8[o3tc_file_size];
	fread( buffer, o3tc_file_size, 1, fp );
	fclose( fp );


	//------ Get a ptr on O3TC_Header.
	//
	u8 *raw_ptr = buffer + 4; // The first 4 bytes are useless.
	O3TC_Header *header = (O3TC_Header *)raw_ptr;

	// Check for O3TC magic number...
	//
	if( strncmp(header->magic_number, "O3TC", 4) != 0 )
	{
		fprintf(stderr, "Bad O3TC header.\n");
		delete [] buffer;
		return( K_ERR );
	}


	//------ Get a ptr on O3TC_Chunk_Header.
	//
	raw_ptr = buffer + 4 + sizeof(O3TC_Header);
	O3TC_Chunk_Header *chunk_header = (O3TC_Chunk_Header *)raw_ptr;

	//------ Get a ptr on compressed pixmap.
	//
	raw_ptr = buffer + 4 + sizeof(O3TC_Header) + sizeof(O3TC_Chunk_Header);
	u8 *pCompressedPixmap = raw_ptr;


	//----- Now do the easy part...
	//

	// Get texture dims.
	//
	img->width = chunk_header->width;
	img->height = chunk_header->height;

	//  Get the number of mipmaps.
	//
	img->mipmaps = chunk_header->num_mipmaps;

	// Get the compression type.
	//
	if( chunk_header->internal_pixel_format == O3_TC_RGBA_S3TC_DXT5) img->pix_type = 4;
	else if( chunk_header->internal_pixel_format == O3_TC_RGB_S3TC_DXT1) img->pix_type = 5;

	// Allocate memory for compressed data and read them!
	//
	img->pixels = new u8[chunk_header->size];
	memcpy( img->pixels, pCompressedPixmap, chunk_header->size );

	// Ok, we're done. Free resources and bye!
	//
	delete [] buffer;
	return(K_OK);
}


*/


 //---------------------------------------------------------------------------
/*
size_t GenericImage::size_raw(const char *name)
{
    const char *pattern = "([0-9]+)x([0-9]+)x([0-9]+)([bdfs])";

    regmatch_t match[5];
    regex_t    reg;
    size_t     siz = 0;
    int w=0, h=0, c=0, b=0;

    assert(regcomp(&reg, pattern, REG_EXTENDED) == 0);

    if (regexec(&reg, name, 5, match, 0) == 0)
    {
        w = (int) strtol(name + match[1].rm_so, NULL, 0);
        h = (int) strtol(name + match[2].rm_so, NULL, 0);
        c = (int) strtol(name + match[3].rm_so, NULL, 0);

        switch (name[match[4].rm_so])
        {
        case 'b': b = 1; break;
        case 's': b = 2; break;
        case 'f': b = 4; break;
        case 'd': b = 8; break;
        }

        siz = w * h * c * b;
    }

    regfree(&reg);

    width = w;
    height = h;
    components = c;
    pix_depth = b;

    return siz;
}


//----------------------------------------------------------------------------

int GenericImage::read_raw(const char *name)
{
    void   *p = NULL;
    FILE  *fp;
    size_t sz;

    if ((sz = size_raw(name)))
    {
        if ((fp = fopen(name, "rb")))
        {
            int w, h, c, b;
            w = width;
            h = height;
            c = components;
            b = pix_depth;

            int s = w * c * b;

            if ((p = malloc(sz)))
                for (int r = h - 1; r >= 0; --r)
                    fread((uint8_t *) p + r * s, 1, s, fp);

            pixels = p;
            fclose(fp);
            return 1;
        }
    }
    return 0;

}

//---------------------------------------------------------------------------

int GenericImage::write_raw(const char *name)
{
    FILE  *fp;
    size_t sz;

 
    if ((sz = size_raw(name)))
    {
        if ((fp = fopen(name, "wb")))
        {
            int w, h, c, b;
            w = width;
            h = height;
            c = components;
            b = pix_depth;

            int s = w * c * b;

            for (int r = h - 1; r >= 0; --r)
                fwrite((uint8_t *) pixels + r * s, 1, s, fp);

            fclose(fp);
            return 1;
        }
        else fail(strerror(errno));
    }
    return 0;
}
*/

//----------------------------------------------------------------------

void GenericImage::load(const char* filename)
{
    const char *ext = filename + strlen(filename) - 4;
   
    if (!strcmp(ext, ".png") || !strcmp(ext, ".PNG"))
        read_png(filename);
    else if (!strcmp(ext, ".jpg") || !strcmp(ext, ".JPG"))
        read_jpg(filename);
//    else if (!strcmp(ext, ".raw") || !strcmp(ext, ".RAW"))
//        read_raw(filename);
    else fail("only support jpeg and png image format\n");
}



void GenericImage::print()
{
    fprintf(stderr, "img size(%d, %d), %d channels, %d pixel depth\n", 
            width, height, components, pix_depth);
}


void GenericImage::unload()
{
    if(pixels)
    {
        free(pixels);
        pixels = 0;
    }
}

bool GenericImage::save(const char* filename)
{
    const char *ext = filename + strlen(filename) - 4;
    
    if (!strcmp(ext, ".png") || !strcmp(ext, ".PNG"))
        write_png(filename);
    else if (!strcmp(ext, ".jpg") || !strcmp(ext, ".JPG"))
        write_jpg(filename, 100);
//    else if (!strcmp(ext, ".raw") || !strcmp(ext, ".RAW"))
//        write_raw(filename);
    else return false;
	
    return true;
}

//-------------------------------------------------------------------
// generate normal (derivative) from 2d image by Sobel operator
//-------------------------------------------------------------------
char* GenericImage::gen_normal()
{
    const float GX[3][3] = {{-1, 0, 1},
                            {-2, 0, 2}, 
                            {-1, 0, 1}};
						    
    const float GY[3][3] = {{-1, -2, -1},
                            {0,  0,  0}, 
                            {1,  2,  1}};					    
	
    int x, y, i, j, nx, ny;
    float pv, dZdX, dZdY, nX, nY, nZ, oolen;
	
    unsigned char* p = (unsigned char*)pixels;
    char* nmap = new char[width * height * 3];
    int index = 0;
	

    // think of the 2d image as a x-y plane
    // pixel value is z value

    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++)
        {
            dZdX = 0;
            dZdY = 0;
            for(i=0; i<3; i++)
            {
                ny = y+i-1;
                if(ny < 0) ny = 0;
                else if(ny >= height) ny = height-1;
				
                for(j=0; j<3; j++)
                {
                    nx = x+j-1;
                    if(nx < 0) nx = 0;
                    else if(nx >= width) nx = width-1;
					
                    pv = (float) p[(ny*width + nx)*components + 0]; //if RGB format, use the red channel
                    dZdX  += pv / 255.0f * GX[i][j];
                    dZdY  += pv / 255.0f * GY[i][j];
                }
            }
			
            //tangent vector (dX, 0, dZ) => (1, 0, dZdX)
            //bitangent vector (0, dY, dZ) => (0, 1, dZdY)
            //normal = cross(tangent, bitangent)
            //nX = -dZdX;
            //nY = -dZdY;
            //nZ = 1;
			
            //for terrain, switch Y and Z notation
            nX = -dZdX;
            nY = 1.0;
            nZ = -dZdY;


            // Normalize
            oolen = 1.0f/((float) sqrt(nX*nX + nY*nY + nZ*nZ));
            nX *= oolen;
            nY *= oolen;
            nZ *= oolen;

            nmap[3*index + 0] = (char)(nX*127);
            nmap[3*index + 1] = (char)(nY*127);
            nmap[3*index + 2] = (char)(nZ*127);
			
		
            //fprintf(stderr, "normal map[%d](%d, %d, %d)\n", index, nmap[3*index + 0], nmap[3*index + 1], nmap[3*index + 2]);
            index++;
        }
    }
    return nmap;
}


void GenericImage::gen_normalMap(const char* filename)
{
    GenericImage dst;
    dst.width = width;
    dst.height = height;
    dst.components = 3;
    dst.pixels = (unsigned char*)malloc(width * height * 3);
	
    char* nmap = gen_normal();

    //cast from char to unsigned char to save as an normalmap image
    //and be able to look at the normalmap and make sense

    int index=0;
    unsigned char* p = (unsigned char*)dst.pixels;

    for(int y = 0; y < height; y++)
    {
        for(int x = 0; x < width; x++)
        {
            p[3*index + 0] = (unsigned char)(nmap[3*index + 0] + 127);
            p[3*index + 1] = (unsigned char)(nmap[3*index + 1] + 127);
            p[3*index + 2] = (unsigned char)(nmap[3*index + 2] + 127);
			
            //fprintf(stderr, "normal map[%d](%d, %d, %d)\n", index, nmap[3*index + 0], nmap[3*index + 1], nmap[3*index + 2]);
            index++;
        }
    }

    bool ret = dst.save(filename);
    if(ret) printf("success! normalmap saved to image %s\n", filename);
    else printf("error! normalmap save to image %s failed\n", filename);
}


//----------------------------------------------------------------------

GLuint GenericImage::resolve_internal_format(int c, int b)
{
    switch (b)
    {
    case 2:
        switch (c)
        {
#if defined(__linux__) || defined(__WIN32)
        case  1: return GL_ALPHA16; //GL_LUMINANCE16; 
        case  2: return GL_LUMINANCE16_ALPHA16; 
#elif defined(__APPLE__)
        case  1: return GL_R16;
        case  2: return GL_RG16;
#endif
        case  3: return GL_RGB16;
        default: return GL_RGBA16;
        }

    case 4:
    case 8:
        switch (c)
        {
#if defined(__WIN32)
        case  1: return GL_FLOAT_R_NV; 
        case  2: return GL_FLOAT_RG_NV; 
#elif defined(__linux__)
        case 1: return GL_ALPHA32F_ARB;
        case 2: return GL_LUMINANCE_ALPHA32F_ARB; 
#elif defined(__APPLE__)
        case  1: return GL_R32F;
        case  2: return GL_RG32F;
#endif
        case  3: return GL_RGB32F_ARB;
        default: return GL_RGBA32F_ARB;
        }

    default:
        switch (c)
        {
#if defined(__linux__) || defined(__WIN32)
        case  1: return GL_ALPHA8; //GL_LUMINANCE; 
        case  2: return GL_LUMINANCE8_ALPHA8; 
#elif defined(__APPLE__)
        case  1: return GL_R8;
        case  2: return GL_RG8;
#endif
        case  3: return GL_RGB8;
        default: return GL_RGBA8;
        }
    }
}

GLuint GenericImage::resolve_external_format(int c)
{
    switch (c)
    {
    case  1: return GL_ALPHA; //GL_LUMINANCE;
    case  2: return GL_LUMINANCE_ALPHA;
    case  3: return GL_RGB;
    default: return GL_RGBA;
    }
}


GLuint GenericImage::resolve_external_type(int b)
{
    switch (b)
    {
    case  2: return GL_UNSIGNED_SHORT;
    case  4: return GL_FLOAT;
    case  8: return GL_DOUBLE;
    default: return GL_UNSIGNED_BYTE;
    }
}

GLuint GenericImage::resolve_tex_target(int w, int h)
{
    return GL_TEXTURE_RECTANGLE_ARB;
}


void GenericImage::transform(int c, GLuint t, float s, float tr)
{
    std::cout << __FILE__ << ":" << __func__ << "\n";

    GLuint datatype = resolve_external_type(pix_depth);

    if(c == components && t == datatype) return;


    //now only handle t = GL_FLOAT
    assert(t == GL_FLOAT);

    unsigned char* ubytep = (unsigned char*)pixels;
    short* shortp = (short*)pixels;

    int size;
    float value=0.0;
    void *trd = NULL;
    float* floatp_;

    switch(t)
    {
    case GL_FLOAT:
        std::cout << "transform img to " << c << " channels" << " ,GL_FLOAT\n";

        size = width * height * c * sizeof(float);
        trd = malloc(size);
        assert(trd != NULL);
        floatp_ = (float*)trd;

        for(int i=0; i<width * height; i++) 
        {
            if(pix_depth==1) value = (float)ubytep[i]/255.0; 
            else if(pix_depth==2) value = (float)shortp[i]/32767.0;
           
            for(int j=0; j<c; j++) floatp_[c*i + j] = s * value + tr;        
        }

        pix_depth = 4;
        break;
        
    default:
        assert(0);
    }

    components = c;
    free(pixels);
    pixels = trd;
}



tex_unit* GenericImage::setup_texture(GLuint target)
{
    std::cout << __FILE__ << ":" << __func__ << "\n";

    tex_unit* tex = new tex_unit;
    tex->setformat(target,
                   resolve_internal_format(components, pix_depth),
                   resolve_external_format(components),
                   resolve_external_type(pix_depth));

    printf("set texture for image (%d, %d, %d, %d)\n", 
           width, height, components, pix_depth);
    tex->create(width, height, 0, (GLvoid*)pixels);
    return tex;
}


fbo_unit* GenericImage::setup_fbo(GLuint target)
{
    std::cout << __FILE__ << ":" << __func__ << "\n";

    fbo_unit* f = new fbo_unit;
    f->setformat(target,
                   resolve_internal_format(components, pix_depth),
                   resolve_external_format(components),
                   resolve_external_type(pix_depth));

    printf("set fbo for image (%d, %d, %d, %d)\n", 
           width, height, components, pix_depth);
    f->init(width, height, 1, false, (GLvoid*)pixels);
    return f;
}

buffer_object* GenericImage::setup_vbo()
{
    std::cout << __FILE__ << ":" << __func__ << "\n";

    buffer_object* f = new buffer_object;

    printf("set vbo for image (%d, %d, %d, %d)\n", 
           width, height, components, pix_depth);

    f->init(GL_ARRAY_BUFFER_ARB, width * height, 
            GL_DYNAMIC_DRAW, components,
            resolve_external_type(pix_depth), pixels);

    return f;
}


void GenericImage::update_texture_subimage(tex_unit* tex,
                                           GLuint extformat,
                                           GLuint datatype)
{
    std::cout << __FILE__ << ":" << __func__ << "\n";

    GLuint e = resolve_external_format(components);
    GLuint t = resolve_external_type(pix_depth);
    assert(e == extformat && t == datatype);
    assert(tex->get_width() == width);
    assert(tex->get_height() == height);

    tex->update_subimage(0, 0, width, height, pixels);    
}
