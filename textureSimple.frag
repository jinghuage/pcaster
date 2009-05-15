#extension GL_ARB_texture_rectangle : enable  

//uniform sampler2DRect tex;
uniform sampler2D tex;
//uniform vec2 vpxy;
uniform bool m;

void main()
{
    //vec4 color = texture2D(tex,gl_TexCoord[0].st);
    //vec4 color = texture2DRect(tex, (gl_FragCoord.xy - vpxy) * 2.56);
    vec4 color = texture2D(tex, gl_TexCoord[0].xy);
    if(m)
    { 
        gl_FragColor = color;
    }
    else
    {
        gl_FragColor = vec4(color.a, 0., 0., 1.);
    }
}
