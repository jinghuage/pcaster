#extension GL_ARB_texture_rectangle : enable  

uniform sampler2DRect tex0;
uniform sampler2DRect tex1;
uniform sampler2DRect tex2;
uniform sampler2DRect tex3;
uniform sampler2DRect tex4;
uniform sampler2DRect tex5;
uniform sampler2DRect tex6;
uniform sampler2DRect tex7;

//uniform sampler2D tex;
//uniform vec2 vpxy;

void main()
{
    //vec4 color = texture2D(tex,gl_TexCoord[0].st);
    vec4 color0 = texture2DRect(tex0, gl_FragCoord.xy);
    vec4 color1 = texture2DRect(tex1, gl_FragCoord.xy);
    vec4 color2 = texture2DRect(tex2, gl_FragCoord.xy);
    vec4 color3 = texture2DRect(tex3, gl_FragCoord.xy);
    vec4 color4 = texture2DRect(tex4, gl_FragCoord.xy);
    vec4 color5 = texture2DRect(tex5, gl_FragCoord.xy);
    vec4 color6 = texture2DRect(tex6, gl_FragCoord.xy);
    vec4 color7 = texture2DRect(tex7, gl_FragCoord.xy);
   
    vec4 color = color0+color1+color2+color3+color4+color5+color6+color7;
    gl_FragColor = vec4(color.rgb, 1.0);
    //gl_FragColor.rgb = color0.rgb + color0.a * color1.rgb;
    //gl_FragColor.a = color0.a * color1.a;
}
