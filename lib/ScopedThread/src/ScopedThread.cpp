#include <thread>
#include <stdexcept>
#include <string>

class ScopedThread {
  public:
	explicit ScopedThread(std::thread thread, std::string name) : _thread(std::move(thread)), _threadName{name} {
		if (!_thread.joinable())
			throw std::logic_error(“No thread”);
	}
	~ScopedThread() { _thread.join(); }
	ScopedThread(ScopedThread const &) = delete;
	ScopedThread &operator=(ScopedThread const &) = delete;

	std::string getThreadName() { return _threadName; }

  protected:
	std::thread _thread;
	std::string _threadName;
};