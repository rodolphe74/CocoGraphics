#define IMAGE_FILENAME_PREFIX "ARTON525CO"
#define PALETTE_FILENAME "ARTON525PAL"
#define IMAGE_FILENAME_CHUNKS 4

/**
 * display a 320*200*16 or 640x200x400 image splitted into 8k chunks
 * use readDECBFile to read data from floppy disc
 * only tested into XROAR emulator (RGB mode)
*/


#include <cmoc.h>
#include <coco.h>

// cmoc compile with --org=4000
// https://subethasoftware.com/2020/04/19/understanding-and-using-the-coco-3-mmu/
// https://www.chibiakumas.com/6809/platform.php#LessonP9

unsigned char ucbLogoPaletteValues[16] = {
    0, 4, 14, 7, 34, 20, 38, 56,
    25, 53, 28, 23, 52, 27, 48, 63
};

// unsigned char ucbLogoPaletteValues[16] = {
//     0, 11, 38, 63, 0, 0, 0, 0,
//     0, 0, 0, 0, 0, 0, 0, 0
// };

unsigned int scrBytesWidth = 160;
unsigned char modulo;
unsigned int offset;
int scrAddress;
unsigned char *pScrAddress;
unsigned char bank;


//  (We only use $FFA1 to control the $2000-$3FFF range)
void initCocoTrois()
{
asm {

;Cpu Speed - WOOOSH!
	sta $FFD9		;Select 1.78 mhz CPU
	; sta $FFD8		;Select 0.89 mhz CPU	

;Turn On CoCo3 function 
	lda #%01000000	;M=Enable MMU Mem Mapper C=0 Coco 3 mode
	sta $FF90
	lda #%00000000	;Task 0 ($FFA0-7 control bank mapping)
	sta $FF91
}
    asm { sync }  // wait for v-sync to change the palette and graphics mode
    memcpy((void *) 0xFFB0, ucbLogoPaletteValues, 16);  // assumes RGB monitor
}

void deuxCentCinquanteSixParCentQuatreVingtDouzeParSeize()
{
asm {
    ;Select 256x192 @ 16 color
	lda #%10000000	;Vmode Bitplane
	sta $FF98
	lda #%00011010  ;see coco3.h
	sta $FF99
	ldx #$C000
	stx $FF9D		;Screen Base /8 = $60000/8=$C000
}
    scrBytesWidth = 128;
}

void troisCentVingtParDeuxCentsParSeize()
{
    // 320x200, 16 colors   0 01 111 10 -> 200 160 16 (see coco3.h)
    asm {
    ;Select 320x200 @ 16 color
	lda #%10000000	;Vmode Bitplane
	sta $FF98
	lda #%00111110  ;see coco3.h
	sta $FF99
	ldx #$C000
	stx $FF9D		;Screen Base /8 = $60000/8=$C000
}
    scrBytesWidth = 160;
}

void sixCentQuaranteParDeuxCentsParQuatre()
{
    // 640x200, 4 colors   0 01 111 01 -> 200 160 4 (see coco3.h)
    asm {
    ;Select 640x200 @ 4 color
	lda #%10000000	;Vmode Bitplane
	sta $FF98
	lda #%00111101  ;see coco3.h
	sta $FF99
	ldx #$C000
	stx $FF9D		;Screen Base /8 = $60000/8=$C000
}
    scrBytesWidth = 160;
}

// Our screen uses a total of 24k - from physical address $60000-$65FFF (Pages $30-32)
// We'll page these in to the $2000-$3FFF range using $FFA1 as required to draw each line of our graphic

// That covers the 512K area. On a 128K CoCo, you can’t even use blocks 00 to 2F, so let’s look at what you can use:
// 30    60000 - 61FFF
// 31    62000 - 63FFF     The first 4 blocks (32K) is where Basic puts the
// 32    64000 - 65FFF     HSCREEN graphics.
// 33    66000 - 67FFF
// 34    68000 - 69FFF     This is where the HGET/HPUT buffer is. (8K)
// 35    6A000 - 6BFFF     This is the secondary stack area. (8K)
// 36    6C000 - 6DFFF     This is where the 40/80 column screen goes. (8K)
// 37    6E000 - 6FFFF     And this one is unused by Basic. (8K)

// 256*192*16 (128 bytes per line):
// $30 	$60000 	Lines 0-63
// $31 	$62000 	Lines 64-127
// $32 	$64000 	Lines 128-191
// 320*200*16 (160 bytes per line):
// $30 	$60000 	Lines 0-50
// $31 	$62000 	Lines 51-101
// $32 	$64000 	Lines 102-152
// $33 	$66000 	Lines 153-203
void drawTwoPixelsPerByte(int x, int y, unsigned char c)
{
    
    modulo = (unsigned char) (x % 2);
    offset = x/2 + y * scrBytesWidth;
    scrAddress = 0x2000 + offset;

    if (offset < 0x2000) {
    asm {
        ldb #$30
        stb $FFA1
    }       
    } else if (offset < 0x4000) {
        scrAddress -= 0x2000;
    asm {
        ldb #$31
        stb $FFA1
    }     
    } else if (offset < 0x6000) {
        scrAddress -= 0x4000;
    asm {
        ldb #$32
        stb $FFA1
    }     
    } else if (offset < 0x8000) {
        scrAddress -= 0x6000;
    asm {
        ldb #$33
        stb $FFA1
    }     
    } else {
        return;
    }

    pScrAddress = (unsigned char *) scrAddress;
    if (modulo == 0) {
        *(pScrAddress) = (*pScrAddress & 15) | ((c & 15) << 4);
    } else {
        *(pScrAddress) = (*pScrAddress & 240) | (c & 15);
    }
}


