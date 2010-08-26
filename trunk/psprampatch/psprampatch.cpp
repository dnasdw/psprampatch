#include <pspkernel.h>
#include <pspctrl.h>
#include <pspumd.h>
#include <pspsysmem_kernel.h>
#include <string.h>

#include "psprampatch.h"

#define MAJOR_VERSION 1
#define MINOR_VERSION 0

PSP_MODULE_INFO("psprampatch", PSP_MODULE_KERNEL, MAJOR_VERSION, MINOR_VERSION);
PSP_MAIN_THREAD_ATTR(0);

unsigned char gameID[12] = {0};

bool getGameID()
{
	if(sceUmdCheckMedium(0) == 0) {
		sceUmdWaitDriveStat(UMD_WAITFORDISC);
	}
	sceUmdActivate(1, "disc0:");
	sceUmdWaitDriveStat(UMD_WAITFORINIT);
	SceUID fd;
	if ((fd = sceIoOpen("disc0:/UMD_DATA.BIN", PSP_O_RDONLY, 0777)) < 0) {
		return false;
	}
	sceIoRead(fd, gameID, 10);
	gameID[10] = 0;
	gameID[11] = 0;
	sceIoClose(fd);
	return true;
}

unsigned int loadData()
{
	unsigned int flags = 0;
	SceUID fd;
	if ((fd = sceIoOpen("ms0:/psprampatch/psprampatch0.bin", PSP_O_RDONLY, 0777)) >= 0) {
		PSPRAMPATCHFILEHEADER prpfileheader;
		sceIoRead(fd, &prpfileheader, sizeof(PSPRAMPATCHFILEHEADER));
		if ((strcmp((char*)prpfileheader.ID, "PSPRAMPATCH")  == 0) && (prpfileheader.major_version * 0x100 + prpfileheader.minor_version <= MAJOR_VERSION * 0x100 + MINOR_VERSION) && (prpfileheader.type == 0)) {
			sceIoLseek(fd, prpfileheader.gamecount_ptr, PSP_SEEK_SET);
			PSPRAMPATCHGAMECOUNT prpgamecount;
			sceIoRead(fd, &prpgamecount, sizeof(PSPRAMPATCHGAMECOUNT));
			if (prpgamecount.count != 0) {
				PSPRAMPATCHGAMETABLE prpgametable;
				for (unsigned int i = 0; i < prpgamecount.count; i++) {
					sceIoLseek(fd, prpgamecount.gametable_ptr + i * sizeof(PSPRAMPATCHGAMETABLE), PSP_SEEK_SET);
					sceIoRead(fd, &prpgametable, sizeof(PSPRAMPATCHGAMETABLE));
					if ((strcmp((char*)prpgametable.ID, (char*)gameID) == 0) && (prpgametable.count != 0)) {
						flags |= 0x00000001;
						break;
					}
				}
			}
		}
		sceIoClose(fd);
	}
	if ((fd = sceIoOpen("ms0:/psprampatch/psprampatch2.bin", PSP_O_RDONLY, 0777)) >= 0) {
		PSPRAMPATCHFILEHEADER prpfileheader;
		sceIoRead(fd, &prpfileheader, sizeof(PSPRAMPATCHFILEHEADER));
		if ((strcmp((char*)prpfileheader.ID, "PSPRAMPATCH")  == 0) && (prpfileheader.major_version * 0x100 + prpfileheader.minor_version <= MAJOR_VERSION * 0x100 + MINOR_VERSION) && (prpfileheader.type == 0)) {
			sceIoLseek(fd, prpfileheader.gamecount_ptr, PSP_SEEK_SET);
			PSPRAMPATCHGAMECOUNT prpgamecount;
			sceIoRead(fd, &prpgamecount, sizeof(PSPRAMPATCHGAMECOUNT));
			if (prpgamecount.count != 0) {
				/*PSPRAMPATCHGAMETABLE prpgametable;
				for (unsigned int i = 0; i < prpgamecount.count; i++) {
					sceIoLseek(fd, prpgamecount.gametable_ptr + i * sizeof(PSPRAMPATCHGAMETABLE), PSP_SEEK_SET);
					sceIoRead(fd, &prpgametable, sizeof(PSPRAMPATCHGAMETABLE));
					if ((strcmp((char*)prpgametable.ID, (char*)gameID) == 0) && (prpgametable.count != 0)) {
						flags |= 0x00000001;
						break;
					}
				}*/
			}
		}
		sceIoClose(fd);
	}
	return flags;
}

