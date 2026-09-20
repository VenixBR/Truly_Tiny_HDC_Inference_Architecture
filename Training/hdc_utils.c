#include "hdc_utils.h"

#include <stdlib.h>

// Loader

uint32_t read_uint32_be(FILE *fp)
{
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

    (void)magic;

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

    (void)magic;

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

uint8_t *get_image_or_label(char *begin, int i, int IM_Size)
{
    return begin + i * IM_Size;
}


// Return the i bit of a byte

uint8_t get_bit(uint8_t the_byte, int index)
{
    uint8_t t = the_byte << index;

    return t >> 7;
}


// Get a bit of a uint8_t vector

uint8_t get_bit_vector(uint8_t *vector, int i)
{
    int byte = i / 8;
    int new_i = i - byte * 8;

    return get_bit(vector[byte], new_i);
}


// Set a bit of a uint8_t vector

void set_bit_vector(uint8_t *vector, int i, uint8_t value)
{
    if (value > 1) {
        printf("ERROR: the informed value is not binary\n");
        exit(EXIT_FAILURE);
    }

    int byte = i / 8;
    int new_i = i - byte * 8;

    uint8_t b[8];

    for (int i = 0; i < 8; i++) {
        b[i] = get_bit(vector[byte], i) << (7 - i);
    }

    b[new_i] = value << (7 - new_i);

    uint8_t new_byte = 0;

    for (int i = 0; i < 8; i++) {
        new_byte += b[i];
    }

    vector[byte] = new_byte;
}


// XOR reduction

uint8_t xor_reduction(
    uint16_t value,
    int b15, int b14, int b13, int b12,
    int b11, int b10, int b9,  int b8,
    int b7,  int b6,  int b5,  int b4,
    int b3,  int b2,  int b1,  int b0)
{
    uint8_t hi = value >> 8;
    uint8_t lo = value;

    uint8_t b[16];

    for (int i = 0; i < 8; i++) {
        b[7 - i]  = get_bit(lo, i);
        b[15 - i] = get_bit(hi, i);
    }

    b[15] &= b0;
    b[14] &= b1;
    b[13] &= b2;
    b[12] &= b3;
    b[11] &= b4;
    b[10] &= b5;
    b[9]  &= b6;
    b[8]  &= b7;
    b[7]  &= b8;
    b[6]  &= b9;
    b[5]  &= b10;
    b[4]  &= b11;
    b[3]  &= b12;
    b[2]  &= b13;
    b[1]  &= b14;
    b[0]  &= b15;

    return b[0] ^ b[1] ^ b[2] ^ b[3] ^
           b[4] ^ b[5] ^ b[6] ^ b[7] ^
           b[8] ^ b[9] ^ b[10] ^ b[11] ^
           b[12] ^ b[13] ^ b[14] ^ b[15];
}


// Roll

int roll(int x, int shift)
{
    int bits = sizeof(int) * 8;

    shift %= bits;

    if (shift > 0) {
        return (x >> shift) | (x << (bits - shift));
    }
    else if (shift < 0) {
        shift = -shift;

        return (x << shift) | (x >> (bits - shift));
    }

    return x;
}


// Roll 16 bits

int roll_16(uint16_t x, int shift)
{
    uint8_t bin[16];

    for (int i = 0; i < 16; i++) {
        bin[15 - i] = x & 1;
        x >>= 1;
    }

    uint8_t temp;

    for (int i = 0; i < shift; i++) {
        temp = bin[0];

        for (int j = 0; j < 15; j++) {
            bin[j] = bin[j + 1];
        }

        bin[15] = temp;
    }

    int result = 0;

    for (int i = 0; i < 16; i++) {
        result += bin[15 - i] << i;
    }

    return result;
}


// Roll HV

uint8_t *roll_HV(uint8_t *vector, int D, int shift)
{
    uint8_t *t_vector = malloc(D / 8);

    for (int i = 0; i < D - shift; i++) {
        set_bit_vector(
            t_vector,
            i + shift,
            get_bit_vector(vector, i)
        );
    }

    for (int i = 0; i < shift; i++) {
        set_bit_vector(
            t_vector,
            i,
            get_bit_vector(vector, D - shift + i)
        );
    }

    return t_vector;
}


// Return pixel

uint8_t get_pixel(uint8_t *image, int image_i, int row, int column)
{
    int image_addr = image_i * 28 * 28;
    int pixel_address = image_addr + row * 28 + column;

    return image[pixel_address];
}


// Print HV

void print_HV(uint8_t *HV, int size)
{
    for (int i = 0; i < size; i++) {
        printf("%d", get_bit_vector(HV, i));
    }
}