// 640*200*4 (160 bytes per line) :
// $30 	$60000 	Lines 0-50
// $31 	$62000 	Lines 51-101
// $32 	$64000 	Lines 102-152
// $33 	$66000 	Lines 153-203
void drawFourPixelsPerByte(int x, int y, unsigned char c)
{
    modulo = (unsigned char) (x % 4);
    offset = x/4 + y * scrBytesWidth;
    scrAddress = 0x2000 + offset;

    if (offset < 0x2000) {
    asm {
        ldb #$30
        stb $FFA1
    }       
    } else if (offset < 0x4000) {
        scrAddress -= 0x2000;
    asm {
        ldb #$31
        stb $FFA1
    }     
    } else if (offset < 0x6000) {
        scrAddress -= 0x4000;
    asm {
        ldb #$32
        stb $FFA1
    }     
    } else if (offset < 0x8000) {
        scrAddress -= 0x6000;
    asm {
        ldb #$33
        stb $FFA1
    }     
    } else {
        return;
    }

    pScrAddress = (unsigned char *) scrAddress;
    if (modulo == 0) {
       *(pScrAddress) = (*pScrAddress & 63) | ((c & 3) << 6);
    } else if (modulo == 1) {
        *(pScrAddress) = (*pScrAddress & 207) | ((c & 3) << 4);
    } else if (modulo == 2) {
        *(pScrAddress) = (*pScrAddress & 243) | ((c & 3) << 2);
    } else {
        *(pScrAddress) = (*pScrAddress & 252) | (c & 3);
    }
}



int loadImageIntoMemory(const char *filename, const unsigned char chunksCount)
{
    unsigned char startFF = 0x30;
    int error = 0;
    size_t size;
    scrAddress = 0x2000;
    pScrAddress = (unsigned char *) scrAddress;
    char fname[12];
    memset(fname, 0, 12);
    char currentChunk[2];

    for (char i = 0; i < chunksCount; i++) {
        memset(currentChunk, 0, 2);
        memset(fname, 0, 12);
        utoa10(i, currentChunk);
        strcpy(fname, filename);
        strcat(fname, currentChunk);
        printf("current chunk:%s\n", fname);

        asm {
            ldb :startFF
            stb $FFA1
        }
        if (readDECBFile(pScrAddress, 0, fname, 0x0600, &size) != 0)
            return 1;  // failed to find or load the file

        startFF++;

    }
}


int main()
{
    size_t size;
    scrAddress = 0x2000;
    pScrAddress = (unsigned char *) scrAddress;

    // read palette
    char palette[16];
    if (readDECBFile(palette, 0, PALETTE_FILENAME, 0x0600, &size) != 0)
         return 1;
    memcpy(ucbLogoPaletteValues, palette, sizeof(palette));

    printf("palette size:%d\n", size);

 
    loadImageIntoMemory(IMAGE_FILENAME_PREFIX, IMAGE_FILENAME_CHUNKS);

    initCocoTrois();
    if (size == 4)
        sixCentQuaranteParDeuxCentsParQuatre();
    else if (size == 16)
        troisCentVingtParDeuxCentsParSeize();
    


	// Draw pixels sample
    // initCocoTrois();
    // troisCentVingtParDeuxCentsParSeize();
    // for (unsigned char y = 0; y < 200; y++) {
    //     for (int x = 0; x < 320; x++) {
    //         if (x < 80)
    //             drawTwoPixelsPerByte(x, y, 1);
    //         else if (x < 160)
    //             drawTwoPixelsPerByte(x, y, 2);
    //         else if (x < 240)
    //             drawTwoPixelsPerByte(x, y, 3);
    //         else 
    //             drawTwoPixelsPerByte(x, y, 4);
    //     }
    // }

    // for (unsigned char y = 0; y < 200; y++) {
    //     for (int x = 0; x < 640; x++) {
    //         if (x < 160)
    //             drawSixCentQuaranteParDeuxCentsParQuatre(x, y, 0);
    //         else if (x < 320)
    //             drawSixCentQuaranteParDeuxCentsParQuatre(x, y, 1);
    //         else if (x < 480)
    //             drawSixCentQuaranteParDeuxCentsParQuatre(x, y, 2);
    //         else 
    //             drawSixCentQuaranteParDeuxCentsParQuatre(x, y, 3);
    //     }
    // }

    while(1);
    return 0;
}
