#extension GL_ARB_texture_rectangle : enable  

uniform sampler2DRect tex;
//uniform sampler2D tex;
uniform vec2 vpxy;
uniform float p;

void main()
{
    //vec4 color = texture2D(tex,gl_TexCoord[0].st);
    vec4 color = texture2DRect(tex, (gl_FragCoord.xy - vpxy) * p);
   
    gl_FragColor = color;
    //gl_FragColor = vec4(1., 0., 0., 1.);
}
