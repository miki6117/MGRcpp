#include "performance.h"

void openDevice(okCFrontPanel *dev)
{
	LOG(INFO) << "Trying to open device";
    auto err_code = dev->OpenBySerial();
    if (err_code == okCFrontPanel::NoError)
    {
		LOG(INFO) << "Open status: " << dev->GetErrorString(err_code);
    }
    else
    {
		LOG(FATAL) << "Failed to open device: " << dev->GetErrorString(err_code);
    }

    dev->ResetFPGA();
}

int main(int argc, char *argv[]) {
  // google::InitGoogleLogging(argv[0]);
	LOG(INFO) << "Program started";
	okCFrontPanel *dev = new okCFrontPanel();
	openDevice(dev);

	const char *default_cfgpath;
    if (argc > 1) default_cfgpath = argv[1];
    else default_cfgpath = "../performance.cfg";
	LOG(INFO) << "Path to config file: " << std::string(default_cfgpath);

	Configurations configs(default_cfgpath);
	configs.writeHeadersToResultFile();
}
