#include "performance.h"

int main(int argc, char *argv[]) {
	google::InitGoogleLogging(argv[0]);
	LOG(INFO) << "Program started";
	okCFrontPanel *dev = new okCFrontPanel();
	okdev::openDevice(dev);

	const char *default_cfgpath;
		if (argc > 1) default_cfgpath = argv[1];
		else default_cfgpath = "../performance.cfg";
	LOG(INFO) << "Path to config file: " << std::string(default_cfgpath);

	Configurations configs(default_cfgpath);
	configs.writeHeadersToResultFile();

	TransferController tc(dev, configs);
	tc.performTransferController();

	delete dev;
}