void manualrampatch()
{
	SceUID fd;
	if ((fd = sceIoOpen("ms0:/psprampatch/psprampatch0.bin", PSP_O_RDONLY, 0777)) < 0) {
		return;
	}
	PSPRAMPATCHFILEHEADER prpfileheader;
	sceIoRead(fd, &prpfileheader, sizeof(PSPRAMPATCHFILEHEADER));
	if ((strcmp((char*)prpfileheader.ID, "PSPRAMPATCH")  == 0) && (prpfileheader.major_version * 0x100 + prpfileheader.minor_version <= MAJOR_VERSION * 0x100 + MINOR_VERSION) && (prpfileheader.type == 0)) {
		sceIoLseek(fd, prpfileheader.gamecount_ptr, PSP_SEEK_SET);
		PSPRAMPATCHGAMECOUNT prpgamecount;
		sceIoRead(fd, &prpgamecount, sizeof(PSPRAMPATCHGAMECOUNT));
		if (prpgamecount.count != 0) {
			PSPRAMPATCHGAMETABLE prpgametable;
			for (unsigned int i = 0; i < prpgamecount.count; i++) {
				sceIoLseek(fd, prpgamecount.gametable_ptr + i * sizeof(PSPRAMPATCHGAMETABLE), PSP_SEEK_SET);
				sceIoRead(fd, &prpgametable, sizeof(PSPRAMPATCHGAMETABLE));
				if ((strcmp((char*)prpgametable.ID, (char*)gameID) == 0) && (prpgametable.count != 0)) {
					PSPRAMPATCHTABLE prptable;
					unsigned int base = 0;
					for (unsigned int i = 0; i < prpgametable.count; i++) {
						sceIoLseek(fd, prpgametable.table_ptr + i * sizeof(PSPRAMPATCHTABLE), PSP_SEEK_SET);
						sceIoRead(fd, &prptable, sizeof(PSPRAMPATCHTABLE));
						if ((prptable.offset & 0x10000000) == 0) {
							base = (prptable.offset & 0x017FFFFF) | 0x48800000;
						} else {
							base = (prptable.offset & 0x007FFFFF) | 0x88000000;
						}
						if (prptable.increase._normal == 0) {
							sceIoLseek(fd, prptable.data_ptr, PSP_SEEK_SET);
							unsigned char temp;
							for (unsigned int j = 0; j < prptable.length._32; j++) {
								sceIoRead(fd, &temp, 1);
								*(unsigned char*)(base + j) = temp;
							}
						} else {
							sceIoLseek(fd, prptable.data_ptr, PSP_SEEK_SET);
							switch (prptable.length._16) {
								case 1:
									{
										sceIoLseek(fd, prptable.data_ptr, PSP_SEEK_SET);
										unsigned char temp;
										sceIoRead(fd, &temp, 1);
										for (unsigned short j = 0; j < prptable.length._count; j++, temp += prptable.increase._value) {
											*(unsigned char*)(base + j * prptable.increase._offset) = temp;
										}
									}
									break;
								case 2:
									{
										sceIoLseek(fd, prptable.data_ptr, PSP_SEEK_SET);
										unsigned short temp;
										sceIoRead(fd, &temp, 2);
										for (unsigned short j = 0; j < prptable.length._count; j++, temp += prptable.increase._value) {
											*(unsigned short*)(base + j * prptable.increase._offset) = temp;
										}
									}
									break;
								case 3:
									{
										sceIoLseek(fd, prptable.data_ptr, PSP_SEEK_SET);
										unsigned short temp0;
										unsigned char temp1;
										sceIoRead(fd, &temp0, 2);
										sceIoRead(fd, &temp1, 1);
										for (unsigned short j = 0; j < prptable.length._count; j++) {
											*(unsigned short*)(base + j * prptable.increase._offset) = temp0;
											*(unsigned short*)(base + j * prptable.increase._offset + 2) = temp1;
											if (temp0 > 0xFFFF - prptable.increase._value) {
												temp1++;
												temp0 -= 0x10000 - prptable.increase._value;
											} else {
												temp0 += prptable.increase._value;
											}
										}
									}
									break;
								case 4:
									{
										sceIoLseek(fd, prptable.data_ptr, PSP_SEEK_SET);
										unsigned int temp;
										sceIoRead(fd, &temp, 4);
										for (unsigned short j = 0; j < prptable.length._count; j++, temp += prptable.increase._value) {
											*(unsigned int*)(base + j * prptable.increase._offset) = temp;
										}
									}
									break;
								default:
									break;
							}
						}
					}
				}
			}
		}
	}
	sceIoClose(fd);
	return;
}

