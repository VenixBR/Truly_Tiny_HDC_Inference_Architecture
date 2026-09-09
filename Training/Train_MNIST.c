#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define IMAGE_SIZE (28*28)
#define NUM_CLASSES 10
#define DIMENSIONS  8192
//#define DIMENSIONS  32
#define FEATURE_LEVELS 255
#define FEATURES (28 * 28)
#define CLASSES 10


// Loader
uint32_t read_uint32_be(FILE *fp) {
    uint8_t buffer[4];

    fread(buffer, 1, 4, fp);

    return ((uint32_t)buffer[0] << 24) |
           ((uint32_t)buffer[1] << 16) |
           ((uint32_t)buffer[2] << 8)  |
           ((uint32_t)buffer[3]);
}

// Load Images
uint8_t *load_images(const char *filename, int *num_images)
{
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL) {
        printf("Erro ao abrir %s\n", filename);
        return NULL;
    }

    uint32_t magic = read_uint32_be(fp);
    uint32_t n     = read_uint32_be(fp);
    uint32_t rows  = read_uint32_be(fp);
    uint32_t cols  = read_uint32_be(fp);

    // printf("Magic: %u\n", magic);
    // printf("Images: %u\n", n);
    // printf("Rows: %u\n", rows);
    // printf("Cols: %u\n", cols);

    *num_images = n;

    uint8_t *images = malloc(n * rows * cols);

    if (images == NULL) {
        fclose(fp);
        return NULL;
    }

    fread(images, 1, n * rows * cols, fp);

    fclose(fp);

    return images;
}

// Load Labels
uint8_t *load_labels(const char *filename, int *num_labels)
{
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL) {
        printf("Erro ao abrir %s\n", filename);
        return NULL;
    }

    uint32_t magic = read_uint32_be(fp);
    uint32_t n     = read_uint32_be(fp);

    // printf("Magic: %u\n", magic);
    // printf("Labels: %u\n", n);

    *num_labels = n;

    uint8_t *labels = malloc(n);

    if (labels == NULL) {
        fclose(fp);
        return NULL;
    }

    fread(labels, 1, n, fp);

    fclose(fp);

    return labels;
}

// Return the address of image i of MNIST
uint8_t *get_image_or_label ( char *begin, int i) {
    return begin + i*IMAGE_SIZE;
}

// Return the i bit of a byte
uint8_t get_bit(uint8_t the_byte,  int index){
    uint8_t t = the_byte<<(index);
    return t>>7;
}

// Get a bit of a unit8_t vector
uint8_t get_bit_vector(uint8_t *vector, int i){
    int byte = i/8;         // Obtain the byte of the index
    int new_i = i-byte*8;   // Obtain the index inside the byte
    return get_bit(vector[byte], new_i);
}

// set a bit of a unit8_t vector
void set_bit_vector(uint8_t *vector, int i, uint8_t value){
    if(value > 1 || value < 0){
        printf("ERROR: the informed value not is binary\n");
        exit(EXIT_FAILURE);
    }


    int byte = i/8;         // Obtain the byte of the index
    int new_i = i-byte*8;   // Obtain the index inside the byte

    // Multiply the bits by yours weights
    uint8_t b[8];
    for(int i=0 ; i<8 ; i++){
        b[i] = get_bit(vector[byte], i)<<(7-i); // olhar
    } //-->

    // Set the bit
    b[new_i] = value<<(7-new_i);
   
    // Accumulate the bits*weights
    uint8_t new_byte = 0;
    for(int i=0 ; i<8 ; i++){
        new_byte += b[i];
    }
    
    vector[byte] = new_byte;
}

uint8_t xor_reduction(uint16_t value, int b15,int b14,int b13,int b12,int b11,int b10,int b9,int b8,int b7,int b6,int b5,int b4,int b3,int b2,int b1,int b0){

    uint8_t hi = value>>8;
    uint8_t lo = value;

    uint8_t b[16];

    for (int i=0 ; i<8 ; i++){
        b[7-i] = get_bit(lo, i);
        b[15-i] = get_bit(hi, i);
    }

    b[15] = b[15] & b0 ;
    b[14] = b[14] & b1;
    b[13] = b[13] & b2;
    b[12] = b[12] & b3;
    b[11] = b[11] & b4;
    b[10] = b[10] & b5;
    b[9] = b[9] & b6;
    b[8] = b[8] & b7;
    b[7] = b[7] & b8;
    b[6] = b[6] & b9;
    b[5] = b[5] & b10;
    b[4] = b[4] & b11;
    b[3] = b[3] & b12;
    b[2] = b[2] & b13;
    b[1] = b[1] & b14;
    b[0] = b[0] & b15;


    return b[0]^b[1]^b[2]^b[3]^b[4]^b[5]^b[6]^b[7]^b[8]^b[9]^b[10]^b[11]^b[12]^b[13]^b[14]^b[15];
}

