//imgfile.cpp
#include <math.h>
#include <string.h>
#include "imgfile.h"

static const GLuint data_type[][2] = {
	{GL_UNSIGNED_BYTE,		1},
	{GL_UNSIGNED_SHORT,		2},
	{GL_FLOAT,			4},
	{GL_INT,			4},
	{O3_TC_RGBA_S3TC_DXT5, 	        2},
	{O3_TC_RGB_S3TC_DXT1, 	        4}
};
    
GenericImage::GenericImage()
{
	width = 0;
	height = 0;
	components = 0;
	pix_type = 0;
	pixels = NULL;
}

GenericImage::~GenericImage()
{
	if(pixels)
	{
		free(pixels);
		pixels = 0;
	}
}

void GenericImage::init()
{
	
}

void GenericImage::init(unsigned char* p, int w, int h, int comp, int ptype)
{
	if(w) width = w;
	if(h) height = h;
	if(comp) components = comp;
	pix_type = ptype;
	
	if(p){
		int mapsize = width * height * components * data_type[pix_type][1];
		pixels = (unsigned char*)malloc(mapsize);
		memcpy(pixels, p, mapsize);
	}

}

#ifndef CONF_NO_PNG

#include <png.h>

static int read_png(const char *filename, GenericImage* img)
{
	int w, h, b;
	
	
    FILE       *filep = NULL;
    png_structp readp = NULL;
    png_infop   infop = NULL;
    png_bytep  *bytep = NULL;

    unsigned char *p = NULL;

    /* Initialize all PNG import data structures. */

    if (!(filep = fopen(filename, "rb"))) return 0;
    else fprintf(stderr, "fopen() file %s ok!\n", filename);
    
    if (!(readp = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        return 0;
    if (!(infop = png_create_info_struct(readp)))
        return 0;

    /* Enable the default PNG error handler. */

    if (setjmp(png_jmpbuf(readp)) == 0)
    {
        /* Read the PNG header. */

        png_init_io (readp, filep);
        png_read_png(readp, infop, PNG_TRANSFORM_STRIP_16 |
                                   PNG_TRANSFORM_PACKING, NULL);
        
        /* Extract image properties. */

        w = (int) png_get_image_width (readp, infop);
        h = (int) png_get_image_height(readp, infop);

        switch (png_get_color_type(readp, infop))
        {
        case PNG_COLOR_TYPE_GRAY:       b = 1; break;
        case PNG_COLOR_TYPE_GRAY_ALPHA: b = 2; break;
        case PNG_COLOR_TYPE_RGB:        b = 3; break;
        case PNG_COLOR_TYPE_RGB_ALPHA:  b = 4; break;
        default:                        b = 0;
        }

        /* Read the pixel data. */

        if ((bytep = png_get_rows(readp, infop)))
        {
            int i, r, c;

            /* Allocate the final pixel buffer and copy pixels there. */

            if ((p = (unsigned char *) malloc((w) * (h) * (b))))
                for (r = 0; r < (h); ++r)
                    for (c = 0; c < (w); ++c)
                        for (i = 0; i < (b); ++i)
                            p[(w)*(b)*r + (b)*c + i] =
                                (unsigned char) bytep[(h)-r-1][(b)*c + i];
        }
    }

    /* Release all resources. */

    png_destroy_read_struct(&readp, &infop, NULL);
    fclose(filep);
    
    img->width = w;
    img->height = h;
    img->components = b;
    img->pixels = p;

    return 1;
    //return p;
}

static int write_png(const char *filename, GenericImage* img)
{	
	int w = img->width;
	int h = img->height;
	int b = img->components;
	unsigned char* p = img->pixels;

    FILE       *filep = NULL;
    png_structp writep = NULL;
    png_infop   infop = NULL;
    //png_bytep  *bytep = NULL;

    /* Initialize all PNG import data structures. */

    if (!(filep = fopen(filename, "wb")))
        return -1;
    if (!(writep = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
        return -1;
    if (!(infop = png_create_info_struct(writep)))
        return -1;

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
	    switch (b)
        {
        case 1: color_type = PNG_COLOR_TYPE_GRAY; break;
        case 2: color_type = PNG_COLOR_TYPE_GRAY_ALPHA; break;
        case 3: color_type = PNG_COLOR_TYPE_RGB; break;
        case 4: color_type = PNG_COLOR_TYPE_RGB_ALPHA; break;
        default: color_type = PNG_COLOR_TYPE_RGB;
        }
	   	png_set_IHDR(writep, infop, w, h, 8, color_type,
	      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    	png_write_info(writep, infop);
    		
    	/* pack pixels into bytes */
   		png_set_packing(writep);
    		      
    	png_bytep row_pointers[h];
    	for (int k = 0; k < h; k++)
     		row_pointers[k] = (png_bytep)p + k*w*b;

		/* write out the entire image data in one call */
   		//png_write_image(writep, row_pointers);
      	/* Write a few rows at a time. */
      	//png_write_rows(writep, &row_pointers[first_row], number_of_rows);

      	/* If you are only writing one row at a time, this works */
      	for (int y = 0; y < h; y++)
      	{
         	png_write_rows(writep, &row_pointers[y], 1);
      	}

	   	png_write_end(writep, infop);
	   	
	   	//png_write_png(writep, infop, PNG_TRANSFORM_STRIP_16 |
        //                           PNG_TRANSFORM_PACKING, NULL);   
    }

    /* Release all resources. */

    png_destroy_write_struct(&writep, &infop);
    fclose(filep);

	return 0;
}
#endif /* CONF_NO_PNG */

/*---------------------------------------------------------------------------*/

#ifndef CONF_NO_JPG

extern "C"
{
#include <jpeglib.h>
}

static int read_jpg(const char *filename, GenericImage* img)
{
    unsigned char *p = NULL;
    int w, h, b;

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
        b = cinfo.output_components;

        /* Allocate the pixel buffer and copy pixels there. */

        if ((p = (unsigned char *) malloc ((w) * (h) * (b))))
        {
            int i = h - 1;

            while (cinfo.output_scanline < cinfo.output_height)
            {
                unsigned char *s = p + i * (w) * (b);
                i -= jpeg_read_scanlines(&cinfo, &s, 1);
            }
        }

        /* Finalize the decompression. */

        jpeg_finish_decompress (&cinfo);
        jpeg_destroy_decompress(&cinfo);

        fclose(fin);
  
	    img->width = w;
	    img->height = h;
	    img->pixels = p;
	    img->components = b;
	    //return p;
	    return 1;
    }
    else return 0;
}

static void write_jpg(const char *filename, GenericImage* img, float quality)
{
	FILE * fout;
	int w = img->width;
	int h = img->height;
	int b = img->components;
	unsigned char* p = img->pixels;
	
	if ((fout = fopen(filename, "wb")))
    {
  		struct jpeg_compress_struct cinfo;
  		struct jpeg_error_mgr jerr;
  
  		cinfo.err = jpeg_std_error(&jerr);
  		jpeg_create_compress(&cinfo);

  		jpeg_stdio_dest(&cinfo, fout);


		cinfo.image_width = w; 				/* image width and height, in pixels */
	 	cinfo.image_height = h;
		cinfo.input_components = b;			/* # of color components per pixel */
		cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
		
		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
		
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
    }
}

#endif /* CONF_NO_JPG */

#ifndef CONF_NO_O3TC
#define K_ERR -1
#define K_OK 0

static int read_O3TC( char *filename, GenericImage* img )
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

#endif //CONF_NO_O3TC


int GenericImage::load(char* filename)
{
    const char *ext = filename + strlen(filename) - 4;
   
    if (!strcmp(ext, ".png") || !strcmp(ext, ".PNG"))
        return read_png(filename, this);
    else if (!strcmp(ext, ".jpg") || !strcmp(ext, ".JPG"))
        return read_jpg(filename, this);
	else if (!strcmp(ext, ".o3tc") || !strcmp(ext, ".O3TC"))
        return read_O3TC(filename, this);
	else return 0;
}

void GenericImage::print()
{
	fprintf(stderr, "img size(%d, %d), %d channels\n", width, height, components);
}


void GenericImage::unload()
{
	if(pixels)
	{
		free(pixels);
		pixels = 0;
	}
	init();
}

bool GenericImage::save(char* filename)
{
    const char *ext = filename + strlen(filename) - 4;
    
    if (!strcmp(ext, ".png") || !strcmp(ext, ".PNG"))
        write_png(filename, this);
    else if (!strcmp(ext, ".jpg") || !strcmp(ext, ".JPG"))
        write_jpg(filename, this, 1.0);
	else return false;
	
    return true;
}

//---------------------------
//image processing
//---------------------------
void GenericImage::gen_normalMap(GenericImage* dst)
{
	dst->width = width;
	dst->height = height;
	dst->components = 3;
	dst->pixels = (unsigned char*)malloc(width * height * 3);
	
	const float GX[3][3] = {{-1, 0, 1},
			        {-2, 0, 2}, 
			        {-1, 0, 1}};
						    
	const float GY[3][3] = {{-1, -2, 1},
				{0,  0,  0}, 
				{1,  2,  1}};					    
	
	int x, y, i, j, nx, ny;
	float pv, dX, dY, nX, nY, nZ, oolen;
	
	unsigned char* nmap = dst->pixels;
	int index = 0;
	
	for(y = 0; y < height; y++)
 	{
 		for(x = 0; x < width; x++)
		{
			dX = 0;
			dY = 0;
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
					
					pv = (float) pixels[(ny*width + nx)*components + 0]; //if RGB format, use the red channel
					dX  += pv / 255.0f * GX[i][j];
					dY  += pv / 255.0f * GY[i][j];
				}
			}
			
			// Cross Product of components of gradient reduces to
			//nX = -dX;
			//nY = -dY;
			//nZ = 1;
			
			//for terrain
			nX = -dX;
			nY = 1;
			nZ = -dY;
			
			// Normalize
			oolen = 1.0f/((float) sqrt(nX*nX + nY*nY + nZ*nZ));
			nX *= oolen;
			nY *= oolen;
			nZ *= oolen;
			
			nmap[3*index + 0] = (unsigned char)(nX*127 + 128);
			nmap[3*index + 1] = (unsigned char)(nY*127 + 128);
			nmap[3*index + 2] = (unsigned char)(nZ*127 + 128);
			
			//fprintf(stderr, "normal map[%d](%d, %d, %d)\n", index, nmap[3*index + 0], nmap[3*index + 1], nmap[3*index + 2]);
			index++;
		}
 	}
}


void GenericImage::setup_texture(tex_unit* tex, int mipmaps, bool comp)
{
// 	if(components == 3) 
// 		tex->setformat(GL_TEXTURE_2D, GL_RGB, GL_RGB, data_type[pix_type][0]);
// 	else if(components == 4)
// 		tex->setformat(GL_TEXTURE_2D, GL_RGBA, GL_RGBA, data_type[pix_type][0]);
	
	tex->create(width, height, 0, (GLvoid*)pixels);
}
