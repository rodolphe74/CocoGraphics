/**
 * Convert an image to coco3 format
 *
 * argument are:
 * - an image filename
 * - a graphic mode (0=320x200x16) (1=640x200x4)
 * ex: ./convert test.jpg 0
 *
 * resulting files are:
 * - a palette (.pal)
 * - a .coc extension file (contains all coco3 image data)
 * - several .co* extension file (previous file splitted into 8k chunks)
 * ex: test.pal test.coc test.co0 test.co1 test.co2 test.co3
 *
 * pixel.bin (pixel.c cmoc compiled coco3 binary) load pal and 8k files
 * to display corresponding image
*/

#include "exoquant.h"
#include <stdio.h>
#include <float.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_STATIC
#include "stb/stb_image_resize.h"

// #define NUM_COLORS 4        // 256 max
#define HIGH_QUALITY 1

unsigned char cocoPalette[64 * 4];
unsigned char *pPalette;
unsigned char *nPalette;
char resultingFilename[128] = {0};
char tempStr[128] = {0};
int numColors = 4;

void setUserDefinedPalette(unsigned char *p)
{
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
    p[3] = 255;

    p[4] = 2;
    p[5] = 74;
    p[6] = 253;
    p[7] = 255;

    p[8] = 253;
    p[9] = 74;
    p[10] = 2;
    p[11] = 255;

    p[12] = 255;
    p[13] = 255;
    p[14] = 255;
    p[15] = 255;
}

void setCocoPalette(unsigned char *optimalPalette, unsigned char *newPalette)
{
    int minIndex = 0;
    float currentDistance = 0;
    for (int i = 0; i < numColors; i++) {
        float minDistance = FLT_MAX;
        int indexColor = i * 4;
        for (int j = 0; j < 64; j++) {
            int indexCocoColor = j * 4;
            currentDistance = sqrtf(
                                  powf(cocoPalette[indexCocoColor] - optimalPalette[indexColor], 2)
                                  + powf(cocoPalette[indexCocoColor + 1] - optimalPalette[indexColor + 1], 2)
                                  + powf(cocoPalette[indexCocoColor + 2] - optimalPalette[indexColor + 2], 2));
            if (currentDistance < minDistance) {
                minDistance = currentDistance;
                minIndex = j;
            }

        }
        printf("%f -> %d (%d,%d,%d)\n", minDistance, minIndex, cocoPalette[minIndex * 4], cocoPalette[minIndex * 4 + 1], cocoPalette[minIndex * 4 + 2]);
        newPalette[indexColor] = cocoPalette[minIndex * 4];
        newPalette[indexColor + 1] = cocoPalette[minIndex * 4 + 1];
        newPalette[indexColor + 2] = cocoPalette[minIndex * 4 + 2];
        newPalette[indexColor + 3] = cocoPalette[minIndex * 4 + 3];
    }
}

int getIndexColor(int r, int g, int b)
{
    for (int i = 0; i < numColors; i++)
        if (r == nPalette[i * 4] && g == nPalette[i * 4 + 1] && b == nPalette[i * 4 + 2])
            return i;

    return 0;   // ?
}

int getCocoColorIndex(int r, int g, int b)
{
    for (int i = 0; i < 64; i++)
        if (r == cocoPalette[i * 4] && g == cocoPalette[i * 4 + 1] && b == cocoPalette[i * 4 + 2])
            return i;

    return 0;   // ?
}

void readCocoPalette()
{
    int xx, yy, c;
    unsigned char *data = stbi_load("cocopal.png", &xx, &yy, &c, 4);
    int colorIndex = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int offset = i * 8 * 4 + j * 4;
            int r = data[offset];
            int g = data[offset + 1];
            int b = data[offset + 2];
            // printf("Color %d = (%d,%d,%d)\n", colorIndex, r, g, b);
            cocoPalette[colorIndex * 4] = r;
            cocoPalette[colorIndex * 4 + 1] = g;
            cocoPalette[colorIndex * 4 + 2] = b;
            cocoPalette[colorIndex * 4 + 3] = 255;
            colorIndex++;
        }
    }
}

void findResultingFilename(char *fname)
{
    char bufStr[128] = {0};

    int separatorPos = -1;
    for (int i = 0; i < strlen(fname); i++)
        if (fname[i] == '/')
            separatorPos = i;

    if (separatorPos > -1) {
        strncpy(resultingFilename, fname + separatorPos + 1, strlen(fname) - separatorPos - 1);
        strcpy(bufStr, resultingFilename);
    } else {
        strcpy(bufStr, fname);
    }

    int extensionPos = -1;
    for (int i = 0; i < strlen(bufStr); i++)
        if (bufStr[i] == '.')
            extensionPos = i;

    if (extensionPos > -1) {
        int sz = (strlen(bufStr)) - (strlen(bufStr) - extensionPos);
        memset(resultingFilename, 0, sizeof(resultingFilename));
        strncpy(resultingFilename, bufStr, sz);
        strcpy(bufStr, resultingFilename);
    }

    memset(resultingFilename, 0, sizeof(resultingFilename));
    strncpy(resultingFilename, bufStr, strlen(bufStr) < 8 ? strlen(bufStr) : 8);

    printf("extension at -> %d\n", extensionPos);
    printf("separator at -> %d\n", separatorPos);
    printf("resulting filename -> %s\n", resultingFilename);
}


