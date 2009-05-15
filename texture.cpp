#include "texture.h"
#include "imgfile.h"


/*
//mostly for reference's purpose
static const GLenum tex_target[] = {
	GL_TEXTURE_1D,
	GL_TEXTURE_2D,
	GL_TEXTURE_3D,
	GL_TEXTURE_CUBE_MAP_ARB,
	GL_TEXTURE_RECTANGLE_ARB
};

static const GLenum cubemap_target[] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, 
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, 
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, 
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB
};

static const GLenum tex_external_format[] = {
    GL_INTENSITY,
    GL_LUMINANCE,
    GL_LUMINANCE_ALPHA,
    GL_RED,
    GL_GREEN,
    GL_BLUE,
    GL_ALPHA_INTEGER_EXT,
    GL_RGB,
    GL_RGBA,
    GL_DEPTH_COMPONENT 
};


static GLuint tex_internal_format[] = {
  //byte formats, data type should be GL_UNSIGNED_BYTE
	GL_INTENSITY,
	GL_LUMINANCE, 
	GL_LUMINANCE_ALPHA, 
	GL_RGB, 		
	GL_RGBA, 	
  
  //ARB_float formats, data type should be GL_FLOAT
	GL_RGBA32F_ARB, 
	GL_RGBA16F_ARB, 
	GL_RGB32F_ARB,  
	GL_RGB16F_ARB, 
	GL_LUMINANCE_ALPHA32F_ARB, 
	GL_LUMINANCE_ALPHA16F_ARB,  
	GL_ALPHA32F_ARB,         
	GL_INTENSITY32F_ARB,   
	GL_LUMINANCE32F_ARB,  
	GL_ALPHA16F_ARB,       
	GL_INTENSITY16F_ARB,    
	GL_LUMINANCE16F_ARB,   
  
  //ATI float Formats, data type should be GL_FLOAT
	GL_RGBA_FLOAT32_ATI,  
	GL_RGBA_FLOAT16_ATI, 
	GL_RGB_FLOAT32_ATI, 
	GL_RGB_FLOAT16_ATI, 
	GL_LUMINANCE_ALPHA_FLOAT32_ATI, 
	GL_LUMINANCE_ALPHA_FLOAT16_ATI, 
	GL_ALPHA_FLOAT32_ATI,           
	GL_ALPHA_FLOAT16_ATI,           
	GL_INTENSITY_FLOAT32_ATI,       
	GL_INTENSITY_FLOAT16_ATI,       
	GL_LUMINANCE_FLOAT32_ATI,       
	GL_LUMINANCE_FLOAT16_ATI,       
  
  //depth formats, data type should be GL_FLOAT
	GL_DEPTH_COMPONENT16,		   
	GL_DEPTH_COMPONENT24,		   
	GL_DEPTH_COMPONENT32,		   
  
  //integer texture with new GL_EXT_texture_integer formats, data type must be GL_INT
	GL_ALPHA16I_EXT, 			
  
  //compress texture
	GL_COMPRESSED_RGBA_ARB, 		
	GL_COMPRESSED_RGB_ARB
};

*/

tex_unit::tex_unit()
{
	m_width = 0;
	m_height = 0;
	m_mipmaps = 0;
//	m_pCompressedPixmap = NULL;
	m_texture_id = 0;
	
	intFormat = GL_RGBA;
	extFormat = GL_RGBA;
	dataType = GL_UNSIGNED_BYTE;
	T = GL_TEXTURE_2D;
}

tex_unit::~tex_unit()
{
// 	if(m_pCompressedPixmap)
// 	{
// 		delete [] m_pCompressedPixmap;
// 		m_pCompressedPixmap=NULL;
// 	}
}

void tex_unit::bind()
{
    glBindTexture(T, m_texture_id);
}

void tex_unit::unbind()
{
    glBindTexture(T, 0);
}

void tex_unit::setformat(GLuint t, GLuint ifmt, GLuint efmt, GLuint dtype)
{
    intFormat = ifmt;
    extFormat = efmt;
    T = t;
    dataType = dtype;
}

