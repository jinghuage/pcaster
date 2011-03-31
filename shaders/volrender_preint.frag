
// gpu volume ray tracing
#extension GL_ARB_texture_rectangle : enable    

uniform sampler2DRect vpos_front; // ray enter position float texture
uniform sampler2DRect vpos_back; //ray exit position float texture
uniform sampler3D volume; //scalar volume with or without derivatives
uniform sampler2D preInt; //preintegration table


uniform vec3  step_scale;
uniform float stepSize;
uniform int   maxSteps;
uniform float multiplier;
uniform vec3  eyePos;

void main (void)
{
    vec3 rayEnter;
    vec3 rayExit;
    vec3 direction, delta;
    vec3 pos;
    vec4 value, value_n, dst, src;
    float rayLength, s;
    int i, steps;


    if( (eyePos.x > 0. && eyePos.x < 1.) &&
        (eyePos.y > 0. && eyePos.y < 1.) &&
        (eyePos.z > 0. && eyePos.z < 1.) )
    {
        rayEnter = eyePos;
        //gl_FragColor = vec4(eyePos, 1.0);
        //return;
    }
    else rayEnter = texture2DRect(vpos_front, gl_FragCoord.xy).rgb;


    //rayEnter = texture2DRect(vpos_front, gl_FragCoord.xy).rgb;
    rayExit = texture2DRect(vpos_back, gl_FragCoord.xy).rgb;
    

    direction = rayExit - rayEnter;
    rayLength = length(direction);
    
    direction = normalize(direction);

    //step_scale is used when data is not cube and ray stepping should be 
    //an-isotropic. Since texture space is always cube, stepping by default is 
    //uniform in each direction
    float newstepSize = stepSize * length(step_scale * direction);

    s = rayLength / newstepSize;
    steps = int(s) + 1;
    
    delta = newstepSize * direction;
    pos = rayEnter;
    value = texture3D(volume, pos);

    dst = vec4(0.0, 0.0, 0.0, 0.0);

    for(i=0; i<maxSteps && i<steps; i++)
    //for(i=0; i<steps; i++)
    {
  
        pos += delta;
        value_n = texture3D(volume, pos);

        src = texture2D(preInt, vec2(value_n.a, value.a));    

        //dst = vec4((1.0 - dst.a) * value.a + dst.a);
        dst = (1.0 - dst.a) * src + dst;   

        value = value_n;
    }
    dst.rgb *= multiplier;
    
    gl_FragColor = dst;
    //gl_FragColor = src;
    //gl_FragColor = vec4(rayLength/1.732, 0., 0., 1.);
    //gl_FragColor = vec4(steps/300., 0., 0., 1.);
    //gl_FragColor = gl_Color;
}