char *shrinkIfNecessary(int graphicMode, const char *inputImage, const int ix, const int iy, char *resizedImage, int *ox, int *oy)
{
    float ratioX = 0, ratioY = 0, ratio;
    int doResize = 0;

    if (graphicMode == 0) {
        ratioX = ix / 320.0;
        printf("ratio x -> %f\n", ratioX);
        doResize = 1;
    } else {
        ratioX = ix / 640.0;
        printf("ratio x -> %f\n", ratioX);
        doResize = 1;
    }

    if (iy > 200) {
        ratioY = iy / 200.0;
        printf("ratio y -> %f\n", ratioY);
        doResize = 1;
    }

    if (doResize) {
        ratio = fmax(ratioX, ratioY);
        printf("ratio -> %f\n", ratio);

        int xx, yy;
        if (graphicMode == 0) {
            xx = ix / ratio;
            yy = iy / ratio;
        } else {
            xx = ix / ratio * 2;
            yy = iy / ratio;
            if (xx > 640) {
                ratio = xx / 640.0;
                xx /= ratio;
                yy /= ratio;
            }
        }

        printf("new dimensions -> %d*%d\n", xx, yy);

        resizedImage = malloc(xx * yy * 4);
        stbir_resize_uint8(inputImage, ix, iy, 4 * ix, resizedImage, xx, yy, xx * 4, 4);
        stbi_write_png("rsz.png", xx, yy, 4, resizedImage, 4 * xx);
        *ox = xx;
        *oy = yy;
        return resizedImage;
    }
    return NULL;
}

char *putIntoCanvas(const char *inputData, int ix, int iy, int grMode, char *outputData, int *ox, int *oy)
{
    int targetw = 320;
    int targeth = 200;
    if (grMode == 0) {
        targetw = 320;
    } else {
        targetw = 640;
    }
    outputData = malloc(targetw * targeth * 4);
    memset(outputData, 0, targetw * targeth * 4);
    int k = 0, l = 0;
    for (int j = 0; j < iy; j++) {
        for (int i = 0; i < ix; i++) {
            if (j < targeth && i < targetw) {
                outputData[(k * targetw + l) * 4] = inputData[(j * ix + i) * 4];
                outputData[(k * targetw + l) * 4 + 1] = inputData[(j * ix + i) * 4 + 1];
                outputData[(k * targetw + l) * 4 + 2] = inputData[(j * ix + i) * 4 + 2];
                outputData[(k * targetw + l) * 4 + 3] = inputData[(j * ix + i) * 4 + 3];
            }
            l++;
        }
        l = 0;
        k++;
    }
    *ox = targetw;
    *oy = targeth;
    return outputData;
}


