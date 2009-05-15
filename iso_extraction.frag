
// gpu volume ray tracing
#extension GL_ARB_texture_rectangle : enable    

uniform sampler2DRect vpos_front; // ray enter position float texture
uniform sampler2DRect vpos_back; //ray exit position float texture
//uniform sampler1D transferFunc; //transfer function
uniform sampler3D volume; //scalar volume with or without derivatives

uniform float stepSize;
uniform int maxSteps;
uniform float isovalue;
uniform float isothres;


void main (void)
{
    vec3 rayEnter;
    vec3 rayExit;
    vec3 direction;
    vec3 pos;
    vec4 value, value_n; //dst, src
    vec3 src, dst;
    float rayLength, s, alp;
    int i, steps;

    rayEnter = texture2DRect(vpos_front, gl_FragCoord.xy).rgb;
    rayExit = texture2DRect(vpos_back, gl_FragCoord.xy).rgb;
    direction = rayExit - rayEnter;
    rayLength = length(direction);

    s = rayLength / stepSize;
    steps = int(s)+2;

    direction = normalize(direction);
 
    pos = rayEnter;// - 5.*stepSize*direction;
    dst = vec3(0.0);

    alp=0.5;
    for(i=0; i<maxSteps && i<steps; i++)
    {
        value = texture3D(volume, pos);
        pos += stepSize * direction;


        //src = vec4(value.r, value.r, value.r, value.r);
        
        //src = step(isovalue, value.r) * vec4(0.0, 1.0, 0.0, 0.01);
        //src = smoothstep(0.25, 0.75, value.r) *  vec4(0., 1., 0., 1.);

        if(abs(value.r - isovalue) < isothres)
        { 
            src = vec3(0.0, 1.0, 0.0);
            src *= alp;
            dst += src;
            break;
        }
        

        //dst = (1.0-dst.a) * src + dst*dst.a;
        //dst = mix(src, dst, dst.a);
        
        //dst.rgb = (1.0-dst.a) * src.rgb + dst.rgb;
        //dst.a = (1.0-dst.a) * src.a + dst.a;

        //if(dst.a >= 1.0) break;
    }
    //if(steps<100) gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    //else 
    gl_FragColor = vec4(dst, 1.0); //+ vec4(0., 1., 0., 1.);
    //gl_FragColor = src;
    //gl_FragColor = vec4(0., 1., 0., 1.);
    //gl_FragColor = gl_Color;
}

