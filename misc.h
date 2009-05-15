#ifndef MISC_H_
#define MISC_H_


//time
double getTimeInSecs();
//eye
void caculate_eyepos(float* headPos, float* headOri, float* leftEye, float* rightEye);

//compute
unsigned char* create_Preintegration_Table(unsigned char*);
void gaussian_filter(int, unsigned char*, int);
void add(unsigned char* src, int width, int height, int shift, unsigned char* dst);
void add(unsigned char* src0, unsigned char* src1, int width, int height, unsigned char* dst);

#endif /*MISC_H_*/
