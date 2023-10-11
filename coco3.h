/* registers */
#define INIT_REGISTER		((unsigned char *)0xFF90)

enum InitBit
{
	Mc0InitBit		= 1 << 0,	/* These two bits together operate to set the */
	Mc1InitBit		= 1 << 1,	/* ROM mapping: */
								/* 0? for 16K int, 16K ext */
								/* 10 for 32K int */
								/* 11 for 32K ext */
	Mc2InitBit		= 1 << 2,	/* standard SCS */
	Mc3InitBit		= 1 << 3,	/* Vector page at FEXX enabled */
	
	GimeFirqInitBit	= 1 << 4,	/* Enable interrupts from GIME to processor */
	GimeIrqInitBit	= 1 << 5,
	
	MmuEnabledInitBit	= 1 << 6,	/* MMU page registers enabled */
	
	CoCoInitBit			= 1 << 7	/* Enables CoCo 2/3 compatibility */
};

#define INIT1_REGISTER		((unsigned char *)0xFF91)

enum Init1Bit
{
	TaskRegisterInit1Bit	= 1 << 0,	/* Selects task 0 or 1 */
	TimerInputClockInit1Bit	= 1 << 5,	/* Selects clock for timer input: */
										/* 0 for 279.365 nsec */
										/* 1 for 63.695 usec */
	RamChipSelectInitBit	= 1 << 6	/* 0 for 64K, 1 for 256K */
};

#define IRQ_REGISTER		((unsigned char *)0xFF92)
#define FIRQ_REGISTER		((unsigned char *)0xFF93)

enum InterruptBit
{
	CartridgeInterruptBit	= 1 << 0,
	KeyboardInterruptBit	= 1 << 1,
	SerialDataInterruptBit	= 1 << 2,
	VertBorderInterruptBit	= 1 << 3,
	HorizBorderInterruptBit	= 1 << 4,
	TimerInterruptBit		= 1 << 5
};

/* The timer is 12-bit, and starts once the MSB is written to */
#define TIMER_MSB			((unsigned char *)0xFF94) /* Timer starts on write here */
#define TIMER_LSB			((unsigned char *)0xFF95)

#define VID_MODE_REGISTER	((unsigned char *)0xFF98)

enum VidModeBit
{
	LinesPerRow0VidModeBit	= 1 << 0,
	LinesPerRow1VidModeBit	= 1 << 1,
	LinesPerRow2VidModeBit	= 1 << 2,
	
	LinesPerRowVidModeBits	= LinesPerRow0VideoModeBit & LinesPerRow1VideoModeBit &
							  LinesPerRow2VideoModeBit,
	
	DisplayHertzVidModeBit	= 1 << 3,	/* 0=60hz, 1=50hz */
	
	MonochromeVidModeBit	= 1 << 4,	/* Whether or not composite out is monochrome */
	
	CmpClrPhsInvVidModeBit	= 1 << 5, 
	
	ModeSelectVidModeBit	= 1 << 7	/* 0=text, 1=gfx */
};

#define VID_RES_REGISTER	((unsigned char *)0xFF99)

enum VidResBit
{
	Colour0VidResBit		= 1 << 0, /* For hi-res text screens, controls whether colour attributes are enabled */
	Colour1VidResBit		= 1 << 1,
	
	ColourVidResBits		= Colour0VidResBit & Colour1VidResBit,
		/* Together, the colour resolution bits are used as follows: */
		/* 00 for 2 colours (8 pixels per byte) */
		/* 01 for 4 colours (4 pixels per byte) */
		/* 10 for 16 colours (2 pixels per byte) */
		/* 11 is unused, but would have been 256 colours */
	
	Horizontal0VidResBit	= 1 << 2,
	Horizontal1VidResBit	= 1 << 3,
	Horizontal2VidResBit	= 1 << 4,
	
	HorizontalVidResBits	= Horizontal0VidResBit & Horizontal1VidResBit &
							  Horizontal2VidResBit,
		/* Text: */
		/* 0?0 for 32 chars/row */
		/* 0?1 for 40 chars/row */
		/* 1?0 for 64 chars/row */
		/* 1?1 for 80 chars/row */
		/* Graphics: */
		/* 000 for 16 bytes per row */
		/* 001 for 20 bytes per row */
		/* 010 for 32 bytes per row */
		/* 011 for 40 bytes per row */
		/* 100 for 64 bytes per row */
		/* 101 for 80 bytes per row */
		/* 110 for 128 bytes per row */
		/* 111 for 160 bytes per row */
	
	ScanLines0VidResBit		= 1 << 5,
	ScanLines1VidResBit		= 1 << 6,
	
	ScanLinesVidResBits		= ScanLines0VidResBit & ScanLines1VidResBit
		/* 00 for 192 scan lines on screen */
		/* 01 for 200 scan lines on screen */
		/* 10 produces an infinite scan line screen */
		/* 11 for 225 scan lines on screen */
};

/* 6-bit colour, similar scheme to palette registers */
#define BORDER_COLOUR_REGISTER	((unsigned char *)0xFF9A)

/* 4-bit to specify how many scanlines to smoothly scroll up */
#define VERTICAL_SCROLL_REGISTER	((unsigned char *)0xFF9C)

/* These define 16-bits of the memory location of */
/* the graphics screen in the 21-bit memory space of the CoCo 3. */
/* This jumps by 8 bytes, leaving the bottom 3 bits undefined here, */
/* and the top two also undefined. Thus the address can only be */
/* located in the space of a 512K CoCo 3. */
/* A 128K CoCo ranges from 0x60000 to 0x7FFFF. */
#define VERTICAL_OFFSET_REGISTER_MSB	((unsigned char *)0xFF9D)
#define VERTICAL_OFFSET_REGISTER_LSB	((unsigned char *)0xFF9E)

/* Bit 7 turns on the horizontal virtual screen */
/* Bits 0-6 define the horizontal offset address */
#define HORIZONTAL_OFFSET_REGISTER		((unsigned char *)0xFF9F)

/* Valid values depend on the memory configuration of the CoCo 3: */
/* 128K: 56-63 */
/* 512K: 0-63 */
/* 1M: 0-127 */
/* 2M: 0-255 */
/* Note: When reading these registers, the top two bits must be masked out. */
/* This is due to a bug in certain machines. */
#define TASK0_PAGE_REGISTERS	((unsigned char[8])0xFFA0)
#define TASK1_PAGE_REGISTERS	((unsigned char[8])0xFFA8)

/* The palette registers define the 16 colours of 16 colour graphics modes. */
/* Lower colour resolutions use the lower numbered palette slots. */
/* For RGB monitors, these are 6-bit RGB as 00RGBRGB */
/* For composite monitors, the pattern is 00YYCCCC (Y=luminosity, C=chroma) */
#define PALETTE_REGISTERS		((unsigned char [16])0xFFB0)

/* A write to SLOW sets CPU to 0.89 MHz */
#define SLOW_POKE_REGISTER		((unsigned char *)0xFFD8)
/* A write to FAST sets CPU to 1.79 MHz */
#define FAST_POKE_REGISTER		((unsigned char *)0xFFD9)
