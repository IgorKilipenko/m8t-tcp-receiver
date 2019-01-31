#ifndef logger_h
#define logger_h

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
};

#endif