/**
 * SDStore.h
 */

#ifndef SDStore_h
#define SDStore_h

#include "Arduino.h"
#include <SPI.h>
#include <SD.h>

class SDStore {
  public:
	SDStore();
	~SDStore();
	static const size_t MAX_FNAME_LEN = 13;
	static const size_t MAX_PATH_LEN = 128;
#ifdef TTGO_BOARD
	static const uint8_t SD_SS = 13;
	static const uint8_t SD_MOSI = 15;
	static const uint8_t SD_MISO = 2;
	static const uint8_t SD_SCK = 14;
#endif
	void generateFileName(char *);
	size_t getNextFileNumber(const char *dirname) const;
	size_t writeToSD(const char *, size_t);
	size_t writeToSD(const uint8_t *, size_t);
	void createFile();
	bool initSdCard();
	void end();
	void closeFile();
	bool isInitialize() const;
	bool isOpenFile();

  private:
	File sdFile;
	bool _isInitSdCard = false;
	char filename[MAX_FNAME_LEN];
	const char *_rootPath{"/"};
	const char *_prefix{"raw"};
	const char *_ext{"ubx"};
	const size_t _max_nums = 999;
};

#endif // SDStore_h