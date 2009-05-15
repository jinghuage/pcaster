
// gpu volume ray tracing
#extension GL_ARB_texture_rectangle : enable    

uniform sampler2DRect vpos_front; // ray enter position float texture
uniform sampler2DRect vpos_back; //ray exit position float texture
uniform sampler1D transferFunc; //transfer function
uniform sampler3D volume; //scalar volume with or without derivatives

uniform float stepSize;
//uniform float threshold;
uniform int   maxSteps;

//uniform float shininess;
//varying vec3 viewVec;

void main (void)
{
    vec3 rayEnter;
    vec3 rayExit;
    vec3 direction;
    vec3 pos;
    vec4 value, value_n, dst, src;
    //vec3 normalSF, normalSB, tnorm, lightVec, reflect;
    //float diffuse, specular;
    float rayLength, step;
    int i, steps;

    rayEnter = texture2DRect(vpos_front, gl_FragCoord.xy).rgb;
    rayExit = texture2DRect(vpos_back, gl_FragCoord.xy).rgb;
    direction = rayExit - rayEnter;
    rayLength = dot(direction, direction);
    //voxels_per_ray = rayLength * 32.;
    step = rayLength / stepSize;
    steps = int(step);
    //steps = 100;
    direction = normalize(direction);
 
    pos = rayEnter;
    //value = texture3D(volume, rayEnter);
    dst = vec4(0.0);
    //dst = texture2DRect(vpos_back, gl_FragCoord.xy);

    for(i=0; i<maxSteps && i<steps; i++)
//    for(i=0; i<steps; i++)
    {
        pos += stepSize * direction;
        value = texture3D(volume, pos);

        src = texture1D(transferFunc, value.r);

        //src = vec4(value.a, value.a, value.a, value.a);
        //src = vec4(src.r, 0., 0., value.a);
        //src = step(0.5, value.a) * vec4(0.0, 1.0, 0.0, 1.0);
        //src = smoothstep(0.25, 0.75, value.a) *  vec4(0., 1., 0., 1.);
        src.rgb *= src.a;
        


        //dst += src;
        //dst = (1.0-dst.a) * src + dst*dst.a;
        //dst = mix(src, dst, dst.a);
        //dst.rgb = (1.0-dst.a) * src.a * src.rgb + dst.a * dst.rgb;
        //dst.a = (1.0-dst.a) * src.a + dst.a;
        
        dst.rgb = (1.0-dst.a) * src.rgb + dst.rgb;
        dst.a = (1.0-dst.a) * src.a + dst.a;

        //src.rgb *= src.a;   // premultiply alpha
        //dst = (1.0 - dst.a) * src + dst;


        //Dst.rgb = (1.0-dst.a) * (1.-src.a) * src.rgb + dst.a * dst.rgb;
        //dst.a = (1.0-dst.a) * (1.0-src.a) + dst.a;

        if(dst.a >= 1.0) break;
        //pos = pos_n;
        //value = value_n;
        //pos += stepSize * direction;
    }

    //dst = texture3D(volume, (rayEnter+rayExit)/2.);
    //dst = texture1D(transferFunc, rayEnter.x);
    
    gl_FragColor = dst;
    //gl_FragColor = src;
    //gl_FragColor = vec4(0., 1., 0., 1.);
    //gl_FragColor = gl_Color;
}