void tex_unit::create(int w, int h, int b,  GLvoid* p)
{
    m_width = w;
    m_height = h;

    glGenTextures(1, &m_texture_id); 
    bind();
    

    glTexImage2D(T, 0, intFormat,
                 m_width, m_height, b, 
                 extFormat, dataType, p);
    
    //printOpenGLError();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);//GL_REPLACE
	
    glTexParameteri(T, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
    glTexParameteri(T, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(T, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(T, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //note: when T is GL_TEXTURE_RECTANGLE, GL_REPEAT is not allowed
    
    unbind();
}

void tex_unit::clear()
{
    bind();
    glTexImage2D(T, 0, intFormat,
                 m_width, m_height, 0, 
                 extFormat, dataType, 0);
    unbind();
}


void tex_unit::create(int w, GLvoid* p)
{
    m_width = w;

    glGenTextures(1, &m_texture_id); 
    bind();
    
    glTexImage1D(GL_TEXTURE_1D, 0, intFormat,
                 m_width, 0, 
                 extFormat, dataType, p);
    
    //printOpenGLError();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);//GL_REPLACE
	
    glTexParameteri(T, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
    glTexParameteri(T, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(T, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(T, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //note: when T is GL_TEXTURE_RECTANGLE, GL_REPEAT is not allowed
    
    unbind();
}

void tex_unit::create(int w, int h, int d, int b, GLvoid* p)
{
    m_width = w;
    m_height = h;
    m_depth= d;

    glGenTextures(1, &m_texture_id); 
    bind();
    
    glTexImage3D(GL_TEXTURE_3D, 0, intFormat,
                 m_width, m_height, m_depth, b, 
                 extFormat, dataType, p);
    
    //printOpenGLError();
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);//GL_REPLACE
	
    glTexParameteri(T, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
    glTexParameteri(T, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(T, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(T, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(T, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    //note: when T is GL_TEXTURE_RECTANGLE, GL_REPEAT is not allowed
    
    unbind();
}


// void tex_unit::switch_data(GLvoid* p)
// {
//     bind();
//     glTexSubImage3D(GL_TEXTURE_3D, 0, 
//                     0, 0, 0,
//                     m_width, m_height, m_depth,
//                     extFormat, dataType, p);
//     printOpenGLError();
//     unbind();
// }

void tex_unit::update_subimage(int ox, int oy, int w, int h, GLvoid* p)
{
    bind();
    glTexSubImage2D(T, 0, 
                    ox, oy,
                    w, h,
                    extFormat, dataType, p);
    printOpenGLError();
    unbind();
}

void tex_unit::update_subimage(int ox, int oy, int oz, int w, int h, int d, GLvoid* p)
{
    bind();
    glTexSubImage3D(T, 0, 
                    ox, oy, oz,
                    w, h, d,
                    extFormat, dataType, p);
    printOpenGLError();
    unbind();
}


void tex_unit::setup_texture(GLvoid* p)
{
    if (m_mipmaps) 
    {
    	gluBuild2DMipmaps(T, intFormat,
                          m_width, m_height,
                          extFormat, dataType, p);

        // Set mipmaps.
        glTexParameteri( T, GL_TEXTURE_BASE_LEVEL, 0 );
        glTexParameteri( T, GL_TEXTURE_MAX_LEVEL, m_mipmaps );
	
        // Set texture trilinear filtering mode.
        glTexParameteri( T, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );	
        glTexParameteri( T, GL_TEXTURE_MAG_FILTER, GL_LINEAR );	
	
        // The texture RGB channels are modulated with the vertex diffuse RGB channels
        glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
        // Set texture addressing mode.
        glTexParameteri( T, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri( T, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    else 
    {
        glTexImage2D(T, 0, intFormat,
                     m_width, m_height, 0, 
                     extFormat, dataType, p);
    
        printOpenGLError();
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);//GL_REPLACE
	
        glTexParameteri(T, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
        glTexParameteri(T, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(T, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(T, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        //note: when T is GL_TEXTURE_RECTANGLE, GL_REPEAT is not allowed

    }
    
    //printOpenGLError();
}

/*
void tex_unit::setup_compressed_texture(GLubyte* p)
{

	int width = m_width;
	int height = m_height;
	int offset = 0;
	int size = 0;
	int mip = 0;
	int blockSize = 0;
	GLenum int_pix_format = dataType;
	GLenum internal_format = 0;
	
	for( mip=0; mip<=m_mipmaps && (width || height); mip++ )
	{
		if (width == 0)
		  width = 1;
		
		if (height == 0)
		  height = 1;
		  
		if(int_pix_format==O3_TC_RGBA_S3TC_DXT5)
		{
			blockSize = 16;
			internal_format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		}
		else if(int_pix_format==O3_TC_RGB_S3TC_DXT1)
		{
			blockSize = 8;
			internal_format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		}
	
		size = ((width+3)/4)*((height+3)/4) * blockSize;
		
		glCompressedTexImage2D( GL_TEXTURE_2D,
		mip, 
		internal_format, 
		width, 	height, 
		0, 
		size, 
		m_pCompressedPixmap + offset );
	
		offset += size;
		width >>= 1;
		height >>= 1;
	}
	
	
	// Set mipmaps.
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_mipmaps );
	
	// Set texture trilinear filtering mode.
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );	
	
	// The texture RGB channels are modulated with the vertex diffuse RGB channels
	glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
	// Set texture addressing mode.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}


void tex_unit::compress_texture(int b, GLubyte* p)
{
  
  int compressed;
  int internalformat;
  int compressed_size;
  
  int num_compressed_format;
  int * compressed_format = NULL;
  unsigned char * img = NULL;


  // Assume tightly packed textures. 
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // Lets have GL do the compression for us and choose the best matching 
  //   compression format 
  gluBuild2DMipmaps(GL_TEXTURE_2D, 
               (b == 4) ? GL_COMPRESSED_RGBA_ARB : GL_COMPRESSED_RGB_ARB, 
               m_width, m_height,
               extFormat, dataType, p);

  glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB, &num_compressed_format);
  compressed_format = (int*)malloc(num_compressed_format * sizeof(int));
  glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS_ARB, compressed_format);

  // Check if the image has been compressed by GL
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed);

  // Query for the compressed internal format 
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalformat);

  // Query for the size of the compressed data texture buffer 
  //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_IMAGE_SIZE_ARB, &compressed_size);
  
  // Allocate a buffer to host a copy of the compressed image data 
  //img = (unsigned char *)malloc(compressed_size * sizeof(unsigned char));

  // get the compressed data buffer 
  //glGetCompressedTexImageARB(GL_TEXTURE_2D, 0, img);

  // pass directly the compressed data to GL 
  // No mipmap though! 
  //glCompressedTexImage2DARB(GL_TEXTURE_2D, 0, internalformat, m_width, m_height, 0, compressed_size, img);

}

*/

