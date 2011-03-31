#extension GL_ARB_texture_rectangle : enable
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2DRect tex;
uniform int texwid;
uniform int overlap_info[128]; //128 enough??

void main()
{    
    int ol_num = overlap_info[0];
    int npixels = 0;
//    int new_linear_pos = 0;
//    vec2 new_pos;
    int id = 0;
    vec2 pos = gl_FragCoord.xy; //gl_FragCoord is (0.5, 0.5) for pixel(0, 0)
    //linear count inside compoiste target, include all composite overlap

    vec2 pos_i = pos - 0.5;
    int linear_count = int( pos_i.y * float(texwid) + pos_i.x );
    int linear_count_ol = 0; //linear count inside current overlap


    for(id=0; id<ol_num; id++) 
    {
        linear_count_ol = linear_count - npixels;
        npixels += overlap_info[id+1];
        if(linear_count < npixels) break;
    }


    if(linear_count >= npixels) //out of range pixel set to white
    {
        gl_FragColor = vec4(1., 1., 1., 1.);
        return;
    }

    //now this pixel belong to overlap id. (start from 0)
    //so need to go retrieve info for overlap id
    int proc_num = 0;
    int n = ol_num + 1;

    for(int i=0; i<id; i++)
    {
        proc_num = overlap_info[n];
        n += (proc_num+1);
    }
    
    proc_num = overlap_info[n];

    //vec4 color0 = texture2DRect(tex, pos);
    //vec4 color = color0;
    vec4 color = vec4(0., 0., 0., 0.);
    vec4 ncolor = vec4(0., 0., 0., 0.);

    for(int i=0; i<proc_num; i++)
    {
        int offset = overlap_info[n+1+i];

        //offset need to be relative to the cbuf_offset
        //this means the origin of the comp_tex
        if(offset <= linear_count) ncolor = texture2DRect(tex, pos);
        //else ncolor = vec4(0., 0., 0., 0.);
        else
        {
            vec2 offset_2d;
            int oy = (offset + linear_count_ol) / texwid;
            int ox = (offset + linear_count_ol) % texwid;
            offset_2d.x = float(ox) + 0.5;
            offset_2d.y = float(oy) + 0.5;

            ncolor = texture2DRect(tex, offset_2d);
        }

        color = (1.0-color.a) * ncolor + color;
        //color += ncolor;
    }

    gl_FragColor = color;
    //gl_FragColor.rgb = color0.rgb + color0.a * color1.rgb;
    //gl_FragColor.a = color0.a * color1.a;
}
