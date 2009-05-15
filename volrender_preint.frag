
// gpu volume ray tracing
#extension GL_ARB_texture_rectangle : enable    

uniform sampler2DRect vpos_front; // ray enter position float texture
uniform sampler2DRect vpos_back; //ray exit position float texture
uniform sampler2D preInt; //preintegration table
uniform sampler3D volume; //scalar volume with or without derivatives
//uniform sampler1D transferfunc;

uniform float stepSize;
uniform int   maxSteps;


void main (void)
{
    vec3 rayEnter;
    vec3 rayExit;
    vec3 direction, delta;
    vec3 pos;
    vec4 value, value_n, dst, src;
    float rayLength, s;
    int i, steps;

    rayEnter = texture2DRect(vpos_front, gl_FragCoord.xy).rgb;
    rayExit = texture2DRect(vpos_back, gl_FragCoord.xy).rgb;
    direction = rayExit - rayEnter;
    rayLength = length(direction);
    
    s = rayLength / stepSize;
    steps = int(s) + 1;

    direction = normalize(direction);
    delta = stepSize * direction;
    pos = rayEnter - 0.0*delta;
    value = texture3D(volume, pos);

    dst = vec4(0.0, 0.0, 0.0, 0.0);

    for(i=0; i<maxSteps && i<steps; i++)
    {
  
        pos += delta;
        value_n = texture3D(volume, pos);

        src = texture2D(preInt, vec2(value_n.r, value.r));        

        //dst = (1.0 - dst.a) * src + dst;
        if(false) dst += src;
	if(true) dst += value_n;
        
        //dst.rgb = src.rgb + src.a * dst.rgb;
        //dst.a = src.a * dst.a;

        value = value_n;
    }
    
    //if(all(equal(dst, vec4(0.0)))) gl_FragColor = vec4(0., 0., 1., 1.);
    //else gl_FragColor = dst;
    gl_FragColor = dst;
    //gl_FragColor = src;
    //gl_FragColor = vec4(0., 1., 0., 1.);
    //gl_FragColor = gl_Color;
}