void manualconditionrampatch()
{
	return;
}

void autorampatch(bool first = false)
{
	return;
}

void autoconditionrampatch(bool first = false)
{
	return;
}

int main_thread(SceSize args, void *argp)
{
	if (!getGameID()) {
		return 1;
	}
	unsigned int flags = loadData();
	bool started = false;
	unsigned int oldButtons = 0;
	SceCtrlData pad;
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);
	while (true) {
		sceCtrlPeekBufferPositive(&pad, 1);
		do {
			if (((oldButtons & (PSP_CTRL_START | PSP_CTRL_VOLDOWN)) == (PSP_CTRL_START | PSP_CTRL_VOLDOWN)) && ((pad.Buttons & (PSP_CTRL_START | PSP_CTRL_VOLDOWN)) != (PSP_CTRL_START | PSP_CTRL_VOLDOWN))) {
				if (started) {
					started = false;
					break;
				} else {
					started = true;
				}
			} else if (((oldButtons & (PSP_CTRL_SELECT | PSP_CTRL_VOLDOWN)) == (PSP_CTRL_SELECT | PSP_CTRL_VOLDOWN)) && ((pad.Buttons & (PSP_CTRL_SELECT | PSP_CTRL_VOLDOWN)) != (PSP_CTRL_SELECT | PSP_CTRL_VOLDOWN))) {
				flags = loadData();
			} else if (((oldButtons & (PSP_CTRL_SELECT | PSP_CTRL_NOTE)) == (PSP_CTRL_SELECT | PSP_CTRL_NOTE)) && ((pad.Buttons & (PSP_CTRL_SELECT | PSP_CTRL_NOTE)) != (PSP_CTRL_SELECT | PSP_CTRL_NOTE))) {
				if (!started) {
					break;
				}
				if (flags & 0x00000001) {
					manualrampatch();
				}
			} else if (((oldButtons & (PSP_CTRL_START | PSP_CTRL_NOTE)) == (PSP_CTRL_START | PSP_CTRL_NOTE)) && ((pad.Buttons & (PSP_CTRL_START | PSP_CTRL_NOTE)) != (PSP_CTRL_START | PSP_CTRL_NOTE))) {
				if (!started) {
					break;
				}
				if (flags & 0x00000002) {
					manualconditionrampatch();
				}
			}
			if (started) {
				if (flags & 0x00000004) {
					autorampatch();
				}
				if (flags & 0x00000008) {
					autoconditionrampatch();
				}
			}
		} while (false);
		oldButtons = pad.Buttons;
		sceKernelDelayThread(50000);
	}
	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

int module_start(SceSize args, void *argp)
{
	SceUID thid = sceKernelCreateThread("psprampatch", main_thread, 7, 0x800, 0, NULL);
	if (thid >= 0) {
		sceKernelStartThread(thid, args, argp);
	}
	return 0;
}

int module_stop(SceSize args, void *argp)
{
	return 0;
}

#ifdef __cplusplus
}
#endif
