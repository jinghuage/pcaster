#ifndef READ_RAWFILE_H_
#define READ_RAWFILE_H_

struct volModel
{
    int w, h, d;
    float wScale, hScale, dScale;
    float vol_scale[3];
    int TFsize;
    unsigned char *TF;
    unsigned char* preInt;
    int component;

    int blocks[3]; //how to divide blocks in x, y, z directions
    int nblocks;
    int bw, bh, bd;
    void** data_w_border;
    void** data_wt_border;
    void* wholedata;
};

volModel* init_volmodel();
int read_rawfile(char*, char*,  volModel*);

bool load_Header(const char*, volModel*);
bool load_Volume_Data(const char*, volModel*);
bool divide_volume_blocks(int, volModel*);


#endif
