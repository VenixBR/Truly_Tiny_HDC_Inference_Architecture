#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "hdc_utils.h"


#define IMAGE_SIZE (28*28)
#define NUM_CLASSES 10
#define DIMENSIONS  8192
#define FEATURE_LEVELS 256
#define FEATURES (28 * 28)
#define CLASSES 10
#define SCALE_FACTOR 2
#define TRAIN_SAMPLES 60000
//#define TRAIN_SAMPLES 1513



int main()
{

    /*#######################################
    ##            Loading Dataset          ##
    #######################################*/

    int num_images;
    int num_labels;

    printf("\n[INFO] Loading Dataset!");
    // Load images and labels
    uint8_t *images = load_images("../Datasets/MNIST/train-images-idx3-ubyte", &num_images);
    uint8_t *labels = load_labels("../Datasets/MNIST/train-labels-idx1-ubyte", &num_labels);

    // Abort if occurs some error in imagens or labels load
    if (images == NULL || labels == NULL) return 1;

    printf("\n[INFO] Dataset loaded!\n");

    // Print an image on terminal
    // for (int x=0 ; x<28 ; x++){
    //     for (int y=0 ; y<28 ; y++){
    //         printf("%3d ", get_pixel(images, 0, x, y));
    //     }
    //     printf("\n");
    // }

    // Print an image on terminal
    // for (int x=0 ; x<28 ; x++){
    //     for (int y=0 ; y<28 ; y++){
    //         printf("%d\n", get_pixel(images, 0, x, y));
    //     }
    // }

    //getchar();

    /*#######################################
    ##       Quantitizing the Images       ##
    #######################################*/

    printf("\n[INFO] Quantitizing the Images");

    for (int i=0 ; i<TRAIN_SAMPLES*FEATURES ; i++){
        // images[i] = round((images[i]/(255.0))*SCALE_FACTOR);
        images[i] = round((images[i]/((double)(FEATURE_LEVELS-1)))*SCALE_FACTOR);
    }

    printf("\n[INFO] Images Quantitized\n");


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
    printf("\n[INFO] Generating BHVs");
    int rolled_i;
    for (uint16_t i=0 ; i<DIMENSIONS ; i++){
        rolled_i = roll_16(i, (i%16));
        set_bit_vector(gen_seed1, i, xor_reduction(rolled_i,0,0,0,0,1,1,0,1,0,0,0,0,1,1,1,0));
        set_bit_vector(gen_seed2, i, xor_reduction(rolled_i,1,1,0,0,0,1,1,1,1,1,1,1,1,1,0,0));
        set_bit_vector(gen_seed3, i, xor_reduction(rolled_i,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1));
    }

    // Print the BHVs
    printf("\n[INFO] BHVs Generated!\n");
    // printf("\n\ngen_seed1 = ");
    // print_HV(gen_seed1, DIMENSIONS)
    // printf("\n\ngen_seed2 = ");
    // print_HV(gen_seed2, DIMENSIONS)
    // printf("\n\ngen_seed3 = ");
    // print_HV(gen_seed3, DIMENSIONS)


    /*#######################################
    ##             Level-ID HVs            ##
    #######################################*/
    printf("\n[INFO] Generating Level-ID HVs");

    uint8_t *level_hvs_bin [FEATURE_LEVELS];
    uint8_t *x_hvs_bin [28];
    uint8_t *y_hvs_bin [28];

    for (int i=0 ; i<FEATURE_LEVELS ; i++)
        level_hvs_bin[i] = roll_HV(gen_seed1, DIMENSIONS, i);
    
    for (int i=0 ; i<28 ; i++){
        x_hvs_bin[i] = roll_HV(gen_seed2, DIMENSIONS, i);
        y_hvs_bin[i] = roll_HV(gen_seed3, DIMENSIONS, i);
    }

    printf("\n[INFO] Level-ID HVs Generated!\n");

    // printf("\n\nx_hvs_bin[3] = ");
    // print_HV(level_hvs_bin[get_pixel(images,0,6,21)], DIMENSIONS)


    /*#######################################
    ##           Encoding Images           ##
    #######################################*/

    printf("\n[INFO] Encoding Images");

    uint16_t encoded [DIMENSIONS];
    uint8_t *encoded_images [TRAIN_SAMPLES];

    for (int image=0 ; image < TRAIN_SAMPLES ; image++){

        // Initialize integer encoded QHV with 0
        for (int i=0 ; i<DIMENSIONS ; i++)
            encoded[i] = 0;

        // Bind and Bundle HVs for each pixel
        for (int x=0 ; x<28 ; x++){
            for (int y=0 ; y<28 ; y++){
                for (int i=0 ; i<DIMENSIONS ; i++){
                    encoded[i] += get_bit_vector(level_hvs_bin[get_pixel(images, image, y, x)],i) ^ get_bit_vector(x_hvs_bin[x], i) ^ get_bit_vector(y_hvs_bin[y], i);
                }
            }
        }

        // Binarizes the integer encoded HV
        encoded_images[image] = malloc((DIMENSIONS/8)* sizeof(uint8_t));
        for (int i=0 ; i<DIMENSIONS ; i++) {
            set_bit_vector(encoded_images[image], i, (encoded[i]>(FEATURES/2)) ? 1 : 0);
        }
    }
    
    printf("\n\n[INFO] %d Images Encoded!", TRAIN_SAMPLES);
    

    printf("\n\nQHV for first image = ");
    print_HV(encoded_images[1512], DIMENSIONS);

    printf("\n");

    printf("\n\n");



    free(images);
    free(labels);
    for (int i = 0 ; i < FEATURE_LEVELS ; i++) {
        free(level_hvs_bin[i]);
    }
    for (int i = 0 ; i < 28 ; i++) {
        free(x_hvs_bin[i]);
        free(y_hvs_bin[i]);
    }
    for (int i = 0 ; i < TRAIN_SAMPLES ; i++) {
        free(encoded_images[i]);
    }
    return 0;
}