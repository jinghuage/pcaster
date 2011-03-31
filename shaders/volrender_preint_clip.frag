
// gpu volume ray tracing
#extension GL_ARB_texture_rectangle : enable    

uniform sampler2DRect vpos_front; // ray enter position float texture
uniform sampler2DRect vpos_back; //ray exit position float texture
uniform sampler3D volume; //scalar volume with or without derivatives
uniform sampler2D preInt; //preintegration table
//uniform sampler1D transferFunc;


uniform float stepSize;
//uniform int   maxSteps;
uniform float multiplier;

//uniform vec4 P;           //object space clip plane 
uniform float nearPlane;    //eye space clip plane


void main (void)
{
    vec4 rayEnter, rayExit, rayCenter;
    vec3 direction, delta, pos;
    vec4 value, value_n, dst, src;
    float rayLength, s;
    int i, steps;

//lighting
/*     vec3 normalSF, normalSB, tnorm, lightVec, viewVec, rft; */
/*     float diffuse, specular, shininess = 8.0; */

    rayEnter = texture2DRect(vpos_front, gl_FragCoord.xy);
    rayExit = texture2DRect(vpos_back, gl_FragCoord.xy);
    direction = rayExit.xyz - rayEnter.xyz;
    rayLength = length(direction);
    
    s = rayLength / stepSize;
    steps = int(s) + 1;

    direction = normalize(direction);
    delta = stepSize * direction;
    
    float e1, e2;
//    e1 = dot(P, gl_TextureMatrix[0] * rayEnter);
//    e2 = dot(P, gl_TextureMatrix[0] * rayExit);

    //eye space clipping plane
    vec4 P = vec4(0., 0., 1., nearPlane); 

    //transformed ray space clipping plane
    vec4 P_ = gl_TextureMatrixInverseTranspose[0] * gl_ModelViewMatrixTranspose * P;
    e1 = dot(P_, rayEnter);
    e2 = dot(P_, rayExit);

    value = vec4(0., 0., 0., 0.);
    dst = vec4(0.0, 0.0, 0.0, 0.0);

    if(e1 > 0.0 && e2 > 0.0)
    {
        gl_FragColor = vec4(0., 0., 0., 0.);
        return;
    }
    else if(e1 <= 0.0 && e2 <= 0.0)
    {
        //gl_FragColor = vec4(0., 0., 1., 1.);
        //return;
        pos = rayEnter.xyz;
        value =  texture3D(volume, pos);
    }
    else
    {
        //gl_FragColor = vec4(0., 1., 0., 1.);
        //return;
        vec4 d = rayExit - rayEnter;
        float t = e1 / (e1-e2);
        //start compositing from the clipping plane
        if(e1 > 0.0)
        {
            pos = (rayEnter + t*d).xyz;
            value = texture3D(volume, pos);
            steps = int (length( (1.-t) * d.xyz ) / stepSize) + 1;
        }
        else if(e1 < 0.0)
        {
            pos = rayEnter.xyz;
            value = texture3D(volume, pos);
            steps = int (length( t * d.xyz ) / stepSize) + 1;
        }

//only show the 2D clipping slice
/*         pos = (rayEnter + t*d).xyz; */
/*         value = texture3D(volume, pos); */
/*         dst = texture1D(transferFunc, value.a); */
/*         gl_FragColor = dst; */
/*         return; */
    }

    for(i=0; i<steps; i++)
    {
        pos += delta;
        value_n = texture3D(volume, pos);

        src = texture2D(preInt, vec2(value_n.a, value.a));    
        //src = texture1D(transferFunc, value.a);

/*         //lighting */
/*         normalSF = value.rgb - 0.5; */
/*         normalSB = value_n.rgb - 0.5; */

/*         //computed normal is inward?? */
/*         tnorm = -normalize(gl_NormalMatrix * (normalSF + normalSB)); */
/*         lightVec = normalize(gl_LightSource[0].position.xyz); */

/*         rft = reflect( -lightVec, tnorm); */
/*         diffuse = max( dot(lightVec, tnorm ), 0.0); */
/*         //specular = max ( dot(gl_LightSource[0].halfVector, tnorm), 0.0); */
/*         viewVec = vec3(0., 0., 1.); */
/*         specular = pow(max(dot(rft, viewVec), 0.0), shininess); */
/*         //specular = pow(specular, shininess); */
/*         src = vec4(gl_LightSource[0].ambient.rgb * src.rgb + */
/*                    gl_LightSource[0].diffuse.rgb * src.rgb * diffuse + */
/*                    gl_LightSource[0].specular.rgb * src.rgb * specular, */
/*                    src.a); */

        src.rgb *= multiplier;
//        dst += src;
        dst = (1.0 - dst.a) * src + dst;   

//early termination, optional (even slower sometime)        
//        if (dst.a > 1.0) break;

        value = value_n;
    }
    
    gl_FragColor = dst;
    //gl_FragColor = vec4(0., 1., 0., 1.);
    //gl_FragColor = gl_Color;
}

