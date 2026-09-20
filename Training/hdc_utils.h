#ifndef HDC_UTILS_H
#define HDC_UTILS_H

#include <stdio.h>
#include <stdint.h>

// Loader
uint32_t read_uint32_be(FILE *fp);

uint8_t *load_images(const char *filename, int *num_images);

uint8_t *load_labels(const char *filename, int *num_labels);

// MNIST
uint8_t *get_image_or_label(char *begin, int i, int IM_Size);

// Bit manipulation
uint8_t get_bit(uint8_t the_byte, int index);

uint8_t get_bit_vector(uint8_t *vector, int i);

void set_bit_vector(uint8_t *vector, int i, uint8_t value);

// XOR reduction
uint8_t xor_reduction(
    uint16_t value,
    int b15, int b14, int b13, int b12,
    int b11, int b10, int b9,  int b8,
    int b7,  int b6,  int b5,  int b4,
    int b3,  int b2,  int b1,  int b0
);

// Roll
int roll(int x, int shift);

int roll_16(uint16_t x, int shift);

uint8_t *roll_HV(uint8_t *vector, int D, int shift);

// MNIST pixel
uint8_t get_pixel(uint8_t *image, int image_i, int row, int column);

// Print
void print_HV(uint8_t *HV, int size);

#endif