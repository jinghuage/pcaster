#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main (int argc, char** argv) {
    printf("Usage: %s <output_file_name> <input_file_names>\n", argv[0]);

    //std::string out_filename(argv[1]);
    //std::string in_filename(argv[2]);
  
    char *out_filename=(char*)malloc(128);
    char *in_filename=(char*)malloc(128);


    if(argc==3)
    {
        out_filename = argv[1];
        in_filename = argv[2];
    }
    else
    {
        sprintf(in_filename, 
                "/Users/jinghua/data/large/raw/les2048x2048x2048.raw");
        sprintf(out_filename, 
                "/Users/jinghua/data/large/raw/les1024x1024x2048.raw");
    }

    FILE* raw_input_file = fopen(in_filename, "rb");
    if (raw_input_file == NULL) {
        printf("Error creating input file %s\n", in_filename);
        perror(NULL);
        return -1;
    }

    FILE* raw_output_file = fopen(out_filename, "wb");
    if (raw_output_file == NULL) {
        printf("Error creating output file %s\n", out_filename);
        perror(NULL);
        return -1;
    }

    size_t input_slice_size = 2048 * 2048;
    size_t output_slice_size = 1024 * 1024;

    unsigned char input_slice_buffer[2048*2048];
    unsigned char output_slice_buffer[1024*1024];
    assert(input_slice_buffer);
    assert(output_slice_buffer);

    bool testing_i = false;
    bool testing_k = false;

    for(int k=0; k<2048; k++)
    {
        //read one slice from input data
        size_t readn = fread(input_slice_buffer, 1, input_slice_size, 
                              raw_input_file);
        assert(readn == input_slice_size);

        for(int j=0; j<1024; j++)
        {
            for(int i=0; i<1024; i++)
            {
                int a = j*2*2048 + i*2;
                int b = j*2*2048 + i*2+1;
                int c = (j*2+1)*2048 + i*2;
                int d = (j*2+1)*2048 + i*2+1;

                unsigned int sum = input_slice_buffer[a] + 
                    input_slice_buffer[b] + 
                    input_slice_buffer[c] +
                    input_slice_buffer[d];
                unsigned char newnum = (unsigned char)(sum/4);
          
                if(testing_i)
                {
                    printf("i, j, k, sum, newnum: %d, %d, %d, %d, %d\n",
                       i, j, k, sum, newnum);
                }
                output_slice_buffer[j*1024+i] = newnum;
            }
        }
        size_t written = fwrite(output_slice_buffer, 1, output_slice_size, 
                                 raw_output_file);
        assert(written == output_slice_size);

        int kc;
        if( testing_k) kc = getchar();
        if(kc == 's')
        {
            testing_k = false;
        }

        //fseek(raw_input_file, input_slice_size, SEEK_CUR);
        //fseek(raw_output_file, output_slice_size, SEEK_CUR);
    }

    // free(input_slice_buffer);
    //free(output_slice_buffer);
    fclose(raw_output_file);
    fclose(raw_input_file);
}
