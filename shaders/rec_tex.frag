#extension GL_ARB_texture_rectangle : enable  

uniform sampler2DRect tex;
//uniform sampler2D tex;
uniform vec2 vp_offset;
uniform vec2 tex_offset;
uniform vec2 tex_mf;

void main()
{
    //vec4 color = texture2D(tex,gl_TexCoord[0].st);
    vec2 t = (gl_FragCoord.xy - vp_offset) * tex_mf + tex_offset;
    vec4 color = texture2DRect(tex, t);
   
    gl_FragColor = color;
    //gl_FragColor = vec4(1., 0., 1., 1.);
}
