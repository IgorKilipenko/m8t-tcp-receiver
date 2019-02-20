#ifndef FStore_h
#define Store_h

#include "Arduino.h"
#include <SPI.h>
#include "SdFat.h"
//#include "sdios.h"

#define SPI_SPEED SD_SCK_MHZ(50)

SdFat sd;

SdFile sdfile;

void sd_setup() {
	if (!sd.begin(chipSelect, SPI_SPEED)) {
		// sd.initErrorHalt();
		logger << "SdFat init filed\n";
	}

	sd.remove("RawWrite.txt");

	if (!file.createContiguous("RawWrite.txt", 512UL * BLOCK_COUNT)) {
		logger << "createContiguous failed\n";
	}

    
}

#endif