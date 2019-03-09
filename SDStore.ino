SDStore::SDStore() {}
SDStore::~SDStore() { end(); }

void SDStore::generateFileName(char *name) {
	const size_t max_nums = 999;
	bool reset = false;
	const char prefix[] = "raw_";
	const char ext[] = ".ubx";
	size_t number = 1;
	while (number != 0) {
		if (number > max_nums) {
			number = 1;
			reset = true;
		}
		sprintf(name, "%s%i%s", prefix, number, ext);
		if (SD.exists(name) && !reset) {
			// file exist
			number++;
		} else {
			// next number
			return;
		}
	}
}

bool SDStore::initSdCard() {
	if (!SD.begin(CS_PIN /*, SPI_QUARTER_SPEED*/)) {
		// initialization failed!
		logger.debug("initialization failed!.\n");
		sdCard = false;
	} else {
		logger.debug("initialization done.\n");
		sdCard = true;
	}

	return sdCard;
}

void SDStore::createFile() {
	generateFileName(filename);
	if (SD.exists(filename)) {
		SD.remove(filename);
		logger.debug("File: %s removed\n", filename);
	}
	sdFile = SD.open(filename, FILE_WRITE);
	logger.debug("File: %s created\n", filename);
}

void SDStore::end() {
	if (sdFile) {
		sdFile.close();
	}
	if (sdCard) {
		SD.end();
	}
}

void SDStore::closeFile() {
	if (sdFile) {
		sdFile.close();
	}
}

size_t SDStore::writeToSD(const char *buffer, size_t bytesCount) { return writeToSD(reinterpret_cast<const uint8_t *>(buffer), bytesCount); }

size_t SDStore::writeToSD(const uint8_t *buffer, size_t bytesCount) {
	size_t res = sdFile.write(buffer, bytesCount);
	sdFile.flush();

	delay(1);
	return res;
}

bool SDStore::isInitialize() { return sdCard ? true : false; }

bool SDStore::isOpenFile() { return sdFile ? true : false; }