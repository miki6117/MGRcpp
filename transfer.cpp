#include "performance.h"
#include <limits>

// #undef max // Uncomment for Windows

void TransferController::runTestBasedOnParameters()
{
	DLOG(INFO) << "Current mode: " << r->mode;
	DLOG(INFO) << "Current direction transfer: " << r->direction;
	DLOG(INFO) << "Current FIFO memory: " << r->memory;
	DLOG(INFO) << "Current FIFO depth value: " << r->depth;
	DLOG(INFO) << "Current size: " << r->pattern_size;
	DLOG(INFO) << "Current pattern: " << r->pattern;

	unsigned char* data = new unsigned char[r->pattern_size];
	auto dir = cfgs.direction_m[r->direction];

	if (transfer_mode != DUPLEX)
	{
		if (dir == READ)
		{
			DLOG(INFO) << "Setting read timer";
			Read readTimer(dev, r, cfgs);
			readTimer.performTimer(data); // TODO: maybe data can be in ITimer class declared??
		}
		else if (dir == WRITE)
		{
			DLOG(INFO) << "Setting write timer";
			Write writeTimer(dev, r, cfgs);
			writeTimer.performTimer(data);
		}
	}
	else
	{
		DLOG(INFO) << "Setting bidir timer";
		Duplex duplexTimer(dev, r, cfgs);
		duplexTimer.performTimer(data);
	}
	r->saveResultsToFile();
	delete[] data;
}

void TransferController::checkIfOpen()
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

void TransferController::runOnSpecificPattern()
{
	checkIfOpen();
	for (const auto &pattern : cfgs.pattern_v)
	{
		DLOG(INFO) << "Current pattern: " << pattern;
		for (auto i = 1; i <= cfgs.statistic_iter; i++)
		{
			DLOG(INFO) << "Current statistical iteration: " << i;
			r->stat_iteration = i;
			r->pattern = pattern;
			runTestBasedOnParameters();
		}
	}
}

void TransferController::runOnSpecificPatternSize()
{
	if (transfer_mode == DUPLEX)
	{
		for (const auto &block_size : cfgs.block_size_v)
		{
			r->block_size = block_size;
			DLOG(INFO) << "Duplex block size set to: " << block_size;
			runOnSpecificPattern();
		}
	}
	else
	{
		r->block_size = r->pattern_size;
		DLOG(INFO) << "Non duplex mode. Block size will be the same as pattern size";
		runOnSpecificPattern();
	}
}

void TransferController::setupFPGA()
{
	std::string bitfiles = cfgs.bitfiles_path + r->mode + "/";
	DLOG(INFO) << "Path to bitfiles for current transfer mode: " << bitfiles;
	std::string bitfile_name = r->direction + "_" + r->mode + "_fifo_" + r->memory + \
							   "_" + std::to_string(r->depth) + ".bit";
	std::string bitfile_to_load = bitfiles + bitfile_name;
	DLOG(INFO) << "FPGA configure file: " << bitfile_to_load;
	auto err_code = dev->ConfigureFPGA(bitfile_to_load);
	if (err_code == okCFrontPanel::NoError) 
	{
		LOG(INFO) << "Configure status for file " << bitfile_name << " : all ok";
	}
	else
	{
		LOG(FATAL) << "FPGA configuration failed [" << dev->GetErrorString(err_code)
				   << "] for file " << bitfile_name;
	}
}

void TransferController::runOnSpecificDepth(std::vector<unsigned int> &depth_v)
{
	for (const auto &depth : depth_v)
	{
		r->depth = depth;
		DLOG(INFO) << "FIFO depth value set to: " << depth;
		setupFPGA();
		for (const auto &size : cfgs.pattern_size_v)
		{
			r->pattern_size = size;
			runOnSpecificPatternSize();
		}
	}
}

void TransferController::specifyDepth(std::vector<unsigned int> &depth_v)
{
	auto direction = cfgs.direction_m[r->direction];

	DLOG(INFO) << "Specifying depth based on " << transfer_mode << " mode and " 
			   << direction << " direction";

	if (transfer_mode == NONSYM && direction == WRITE)
	{
		depth_v = {32, 64, 256, 1024}; //TODO: If 16 in depth_v then 32!!
		DLOG(INFO) << "Depth values specified for NONSYM mode and WRITE direction";
	}
	else if (transfer_mode == DUPLEX)
	{
		depth_v = {1024}; //TODO: Fill in (2048????)
		DLOG(INFO) << "Depth values specified for DUPLEX mode";
	}
	else
	{
		depth_v = cfgs.depth_v;
		DLOG(INFO) << "Depth values copied from config file";
	}
}

void TransferController::specifyDirection(std::vector<std::string> &direction_v)
{
	if (transfer_mode == DUPLEX)
	{
		direction_v.push_back("bidir");
	}
	else
	{
		direction_v = cfgs.direction_v;
	}
	DLOG(INFO) << "Specified direction for " << r->mode;
}

void TransferController::runOnSpecificMemory(std::vector<std::string> &memory_v)
{
	std::vector<std::string> direction_v;
	specifyDirection(direction_v);

	for (const auto &direction : direction_v)
	{
		r->direction = direction;
		DLOG(INFO) << "Direction transfer mode set to: " << direction;
		for (const auto &memory : memory_v)
		{
			r->memory = memory;
			DLOG(INFO) << "FIFO memory mode set to: " << memory;
			std::vector<unsigned int> depth_v;
			specifyDepth(depth_v);
			runOnSpecificDepth(depth_v);
		}
	}
}

void TransferController::runOnSpecificMode()
{
	std::vector<std::string> memory_v_for_specific_mode;

	if (transfer_mode == NONSYM)
	{
		memory_v_for_specific_mode = {"blockram"};
		DLOG(WARNING) << "FYI: For nonsym mode, the only valid memory is blockram";
	}
	else
	{
		memory_v_for_specific_mode = cfgs.memory_v;
		DLOG(INFO) << "Initialized memory vector for: " << r->mode;
	}
	runOnSpecificMemory(memory_v_for_specific_mode);
}

void TransferController::performTransferController()
{
	r = new Results(dev, cfgs);
	DLOG(INFO) << "Memory allocated for Results class";
	for (const auto &mode : cfgs.mode_v)
	{
		r->mode = mode;
		transfer_mode = cfgs.mode_m[mode];
		DLOG(INFO) <<  "Transfer mode set to: : " << mode;
		runOnSpecificMode();
	}
	delete r;
}