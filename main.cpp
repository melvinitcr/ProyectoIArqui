/* 
 * File:   main.cpp
 * Author: melvin
 *
 * Created on September 21, 2015, 11:58 AM
 */

#include <FreeImage.h>
#include <omp.h>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <string.h>
using namespace std;

#define BPP 32 //Bits per pixel 
FIBITMAP* bitmap;
FIBITMAP * new_bitmap;

/**
 * Multiplica dos numeros complejos 
 * @param Z1
 * @param Z2
 * @param result
 */
void multComplex(float *Z1, float *Z2, float *result) {
    result[0] = Z1[0] * Z2[0] - Z1[1] * Z2[1];
    result[1] = Z1[0] * Z2[1] + Z1[1] * Z2[0];
}

/**
 * Divide dos numeros complejos
 * @param Z1
 * @param Z2
 * @param result
 */
void divComplex(float *Z1, float *Z2, float *result) {
    float div = Z2[0] * Z2[0] + Z2[1] * Z2[1];
    float resultMULT[2];
    Z2[1] = Z2[1] * -1;
    multComplex(Z1, Z2, resultMULT);
    result[0] = resultMULT[0] / div;
    result[1] = resultMULT[1] / div;
}

/**
 * Realiza el mapeo de Z a W
 * @param ZX
 * @param result
 */
void mapper(float *ZX, float *result) {

    float Z[2] = {ZX[0], ZX[1]};
    float C[2] = {0.003, 0};
    float resultMULT[2];
    multComplex(C, Z, resultMULT);
    resultMULT[0] += 1;
    resultMULT[1] += 1;
    float A[2] = {2.1, 2.1};
    float resultMULT2[2];
    multComplex(A, Z, resultMULT2);
    divComplex(resultMULT2, resultMULT, result);

}

/**
 * Realiza el mapeo inverso de W a Z
 * @param ZX
 * @param result
 */
void inverseMapper(float *ZX, float *result) {

    float W[2] = {ZX[0], ZX[1]};
    float C[2] = {0.003, 0};
    float resultMULT[2];
    multComplex(C, W, resultMULT);
    resultMULT[0] += -2.1;
    resultMULT[1] += -2.1;
    float D[2] = {-1, -1};
    float resultMULT2[2];
    multComplex(D, W, resultMULT2);
    divComplex(resultMULT2, resultMULT, result);
    //    cout << "A: " << resultMULT[0] << " B: " << resultMULT[1] << "\n";
}

/**
 * Evalua si un color es negro en su totalidad
 * @param color0
 * @return 
 */
int isBlack(RGBQUAD *color0) {
    if (color0->rgbBlue == 0 | color0->rgbGreen == 0 | color0->rgbRed == 0) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * Dada la coordenada de un pixel obtiene un color promedio apartir de los
 * pixeles aldededor de la coordenada
 * @param a
 * @param b
 * @param color
 */
void suavizar(int a, int b, RGBQUAD *color) {


    RGBQUAD color1;
    FreeImage_GetPixelColor(bitmap, a + 1, b + 1, &color1);

    RGBQUAD color2;
    FreeImage_GetPixelColor(bitmap, a - 1, b + 1, &color2);

    RGBQUAD color3;
    FreeImage_GetPixelColor(bitmap, a + 1, b - 1, &color3);

    RGBQUAD color4;
    FreeImage_GetPixelColor(bitmap, a - 1, b - 1, &color4);


    if (isBlack(&color1) | isBlack(&color2) | isBlack(&color3) | isBlack(&color4)) {
        FreeImage_GetPixelColor(bitmap, a, b, color);
    } else {

        int red = color1.rgbRed + color2.rgbRed + color3.rgbRed + color4.rgbRed;
        int green = color1.rgbGreen + color2.rgbGreen + color3.rgbGreen + color4.rgbGreen;
        int blue = color1.rgbBlue + color2.rgbBlue + color3.rgbBlue + color4.rgbBlue;

        color->rgbGreen = green / 4;
        color->rgbBlue = blue / 4;
        color->rgbRed = red / 4;
    }
}

char foto[99];

int main(int argc, char** argv) {


    if (argc <= 2) {
        exit(0);
    }

    bzero(foto, 99);
    // printf("%s\n", argv[2]);
    strcpy(foto, argv[2]);

    int count = atoi(argv[4]);

    if (count == 1) {
        printf("Ejecucion,Tiempo\n");
    }

    double start_time, run_time;
    start_time = omp_get_wtime();

    FreeImage_Initialise();
    atexit(FreeImage_DeInitialise);

    //strcpy(foto, "sample.png");

    FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(foto, 0);
    bitmap = FreeImage_Load(formato, foto);


    FIBITMAP* temp = FreeImage_ConvertTo32Bits(bitmap);


    int width = FreeImage_GetWidth(temp);
    int height = FreeImage_GetHeight(temp);


    FreeImage_Unload(bitmap);


    bitmap = temp;

    float Zwidth[2] = {(float) width, 0};
    float Zheight[2] = {0, (float) height};
    float resultwidth[2];
    float resultheight[2];


    mapper(Zwidth, resultwidth);
    mapper(Zheight, resultheight);

    RGBQUAD color;
    color.rgbGreen = 0;
    color.rgbBlue = 0;
    color.rgbRed = 0;


    new_bitmap = FreeImage_Allocate(width, height, BPP);


//#pragma omp simd
    for (float i = 0; i < width; i++) {
        for (float j = 0; j < height; j++) {

            float Z[2] = {i, height - j};
            float resultMap[2];
            inverseMapper(Z, resultMap);

            if (resultMap[0] < 0 | resultMap[1] < 0 | resultMap[0] > width | resultMap[1] > height) {
                FreeImage_SetPixelColor(new_bitmap, i, j, &color);
            } else {
                RGBQUAD color;
                suavizar(resultMap[0], height - resultMap[1], &color);
                FreeImage_SetPixelColor(new_bitmap, i, j, &color);
            }
        }
    }

    FIBITMAP * salida = FreeImage_Allocate(resultwidth[0], resultheight[1], BPP);


    for (int i = 0; i < resultwidth[0]; i++) {
        for (int j = 0; j < resultheight[1]; j++) {
            RGBQUAD color;
            FreeImage_GetPixelColor(new_bitmap, i, height - j, &color);
            FreeImage_SetPixelColor(salida, i, resultheight[1] - j, &color);
        }
    }

    FreeImage_Save(FIF_BMP, salida, "output.bmp");
    FreeImage_Unload(bitmap);

    run_time = omp_get_wtime() - start_time;
    printf("%d,%lf\n", count, run_time);


    return 0;
}
