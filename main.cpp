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
using namespace std;

#define BPP 32 //Bits per pixel 
FIBITMAP* bitmap;
FIBITMAP * new_bitmap;

void multComplex(float *Z1, float *Z2, float *result) {
    result[0] = Z1[0] * Z2[0] - Z1[1] * Z2[1];
    result[1] = Z1[0] * Z2[1] + Z1[1] * Z2[0];
}

void divComplex(float *Z1, float *Z2, float *result) {
    float div = Z2[0] * Z2[0] + Z2[1] * Z2[1];
    float resultMULT[2];
    Z2[1] = Z2[1] * -1;
    multComplex(Z1, Z2, resultMULT);
    result[0] = resultMULT[0] / div;
    result[1] = resultMULT[1] / div;
}

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

void suavizar(int a, int b, RGBQUAD *color) {

    RGBQUAD color1;
    FreeImage_GetPixelColor(new_bitmap, a + 1, b, &color1);

    RGBQUAD color2;
    FreeImage_GetPixelColor(new_bitmap, a - 1, b, &color2);

    RGBQUAD color3;
    FreeImage_GetPixelColor(new_bitmap, a, b + 1, &color3);

    RGBQUAD color4;
    FreeImage_GetPixelColor(new_bitmap, a, b - 1, &color4);

    RGBQUAD color5;
    FreeImage_GetPixelColor(new_bitmap, a + 1, b + 1, &color5);

    RGBQUAD color6;
    FreeImage_GetPixelColor(new_bitmap, a - 1, b + 1, &color6);

    RGBQUAD color7;
    FreeImage_GetPixelColor(new_bitmap, a - 1, b - 1, &color7);

    RGBQUAD color8;
    FreeImage_GetPixelColor(new_bitmap, a + 1, b - 1, &color8);


    int red = color1.rgbRed + color2.rgbRed + color3.rgbRed + color4.rgbRed + color5.rgbRed + color6.rgbRed + color7.rgbRed + color8.rgbRed;
    int green = color1.rgbGreen + color2.rgbGreen + color3.rgbGreen + color4.rgbGreen + color5.rgbGreen + color6.rgbGreen + color7.rgbGreen + color8.rgbGreen;
    int blue = color1.rgbBlue + color2.rgbBlue + color3.rgbBlue + color4.rgbBlue + color5.rgbBlue + color6.rgbBlue + color7.rgbBlue + color8.rgbBlue;

    color->rgbGreen = green / 8;
    color->rgbBlue = blue / 8;
    color->rgbRed = red / 8;

}

int main(int argc, char** argv) {

    double start_time, run_time;
    start_time = omp_get_wtime();
    
    FreeImage_Initialise();
    atexit(FreeImage_DeInitialise);

    FREE_IMAGE_FORMAT formato = FreeImage_GetFileType("gon.png", 0);
    bitmap = FreeImage_Load(formato, "gon.png");


    FIBITMAP* temp = FreeImage_ConvertTo32Bits(bitmap);

    int width = FreeImage_GetWidth(temp);
    int height = FreeImage_GetHeight(temp);


    float Zwidth[2] = {width, 0};
    float Zheight[2] = {0, height};

    float resultwidth[2];
    float resultheight[2];

    mapper(Zwidth, resultwidth);
    mapper(Zheight, resultheight);

    new_bitmap = FreeImage_Allocate(resultwidth[0], resultheight[1], BPP);

    FreeImage_Unload(bitmap);
    bitmap = temp;

    for (int i = 0; i < resultwidth[0]; i++) {

        for (int j = 0; j < resultheight[1]; j++) {

            float Z[2] = {i, resultheight[1] - j};
            float resultMap[2];
            inverseMapper(Z, resultMap);

            if (resultMap[0] < 0 | resultMap[1] < 0 | resultMap[0] > width | resultMap[1] > resultheight[1]) {
                RGBQUAD color;
                color.rgbGreen = 0;
                color.rgbBlue = 0;
                color.rgbRed = 0;
                FreeImage_SetPixelColor(new_bitmap, i, j, &color);
            } else {
                RGBQUAD color;
                FreeImage_GetPixelColor(bitmap, resultMap[0], resultheight[1] - resultMap[1], &color);
                FreeImage_SetPixelColor(new_bitmap, i, j, &color);
            }
        }
    }


    for (int i = 1; i < resultwidth[0] - 2; i++) {
        for (int j = 1; j < resultheight[1] - 2; j++) {
            RGBQUAD color;
            FreeImage_GetPixelColor(new_bitmap, i, j, &color);
            suavizar(i, j, &color);
            FreeImage_SetPixelColor(new_bitmap, i, j, &color);
        }
    }


    FreeImage_Save(FIF_BMP, new_bitmap, "output.bmp");
    FreeImage_Unload(bitmap);
    
    run_time = omp_get_wtime() - start_time;
    printf("\n Ejecutado en %lf seconds \n", run_time);
    
    return 0;
}
