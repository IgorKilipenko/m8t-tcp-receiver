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
	void generateFileName(char *);
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
	bool sdCard = false;
	char filename[MAX_FNAME_LEN];
	const char * _rootPath = "/";
};

#endif // SDStore_h