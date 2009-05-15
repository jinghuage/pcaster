#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "read_rawfile.h"

#define max(a, b) (a)>(b)?(a):(b)

bool load_Header( const char* filename, volModel* vol )
{
    printf("load model header file: %s\n", filename);

    FILE* fp = fopen(filename, "r");
    if(fp == NULL) 
    {
        printf("can't read file %s\n", filename);
        return false;
    }

    fscanf(fp, "w=%d\n", &vol->w);
    fscanf(fp, "h=%d\n", &vol->h);
    fscanf(fp, "d=%d\n", &vol->d);
    fscanf(fp, "wScale=%f\n", &vol->wScale);
    fscanf(fp, "hScale=%f\n", &vol->hScale);
    fscanf(fp, "dScale=%f\n", &vol->dScale);

    char dummy[16];
    fscanf(fp, "%s\n", dummy);

    int i;
    fscanf(fp, "size=%d\n", &vol->TFsize);
    vol->TF = new unsigned char[vol->TFsize*4];

    for( i=0; i<vol->TFsize; i++ )
    {
        int r,g, b, a;
        fscanf(fp, "r=%d\n", &r);
        fscanf(fp, "g=%d\n", &g);
        fscanf(fp, "b=%d\n", &b);
        fscanf(fp, "a=%d\n", &a);
            

//         if( a < 10 )
//         {
//             printf("get rid of this entry(%d, %d, %d, %d)\n", r, g, b, a);
//             r=g=b=a=0;
//         }

        vol->TF[4*i+0] = r;
        vol->TF[4*i+1] = g;
        vol->TF[4*i+2] = b;
        vol->TF[4*i+3] = a;
        
    }
    return true;
}

//-----------------------------------------------------------------------------
// Calculates minimal power of 2 which is greater than given number
//-----------------------------------------------------------------------------
static int calcMinPow2( unsigned int size )
{
    if( size == 0 )
        return 0;
    
    size--;
    int res = 1;

    while( size > 0 )
    {
        res  <<= 1;
        size >>= 1;
    }
    return res;
}


bool load_Volume_Data(const char* filename, volModel* vol)
{
    void *wholedata;

    const int W = calcMinPow2( vol->w );
    const int H = calcMinPow2( vol->h );
    const int D = calcMinPow2( vol->d );

    // Reading of requested part of a volume
    const int comp = 1;
    assert ( comp == 1 );
    const int stride = sizeof(float);
    printf("set components to be %d\n", comp);

    wholedata = malloc(W*H*D*comp*stride);
    memset(wholedata, 0, W*H*D*comp*stride);

    FILE *fp = fopen(filename, "rb");
    if( fp == NULL )
    {
        printf("Can't open model data file %s\n", filename);
        return false;
    }

    if( W==vol->w && H==vol->h ) // width and height are power of 2
    {
        printf("width and height are power of 2\n");
        fread( wholedata, 1, W*H*D*comp*stride, fp);
        //fseek(fp, 0, SEEK_SET);
    }

    fclose(fp);
    vol->wholedata = wholedata;
    vol->component = comp;
    vol->w = W;
    vol->h = H;
    vol->d = D;

    return true;
}


