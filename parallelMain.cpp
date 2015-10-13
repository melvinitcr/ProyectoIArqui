/* 
 * File:   parallelMain.cpp
 * Author: manu
 *
 * Created on 21 de septiembre de 2015, 10:53 PM
 */

#include <FreeImage.h>
#include <omp.h>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <string.h>
using namespace std;

#define BPP 32 //Bits per pixel 
FIBITMAP* bitmapP;
FIBITMAP * new_bitmapP;

/**
 * Multiplica dos numeros complejos 
 * @param Z1
 * @param Z2
 * @param result
 */
void multComplexP(float *Z1, float *Z2, float *result) {
    result[0] = Z1[0] * Z2[0] - Z1[1] * Z2[1];
    result[1] = Z1[0] * Z2[1] + Z1[1] * Z2[0];
}

/**
 * Divide dos numeros complejos
 * @param Z1
 * @param Z2
 * @param result
 */
void divComplexP(float *Z1, float *Z2, float *result) {
    float div = Z2[0] * Z2[0] + Z2[1] * Z2[1];
    float resultMULT[2];
    Z2[1] = Z2[1] * -1;
    multComplexP(Z1, Z2, resultMULT);
    result[0] = resultMULT[0] / div;
    result[1] = resultMULT[1] / div;
}

/**
 * Realiza el mapeo de Z a W
 * @param ZX
 * @param result
 */
void mapperP(float *ZX, float *result) {

    float Z[2] = {ZX[0], ZX[1]};
    float C[2] = {0.003, 0};
    float resultMULT[2];
    multComplexP(C, Z, resultMULT);
    resultMULT[0] += 1;
    resultMULT[1] += 1;
    float A[2] = {2.1, 2.1};
    float resultMULT2[2];
    multComplexP(A, Z, resultMULT2);
    divComplexP(resultMULT2, resultMULT, result);

}

/**
 * Realiza el mapeo inverso de W a Z
 * @param ZX
 * @param result
 */
void inverseMapperP(float *ZX, float *result) {

    float W[2] = {ZX[0], ZX[1]};
    float C[2] = {0.003, 0};
    float resultMULT[2];
    multComplexP(C, W, resultMULT);
    resultMULT[0] += -2.1;
    resultMULT[1] += -2.1;
    float D[2] = {-1, -1};
    float resultMULT2[2];
    multComplexP(D, W, resultMULT2);
    divComplexP(resultMULT2, resultMULT, result);
    //    cout << "A: " << resultMULT[0] << " B: " << resultMULT[1] << "\n";
}

/**
 * Evalua si un color es negro en su totalidad
 * @param color0
 * @return 
 */
int isBlackP(RGBQUAD *color0) {
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
void suavizarP(int a, int b, RGBQUAD *color) {


    RGBQUAD color1;
    FreeImage_GetPixelColor(bitmapP, a + 1, b + 1, &color1);

    RGBQUAD color2;
    FreeImage_GetPixelColor(bitmapP, a - 1, b + 1, &color2);

    RGBQUAD color3;
    FreeImage_GetPixelColor(bitmapP, a + 1, b - 1, &color3);

    RGBQUAD color4;
    FreeImage_GetPixelColor(bitmapP, a - 1, b - 1, &color4);


    if (isBlackP(&color1) | isBlackP(&color2) | isBlackP(&color3) | isBlackP(&color4)) {
        FreeImage_GetPixelColor(bitmapP, a, b, color);
    } else {

        int red = color1.rgbRed + color2.rgbRed + color3.rgbRed + color4.rgbRed;
        int green = color1.rgbGreen + color2.rgbGreen + color3.rgbGreen + color4.rgbGreen;
        int blue = color1.rgbBlue + color2.rgbBlue + color3.rgbBlue + color4.rgbBlue;

        color->rgbGreen = green / 4;
        color->rgbBlue = blue / 4;
        color->rgbRed = red / 4;
    }
}

//char fotoP[99];
//
//int main(int argc, char** argv) {
//
//
//    if (argc <= 2) {
//        exit(0);
//    }
//
//    bzero(fotoP, 99);
//    // printf("%s\n", argv[2]);
//    strcpy(fotoP, argv[2]);
//
//    int count = atoi(argv[4]);
//
//    if (count == 1) {
//        printf("Ejecucion,Tiempo\n");
//    }
//
//    double start_time, run_time;
//    start_time = omp_get_wtime();
//
//    FreeImage_Initialise();
//    atexit(FreeImage_DeInitialise);
//
////    strcpy(fotoP, "sample.png");
//
//    FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(fotoP, 0);
//    bitmapP = FreeImage_Load(formato, fotoP);
//
//
//    FIBITMAP* temp = FreeImage_ConvertTo32Bits(bitmapP);
//
//
//    int width = FreeImage_GetWidth(temp);
//    int height = FreeImage_GetHeight(temp);
//
//
//    FreeImage_Unload(bitmapP);
//
//
//    bitmapP = temp;
//
//    float Zwidth[2] = {(float) width, 0};
//    float Zheight[2] = {0, (float) height};
//    float resultwidth[2];
//    float resultheight[2];
//
//
//    mapperP(Zwidth, resultwidth);
//    mapperP(Zheight, resultheight);
//
//    RGBQUAD color;
//    color.rgbGreen = 0;
//    color.rgbBlue = 0;
//    color.rgbRed = 0;
//
//    new_bitmapP = FreeImage_Allocate(width, height, BPP);
//    
//    FIBITMAP * salida;
//    
////#pragma omp simd
//    for (float i = 0; i < width; i++) {
//        for (float j = 0; j < height; j++) {
//
//            float Z[2] = {i, height - j};
//            float resultMap[2];
//            inverseMapperP(Z, resultMap);
//
//            if (resultMap[0] < 0 | resultMap[1] < 0 | resultMap[0] > width | resultMap[1] > height) {
//                FreeImage_SetPixelColor(new_bitmapP, i, j, &color);
//            } else {
//                RGBQUAD color;
//                suavizarP(resultMap[0], height - resultMap[1], &color);
//                FreeImage_SetPixelColor(new_bitmapP, i, j, &color);
//            }
//        }
//    }
//
//    salida = FreeImage_Allocate(resultwidth[0], resultheight[1], BPP);
//
//    int numprocs = omp_get_num_procs();
//    for (int i=1; i<=numprocs; i++){
//        //printf("%d\n", i);
//        omp_set_num_threads(i);
//    }
//    
//    for (int i = 0; i < resultwidth[0]; i++) {
//        RGBQUAD color;
//        int j;
//        int resu = resultheight[1];
//            #pragma omp parallel for private (color)
//            for(j = 0; j < resu; j++) {
//                //RGBQUAD color;
//                FreeImage_GetPixelColor(new_bitmapP, i, height - j, &color);
//                FreeImage_SetPixelColor(salida, i, resultheight[1] - j, &color);
//            }
//    } 
//    
//
//    FreeImage_Save(FIF_BMP, salida, "output.bmp");
//    FreeImage_Unload(bitmapP);
//
//    run_time = omp_get_wtime() - start_time;
//    printf("%d,%lf\n", count, run_time);
//
//
//    return 0;
//}
