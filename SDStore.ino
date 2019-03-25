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
	logger.trace("Start init SD card...\n");
	assert(CS_PIN);
	logger.trace("CS pin : %i\n", CS_PIN);

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
	assert(sizeof(_rootPath) + sizeof(filename) < MAX_PATH_LEN);
	char path[MAX_PATH_LEN] {0};
	strcat(path, _rootPath);
	strcat(path, filename);
	if (SD.exists(path)) {
		SD.remove(path);
		logger.debug("File: %s removed\n", filename);
	}

	sdFile = SD.open(path, FILE_WRITE);
	if (sdFile) {
		logger.debug("File: %s created\n", path);
	} else {
		logger.error("Failed to open file for writing, file name : [%s]\n", path);
	}
}

void SDStore::end() {
	closeFile();
	if (sdCard) {
		SD.end();
		logger.trace("SD card closed, is init = [%s]\n", isInitialize() ? "true" : "false");
	}
}

void SDStore::closeFile() {
	if (sdFile) {
		sdFile.flush();
		sdFile.close();
		logger.trace("File closed, isOpenFile = [%s]\n", isOpenFile() ? "true" : "false");
	}
}

size_t SDStore::writeToSD(const char *buffer, size_t bytesCount) { return writeToSD(reinterpret_cast<const uint8_t *>(buffer), bytesCount); }

size_t SDStore::writeToSD(const uint8_t *buffer, size_t bytesCount) {
	logger.trace("Start write to SD card...\n");
	size_t count = sdFile.write(buffer, bytesCount);
	if (count) {
		sdFile.flush();
	}

	logger.trace("Bytes count : [%i]\n", count);
	if (count < bytesCount) {
		if (!count) {
			logger.error("Write failed\n");
		}
		logger.debug("WARN! Not all bytes write to SD, input count = [%i], writed count = [%i]\n", bytesCount, count);
	}
	return count;
}

bool SDStore::isInitialize() const { return sdCard ? true : false; }

bool SDStore::isOpenFile() { return sdFile ? true : false; }