#ifndef __PSP_RAM_PATCH__
#define __PSP_RAM_PATCH__

struct PSPRAMPATCHFILEHEADER {
	unsigned char ID[12];
	unsigned short major_version;
	unsigned short minor_version;
	unsigned int type;
	unsigned int gamecount_ptr;
};

struct PSPRAMPATCHGAMECOUNT {
	unsigned int count;
	unsigned int gametable_ptr;
};

struct PSPRAMPATCHGAMETABLE {
	unsigned char ID[12];
	unsigned int count;
	unsigned int table_ptr;
};

struct PSPRAMPATCHTABLE {
	unsigned int offset;
	union {
		unsigned int _32;
		struct {
			unsigned short _16;
			unsigned short _count;
		};
	} length;
	unsigned int data_ptr;
	union {
		unsigned int _normal;
		struct {
			short _offset;
			short _value;
		};
	} increase;
};

#endif
