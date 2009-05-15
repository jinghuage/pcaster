//varying vec3 viewVec;
//varying vec3 lightVec;

void main(void)
{
    //gl_TexCoord[0] = gl_MultiTexCoord0;
    //gl_TexCoord[1] = gl_MultiTexCoord1;

    //viewVec and lightVec in world coord
    //vec4 ec = gl_ModelViewMatrix * gl_Vertex;
    //viewVec = gl_Vertex.xyz;
    //lightVec = (gl_ModelViewMatrixInverse * gl_LightSource[0].position - gl_Vertex).xyz;

    gl_Position = ftransform();
}
