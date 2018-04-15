#include "performance.h"

void okdev::checkIfOpen(okCFrontPanel *dev)
{
	DLOG(INFO) << "Checking if device is open...";
	if (dev->IsOpen())
	{
		DLOG(INFO) << "Device is open";
	}
	else
	{
		LOG(FATAL) << "Device disconnected. Program stopped";
	}
}

void okdev::openDevice(okCFrontPanel *dev)
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

void okdev::setupFPGA(okCFrontPanel *dev, const std::string &path_to_bitfile)
{
	DLOG(INFO) << "FPGA configure file: " << path_to_bitfile;
	auto err_code = dev->ConfigureFPGA(path_to_bitfile);
	if (err_code == okCFrontPanel::NoError) 
	{
		LOG(INFO) << "Configure status for file " << path_to_bitfile << " : all ok";
	}
	else
	{
		LOG(FATAL) << "FPGA configuration failed [" << dev->GetErrorString(err_code)
				   << "] for file " << path_to_bitfile;
	}
}