// Implements the roll function
int roll(int x, int shift)
{
    int bits = sizeof(int) * 8;

    shift %= bits;

    //right
    if (shift > 0) {
        return (x >> shift) | (x << (bits - shift));
    }
    // Left
    else if (shift < 0) {
        shift = -shift;
        return (x << shift) | (x >> (bits - shift));
    }
    return x;
}

// Implements the roll function
int roll_16(uint16_t x, int shift){
    
    uint8_t bin[16];
    for (int i = 0; i < 16; i++) {
        bin[15 - i] = x & 1;
        x >>= 1;
    }

     uint8_t output[16];
    // for (int i = 0; i < 16; i++) {
    //     output[i] = bin[(i + 16) % 16];
    // }

    uint8_t temp;
    for (int i=0 ; i<shift ; i++){
        temp = bin[0];
        for(int j=0 ; j<15 ; j++){
            bin[j] = bin[j+1];
        }
        bin[15] = temp;
    }

    int result=0;
    for(int i=0 ; i<16 ; i++)
        result += bin[15-i]<<i;

    return result;

}

// Implements the right roll function in a HV
uint8_t* roll_HV(uint8_t *vector, int D, int shift){

    uint8_t *t_vector = malloc((D/8) * sizeof(uint8_t));

    for(int i=0 ; i<D-shift ; i++){
        set_bit_vector(t_vector,i+shift,get_bit_vector(vector, i));
    }

    for(int i=0 ; i<shift ; i++){
        set_bit_vector(t_vector,i,get_bit_vector(vector, D-shift+i));
    }

    return t_vector;
}

int main()
{
    int num_images;
    int num_labels;

    // Load images and labels
    uint8_t *images = load_images("../Datasets/MNIST/train-images-idx3-ubyte", &num_images);
    uint8_t *labels = load_labels("../Datasets/MNIST/train-labels-idx1-ubyte", &num_labels);

    // Abort if occurs some error in imagens or labels load
    if (images == NULL || labels == NULL) return 1;

    printf("\n[INFO] Dataset loaded!\n");



    /*#######################################
    ##        Generation of BHVs           ##
    #######################################*/

    // Create the BHVs packaging 8 bits in one byte
    uint8_t gen_seed1[DIMENSIONS/8];
    uint8_t gen_seed2[DIMENSIONS/8];
    uint8_t gen_seed3[DIMENSIONS/8];

    // Initialize the BHVs with 0 in all indexes
    for (int i=0 ; i<DIMENSIONS; i++){
        set_bit_vector(gen_seed1, i, 0);
        set_bit_vector(gen_seed2, i, 0);
        set_bit_vector(gen_seed3, i, 0);
    }

    // Generate the bits of BHVs
    int rolled_i;
    for (uint16_t i=0 ; i<DIMENSIONS ; i++){
        rolled_i = roll_16(i, (i%16));
        set_bit_vector(gen_seed1, i, xor_reduction(rolled_i,0,0,0,0,1,1,0,1,0,0,0,0,1,1,1,0));
        set_bit_vector(gen_seed2, i, xor_reduction(rolled_i,1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,0));
        set_bit_vector(gen_seed3, i, xor_reduction(rolled_i,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1));
    }

    // Print the BHVs
    printf("\n\n[INFO] BHVs Generated!");
    printf("\n\ngen_seed1 = ");
    for (int i=0 ; i<DIMENSIONS ; i++){
        printf("%b",get_bit_vector(gen_seed1, i));
    }
    printf("\n\ngen_seed2 = ");
    for (int i=0 ; i<DIMENSIONS ; i++){
        printf("%b",get_bit_vector(gen_seed2, i));
    }
    printf("\n\ngen_seed3 = ");
    for (int i=0 ; i<DIMENSIONS ; i++){
        printf("%b",get_bit_vector(gen_seed3, i));
    }


    /*#######################################
    ##             Level-ID HVs            ##
    #######################################*/

    uint8_t *level_hvs_bin [FEATURE_LEVELS];
    uint8_t *x_hvs_bin [28];
    uint8_t *y_hvs_bin [28];

    for (int i=0 ; i<FEATURE_LEVELS ; i++)
        level_hvs_bin[i] = roll_HV(gen_seed1, DIMENSIONS, i);
    
    for (int i=0 ; i<28 ; i++){
        x_hvs_bin[i] = roll_HV(gen_seed2, DIMENSIONS, i);
        y_hvs_bin[i] = roll_HV(gen_seed3, DIMENSIONS, i);
    }

    printf("\n\n[INFO] Lel_ID HVs Generated!");

    printf("\n\nx_hvs_bin[3] = ");
    for (int i=0 ; i<DIMENSIONS ; i++){
        printf("%b",get_bit_vector(x_hvs_bin[3], i));
    }


    printf("\n");

    printf("\n\n");



    free(images);
    free(labels);
    for (int i = 0; i < FEATURE_LEVELS; i++) {
        free(level_hvs_bin[i]);
    }
    for (int i = 0; i < 28; i++) {
        free(x_hvs_bin[i]);
        free(y_hvs_bin[i]);
    }
    return 0;
}