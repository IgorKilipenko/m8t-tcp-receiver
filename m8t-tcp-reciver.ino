
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

//#define DEBUG
#define CS_PIN D8

class Logger
{
  public:
    template <typename T>
    void debug(T, ...);
    template <typename T>
    void print(T);
    template <typename T>
    void printf(T, ...);
    template <typename T>
    void println(T);

  private:
    bool test();
} logger;

void getDataFromClients();

void generateFileName(char *);

//Logger logger;

char ssid[] = "Keenetic-9267";
char pass[] = "1234567890";

char alt_ssid[] = "Redmi5Igor123";
char alt_pass[] = "1234567890";



WiFiServer server(7042);
bool wifiConnected = false;
const int MAX_CLIENTS = 5;
WiFiClient clients[MAX_CLIENTS];

unsigned long start;

const size_t MAX_FNAME_LEN = 13;
char filename[MAX_FNAME_LEN];
//ublox56_uart1_rawtrk.cmd
const char cmdsStart[] = "!UBX CFG-MSG 3 10 0 1 0 0 0 0";
const char cmdsStop[] = "!UBX CFG-MSG 3 10 0 0 0 0 0 0\r\n!UBX CFG-MSG 3  2 0 0 0 0 0 0\r\n!UBX CFG-MSG 1 32 0 0 0 0 0 0\r\n!UBX CFG-MSG 1 34 0 0 0 0 0 0";

static const int RXPin = D2, TXPin = D1;
static const uint32_t GPSBaud = 115200;

//SoftwareSerial ss(RXPin, TXPin);

File myFile;
bool cdCard = false;

void setup()
{
    //pinMode(RXPin, INPUT);
    //pinMode(TXPin, OUTPUT);

    Serial.begin(GPSBaud);
    WiFi.begin(ssid, pass);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 30000)
    {
        delay(500);
        logger.print(".");
    }
#ifdef DEBUG
    if (WiFi.status() == WL_CONNECTED)
    {
        logger.debug("WiFi connected on ");
        logger.debug("IP -> %s\n", WiFi.localIP().toString().c_str());
    }
    else
    {
        logger.print("WiFi not connected");
    }
#endif

    if (!SD.begin(CS_PIN))
    {
        //initialization failed!
        cdCard = false;
        //return;
    }
    else
    {
        cdCard = true;
        logger.debug("initialization done.\n");
        generateFileName(filename);
        if (SD.exists(filename))
        {
            SD.remove(filename);
            logger.debug("File: %s removed\n", filename);
        }
        myFile = SD.open(filename, FILE_WRITE);
        logger.debug("File: %s created\n", filename);
    }

    for (unsigned long start = millis(); millis() - start < 1000;)
    {
        //Serial1.print("#");
        delay(500);
    }

    server.begin();
    server.setNoDelay(true);

    start = millis();
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        bool clientConnected = false;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            WiFiClient &client = clients[i];
            if (client && client.connected())
            {
                while (client.available())
                {
                    Serial.write(client.read());
                    logger.debug("Data has been sent to Serial\n");
                }
            }
            else if (!client || !client.connected())
            {
                if (client)
                { // Нет такого события
                    client.stop();
                    logger.debug("Close connection to client\n");
                }
                if (server.hasClient())
                {
                    client = server.available();
                    clientConnected = true;
                    logger.debug("Client connected, ip address: %s\n\t-> Connected clients count: %i\n", client.remoteIP().toString().c_str(), i + 1);
                    break;
                }
            }
            clientConnected = false;
        }

        if (server.hasClient() && !clientConnected)
        {
            server.available().stop();
            logger.debug("MAX_CONNECTION, connection closed.\n");
        }
    }

#ifdef DEBUG
    size_t bytesCount = 2;
#else
    size_t bytesCount = Serial.available();
#endif
    if (bytesCount > 0)
    {
        //logger.debug("Start send data...\n");
#ifdef DEBUG
        byte buffer[] = "T";
#else
        byte buffer[bytesCount];
        Serial.readBytes(buffer, bytesCount);
#endif

        if (myFile)
        {
#ifdef DEBUG
            logger.debug("Write data: (%s) to SD", buffer);
#else
            myFile.write(buffer, bytesCount);
#endif
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                WiFiClient &client = clients[i];
                if (&client && client && client.connected())
                {
                    client.write(buffer, bytesCount);
                    logger.debug("Data has been sent to client\n");
                    delay(1);
                }
            }
        }

#ifdef DEBUG
        delay(1000);
#else
        delay(1);
#endif
    }
    if (myFile)
    {
        myFile.flush();
    }

    delay(1);
}

template <typename T>
void Logger::debug(T format, ...)
{
    if (!test())
        return;
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 255, format, args);
    va_end(args);

    Serial.print(buffer);
}
template <typename T>
void Logger::print(T str)
{
    if (!test())
        return;
    Serial.print(str);
}
template <typename T>
void Logger::println(T str)
{
    if (!test())
        return;
    Serial.println(str);
}
template <typename T>
void Logger::printf(T format, ...)
{
    if (!test())
        return;

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 255, format, args);
    va_end(args);

    Serial.print(buffer);
}

bool Logger::test()
{
#ifdef DEBUG
    return true;
#else
    return false;
#endif
}

void getDataFromClients()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            WiFiClient &client = clients[i];
            if (&client && client && client.connected())
            {
                while (client.available())
                {
                    Serial.write(client.read());
                }

                logger.debug("Data has been sent to Serial\n");
                delay(1);
            }
        }
    }
}

void generateFileName(char *name)
{
    const size_t max_nums = 999;
    bool reset = false;
    const char prefix[] = "raw_";
    const char ext[] = ".ubx";
    size_t number = 1;
    while (number != 0)
    {
        if (number > max_nums)
        {
            number = 1;
            reset = true;
        }
        sprintf(name, "%s%i%s", prefix, number, ext);
        if (SD.exists(name) && !reset)
        {
            // file exist
            number++;
        }
        else
        {
            // next number
            return;
        }
    }
}