

void main()
{

    gl_TexCoord[0] = gl_MultiTexCoord0;
    //gl_TexCoord[0] = gl_Vertex;
    gl_Position = ftransform();
} 
