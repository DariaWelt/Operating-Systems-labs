#include "Notify.h"

class Daemon {
private:
	Daemon();
	void daemonise();
	static void handleSignals();
	void terminate();

	const std::string PID_PATH = "/var/run/notifyDaemon.pid";
	std::string configPath;
	bool wasTerminated = false;   // true when was SIGTERM 
	bool isInitialised = false;
public:
	Daemon(const Daemon&) = delete;
	Daemon(Daemon&&) = delete;
	static Daemon& getInstance() {
        static Daemon instance;
		return instance;
	}
	void setConfig(const std::string& path);
	void run();

};