int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("argc:%d\n", argc);
        printf("arguments must be exactly [filename] [graphics mode]\n");
        return 0;
    }

    char *fname = argv[1];      // filename
    char *grMode = argv[2];     // graphic mode (0=640*200 1=320*200)

    if (*grMode == '0') {
        numColors = 16;
    } else {
        numColors = 4;
    }
    pPalette = malloc(4 * numColors);
    nPalette = malloc(4 * numColors);

    findResultingFilename(fname);
    printf("resulting filename -> %s\n", resultingFilename);

    readCocoPalette();

    int x, y, c;

    unsigned char *data = stbi_load(fname, &x, &y, &c, 4);

    printf("Image size:%d*%d*%d\n", x, y, c);
    unsigned char *indexedPaletteData = malloc(x * y);


    // resize ?
    int xx, yy;
    unsigned char *dataResized = NULL;
    dataResized = shrinkIfNecessary(*grMode == '0' ? 0 : 1, data, x, y, dataResized, &xx, &yy);
    if (!dataResized) {
        xx = x;
        yy = y;
        dataResized = malloc(x * y * 4);
    }

    // find optimal palette
    exq_data *pExq;
    pExq = exq_init();
    exq_no_transparency(pExq);
    exq_feed(pExq, dataResized, xx * yy);
    exq_quantize(pExq, numColors);
    exq_quantize_hq(pExq, numColors);
    exq_quantize_ex(pExq, numColors, HIGH_QUALITY);
    exq_get_palette(pExq, pPalette, numColors);

    // and convert it in COCO color space
    setCocoPalette(pPalette, nPalette);

    for (int i = 0; i < numColors * 4; i += 4)
        printf("Optimal palette %d -> (%d,%d,%d)\n", i / 4, pPalette[i], pPalette[i + 1], pPalette[i + 2]);

    for (int i = 0; i < numColors * 4; i += 4)
        printf("Corrected palette %d -> (%d,%d,%d)\n", i / 4, nPalette[i], nPalette[i + 1], nPalette[i + 2]);

    // dithering
    exq_map_image(pExq, xx * yy, dataResized, indexedPaletteData);
    exq_map_image_ordered(pExq, xx, yy, dataResized, indexedPaletteData);
    // exq_map_image_random(pExq, xx * yy, dataResized, indexedPaletteData);

    for (int i = 0, j = 0; i < xx * yy * 4; i += 4, j++) {
        dataResized[i] =  *(nPalette + indexedPaletteData[j] * 4);
        dataResized[i + 1] =  *(nPalette + indexedPaletteData[j] * 4 + 1);
        dataResized[i + 2] =  *(nPalette + indexedPaletteData[j] * 4 + 2);
        dataResized[i + 3] =  *(nPalette + indexedPaletteData[j] * 4 + 3);
    }

    // sometimes resized image doesn't fit on coco screen exactly
    char *canvasData = NULL;
    int ox, oy;
    canvasData = putIntoCanvas(dataResized, xx, yy, *grMode == '0' ? 0 : 1, canvasData, &ox, &oy);
    xx = ox;
    yy = oy;
    free(dataResized);
    dataResized = canvasData;
    printf("canvas size -> %d*%d\n", xx, yy);

    exq_free(pExq);
    stbi_write_png("resultdt.png", xx, yy, 4, dataResized, 4 * xx);

    // write coco palette
    memset(tempStr, 0, sizeof(tempStr));
    strcpy(tempStr, resultingFilename);
    strcat(tempStr, ".pal");
    FILE *fPal = fopen(tempStr, "wb");
    for (int i = 0; i < numColors; i++) {
        unsigned char index = (unsigned char) getCocoColorIndex(nPalette[i * 4], nPalette[i * 4 + 1], nPalette[i * 4 + 2]);
        printf("palette index %d -> (%d,%d,%d)=%d\n", i,  nPalette[i * 4], nPalette[i * 4 + 1], nPalette[i * 4 + 2],  index);
        fwrite(&index, 1, 1, fPal);
    }
    fclose(fPal);

    // write in coco memory format. (must be splitted in 8k chunks after)
    char cocFilename[128] = {0};
    memset(tempStr, 0, sizeof(tempStr));
    strcpy(tempStr, resultingFilename);
    strcat(tempStr, ".coc");
    strcpy(cocFilename, tempStr);
    FILE *fGra = fopen(cocFilename, "wb");
    int byteCount = 0;
    unsigned char byteComplete[4];
    for (int i = 0; i < xx * yy * 4; i += 4) {
        int index = getIndexColor(dataResized[i], dataResized[i + 1], dataResized[i + 2]);
        // printf("%d ", index);
        byteComplete[byteCount] = index;
        byteCount++;
        if (*grMode == '1' && byteCount == 4) {
            // 640*200*4 (4 pixels per byte)
            unsigned char b = ((byteComplete[0] & 15) << 6) | ((byteComplete[1] & 15) << 4) | ((byteComplete[2] & 15) << 2) | (byteComplete[3] & 15);
            fwrite(&b, sizeof(char), 1, fGra);
            byteCount = 0;
        } else if (*grMode == '0' && byteCount == 2) {
            // 320*200*16 (2 pixels per byte)
            unsigned char b = ((byteComplete[0] & 15) << 4) | (byteComplete[1] & 15);
            fwrite(&b, sizeof(char), 1, fGra);
            byteCount = 0;
        }
    }
    fclose(fGra);

    // split
    int bytesWritten = 0;
    int chunk = -1;
    char sChunk[] = "0";
    fGra = fopen(cocFilename, "rb");
    fseek(fGra, 0, SEEK_END);
    size_t fGraSz = ftell(fGra);
    fseek(fGra, 0, SEEK_SET);
    FILE *fChunk = NULL;
    char b;
    for (int i = 0; i < fGraSz; i++) {
        fread(&b, 1, 1, fGra);
        if (i % 8192 == 0) {
            if (i) fclose(fChunk);
            printf("i->%d\n", i);
            chunk++;
            sChunk[0] = 48 + chunk;
            memset(tempStr, 0, sizeof(tempStr));
            strcpy(tempStr, resultingFilename);
            strcat(tempStr, ".co");
            strcat(tempStr, sChunk);
            fChunk = fopen(tempStr, "wb");
        }
        fwrite(&b, 1, 1, fChunk);
    }
    fclose(fGra);
    fclose(fChunk);

    // cleanings
    stbi_image_free(data);
    free(indexedPaletteData);
    free(dataResized);
    free(nPalette);
    free(pPalette);
    return 0;
}