bool divide_volume_blocks(int border, volModel* vol)
{

    int nbw = vol->blocks[0];
    int nbh = vol->blocks[1];
    int nbd = vol->blocks[2];
    int numofblocks = nbw * nbh * nbd;
    void **data = new void* [numofblocks];


    int bw = vol->w / nbw;
    int bh = vol->h / nbh;
    int bd = vol->d / nbd;

    const int bW = calcMinPow2( bw ) + 2*border;
    const int bH = calcMinPow2( bh ) + 2*border;
    const int bD = calcMinPow2( bd ) + 2*border;
    const unsigned int comp = vol->component;

    int n;
    for(n=0; n<numofblocks; n++)
    {
        data[n] = malloc( bW*bH*bD*comp);
        memset(data[n], 0, bW*bH*bD*comp);
    }


    const unsigned int  W4 = vol->w * comp;
    const unsigned int WH4 = vol->w * vol->h * comp;
    const unsigned int bW4 = bW * comp;
    const unsigned int bWH4 = bW * bH * comp;
    const unsigned int border4 = border * comp;


    printf("volume: w=%d, h=%d, d=%d\n", vol->w, vol->h, vol->d); 
    printf("blocks: nbw=%d, nbh=%d, nbd=%d\n", nbw, nbh, nbd); 
    printf("volume block: bW=%d, bH=%d, bD=%d\n", bW, bH, bD);
    printf("volume block: bw=%d, bh=%d, bd=%d\n", bw, bh, bd);
    printf("bW4=%d, bWH4=%d, border4=%d\n", bW4, bWH4, border4);
    printf("W4=%d, WH4=%d\n", W4, WH4);
    
    
    int ibw, ibh, ibd, i, j;
    int offset1, offset2, size;
    int w_l, w_r, h_d, h_u, d_b, d_f, vol_h, vol_d;



    for(ibd=0; ibd<nbd; ibd++)
    {
        for(ibh=0; ibh<nbh; ibh++)
        {
            for(ibw=0; ibw<nbw; ibw++)
            {
                printf("ibd,ibh,ibw=(%d, %d, %d)\n", ibd, ibh, ibw);
                n = nbw * nbh * ibd + nbw * ibh + ibw;
                printf("block %d\n", n);
                
                w_l = ibw * bw - border;
                w_r = w_l + bW - 1;
                h_d = ibh * bh - border;
                h_u = h_d + bH - 1;
                d_b = ibd * bd - border;
                d_f = d_b + bD - 1;
                printf("w(%d, %d), h(%d, %d), d(%d, %d)\n", 
                       w_l, w_r, h_d, h_u, d_b, d_f);

                for(i=0; i<bD; i++)
                {
                    vol_d = i + d_b;
                    if( vol_d < 0 || vol_d >= vol->d ){ continue; }
                    

                    for(j=0; j<bH; j++)
                    {                      
                        vol_h = j + h_d;
                        if( vol_h <0 || vol_h >=vol->h )
                        {
                            continue;
                        }
                        
                        //w_r = bW - 1 + w_l;
                        printf("i=%d, j=%d, vol_d=%d, vol_h=%d\n", 
                               i, j, vol_d, vol_h);
                        offset1 = vol_d * WH4 + vol_h * W4 + w_l * comp;
                        offset2 = i * bWH4 + j * bW4;
                        size = (w_r - w_l + 1) * comp;
                        if(w_l < 0)
                        {
                            offset1 += border4;
                            offset2 += border4;
                            size -= border4;
                        }
                        if(w_r >= vol->w)
                        {
                            size -= border4;
                        }
  
                        memcpy((char*)(data[n]) + offset2, 
                               (char*)(vol->wholedata) + offset1, size);
                    }
 
                }
            }
        }
    }


    if(border==0)
    {
        vol->bw = bW;
        vol->bh = bH;
        vol->bd = bD;
    }
    if(border>0) vol->data_w_border = data;
    else if(border==0) vol->data_wt_border = data;
    vol->nblocks = numofblocks;
    
    return true;
}

volModel* init_volmodel()
{
    volModel *vol = new volModel;

    vol->wScale = 1.0;
    vol->hScale = 1.0;
    vol->dScale = 1.0;

    vol->vol_scale[0] = vol->vol_scale[1] = vol->vol_scale[2] = 1.0;

    vol->blocks[0] = vol->blocks[1] = vol->blocks[2] = 1;
    vol->nblocks = 1;
    
    return vol;
}

int read_rawfile(char* volfile, char* vhffile, volModel* vol)
{

    load_Header(vhffile, vol);

    int max_dim = max(max(vol->w, vol->h), vol->d);
    float s0, s1, s2;
    s0 =  vol->wScale * vol->w / max_dim;
    s1 =  vol->hScale * vol->h / max_dim;
    s2 =  vol->dScale * vol->d / max_dim;

    float max_s = max(max(s0, s1), s2);
    float vol_scale[3];
    vol_scale[0] =  s0 / max_s;
    vol_scale[1] =  s1 / max_s;
    vol_scale[2] =  s2 / max_s;

    printf("volume scale(%f, %f, %f)\n", vol_scale[0], vol_scale[1], vol_scale[2]);
    vol->vol_scale[0] = vol_scale[0];
    vol->vol_scale[1] = vol_scale[1];
    vol->vol_scale[2] = vol_scale[2];

    //load the whole volume
    load_Volume_Data(volfile, vol);

// //-----------------------------------------------------------------------------
// //set the volModel's block info
// //----------------------------------------------------------------------------- 
//     vol->blocks[0] = nbw;
//     vol->blocks[1] = nbh;
//     vol->blocks[2] = nbd;
//     vol->nblocks = nbw*nbh*nbd;

// //-----------------------------------------------------------------------------
// //two sets of data blocks available at the same time, for easy switching
// //-----------------------------------------------------------------------------    
//     divide_volume_blocks(0);
//     divide_volume_blocks(border);

    return 0;
}

