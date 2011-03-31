
#include "colorkey.h"
#include "scanline.h"
#include "footprint.h"

const unsigned int ColorKey::keyLen = 2;

FootPrint fp;

int main()
{
    ScanLine s1, s2;

    int segsize = 10;

    unsigned int segs1[] = {1, 3, 4, 8, 9, 13, 14, 34, 35, 76};
    s1.set_seg(segs1, segs1+segsize);

    unsigned int segs2[] = {6, 10, 11, 17, 18, 36, 37, 46, 47, 56};
    s2.set_seg(segs2, segs2+segsize);

    unsigned int ck1[] = {0,1, 0,2, 0,4, 0,8, 0,16};
    unsigned int ck2[] = {64,0, 128,0, 256,0, 512,0, 1024,0};

    for(int i=0; i<segsize/2; i++)
    {
        ColorKey k1(ck1 + ColorKey::keyLen * i);
        s1.add_key(k1);
        ColorKey k2(ck2 + ColorKey::keyLen * i);
        s2.add_key(k2);           
    }

    ScanLine S;
    S = s1;
    S += s2;

    S.print();
    S.print_key();

    return 0;
}

