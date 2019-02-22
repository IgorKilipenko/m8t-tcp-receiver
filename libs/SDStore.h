/**
 * SDStore.h
 */

#ifndef SDStore_h
#define SDStore_h

#include "Arduino.h"
#include <SPI.h>
#include <SD.h>

#ifndef CS_PIN
#define CS_PIN D8
#endif // CS_PIN

class SDStore {
  public:
	SDStore();
	~SDStore();
	static const size_t MAX_FNAME_LEN = 13;
	void generateFileName(char *);
	void writeToSD(char[], size_t);
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