//varying vec3 viewVec;

void main(void)
{
    //gl_TexCoord[0] = gl_MultiTexCoord0;
    //gl_TexCoord[1] = gl_MultiTexCoord1;

    //vec4 ec = gl_ModelViewMatrix * gl_Vertex;
    //viewVec = ec.xyz;
    gl_Position = ftransform();
}
