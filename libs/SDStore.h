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
	void generateFileName(char *);
	size_t writeToSD(const char*, size_t);
	size_t writeToSD(const uint8_t*, size_t);
	void createFile();
	bool initSdCard();
	void end();
	void closeFile();
	bool isInitialize();
	bool isOpenFile();

  private:
	File sdFile;
	bool sdCard = false;
	char filename[MAX_FNAME_LEN];
};

#endif // SDStore_h