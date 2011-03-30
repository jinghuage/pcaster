#extension GL_ARB_texture_rectangle : enable  

uniform sampler2DRect tex;
uniform int numofpieces;
uniform float offset;
//uniform sampler2D tex;
//uniform vec2 vpxy;

void main()
{
    //vec4 color = texture2D(tex,gl_TexCoord[0].st);
    vec2 texcoord = gl_FragCoord.xy;
    vec4 color0 = texture2DRect(tex, texcoord);
    vec4 color = color0;

    for(int i=1; i<numofpieces; i++)
    {
        texcoord = gl_FragCoord.xy + i * vec2(0., offset);
        vec4 color1 = texture2DRect(tex, texcoord);
        //color = (1.0-color.a) * color1 + color;
        color += color1;
    }
    
    gl_FragColor = color;
    //gl_FragColor.rgb = color0.rgb + color0.a * color1.rgb;
    //gl_FragColor.a = color0.a * color1.a;
}

