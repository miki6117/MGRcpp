#include "performance.h"
#include <limits>

// #undef max // Uncomment for Windows

void TransferController::saveResults()
{
	Results results(dev, cfgs);
	results.block_size = block_size;
	results.depth = depth;
	results.pattern_size = pattern_size;
	results.stat_iteration = stat_iteration;
	results.mode = mode;
	results.direction = direction;
	results.memory = memory;
	results.pattern = pattern;
	results.saveResultsToFile();
}

void TransferController::performDuplexTimer()
{
	DLOG(INFO) << "Setting duplex timer";
	Duplex duplex_timer(dev, cfgs.mode_m[mode], cfgs.pattern_m[pattern], block_size);
	duplex_timer.performTimer(pattern_size, cfgs.iterations);
	pc_duration_total = duplex_timer.pc_duration_total;
	errors = duplex_timer.errors;
}

void TransferController::performWriteTimer()
{
	DLOG(INFO) << "Setting write timer";
	Write write_timer(dev, cfgs.mode_m[mode], cfgs.pattern_m[pattern]);
	write_timer.performTimer(pattern_size, cfgs.iterations);
	pc_duration_total = write_timer.pc_duration_total;
}

void TransferController::performReadTimer()
{
	DLOG(INFO) << "Setting read timer";
	Read read_timer(dev, cfgs.mode_m[mode], cfgs.pattern_m[pattern]);
	read_timer.performTimer(pattern_size, cfgs.iterations);
	pc_duration_total = read_timer.pc_duration_total;
	errors = read_timer.errors;
}

void TransferController::runTestBasedOnParameters()
{
	DLOG(INFO) << "Current mode: " << mode;
	DLOG(INFO) << "Current direction transfer: " << direction;
	DLOG(INFO) << "Current FIFO memory: " << memory;
	DLOG(INFO) << "Current FIFO depth value: " << depth;
	DLOG(INFO) << "Current size: " << pattern_size;
	DLOG(INFO) << "Current pattern: " << pattern;

	if (transfer_mode != DUPLEX)
	{
		if (transfer_direction == READ)
		{
			performReadTimer();
		}
		else if (transfer_direction == WRITE)
		{
			performWriteTimer();
		}
	}
	else
	{
		performDuplexTimer();
	}
	saveResults();
}

void TransferController::runOnSpecificPattern()
{
	okdev::checkIfOpen(dev);
	for (const auto &pattern : cfgs.pattern_v)
	{
		this->pattern = pattern;
		DLOG(INFO) << "Current pattern: " << pattern;
		for (unsigned int i = 1; i <= cfgs.statistic_iter; i++)
		{
			stat_iteration = i;
			DLOG(INFO) << "Current statistical iteration: " << i;
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
			this->block_size = block_size;
			DLOG(INFO) << "Duplex block size set to: " << block_size;
			runOnSpecificPattern();
		}
	}
	else
	{
		block_size = pattern_size;
		DLOG(INFO) << "Non duplex mode. Block size will be the same as pattern size";
		runOnSpecificPattern();
	}
}

void TransferController::setupFPGA()
{
	std::string bitfiles = cfgs.bitfiles_path + mode + "/";
	DLOG(INFO) << "Path to bitfiles for current transfer mode: " << bitfiles;
	std::string bitfile_name = direction + "_" + mode + "_fifo_" + memory + \
							   "_" + std::to_string(depth) + ".bit";
	std::string bitfile_to_load = bitfiles + bitfile_name;
	okdev::setupFPGA(dev, bitfile_to_load);
}

void TransferController::runOnSpecificDepth(std::vector<unsigned int> &depth_v)
{
	for (const auto &depth : depth_v)
	{
		this->depth = depth;
		DLOG(INFO) << "FIFO depth value set to: " << depth;
		setupFPGA();
		for (const auto &size : cfgs.pattern_size_v)
		{
			pattern_size = size;
			runOnSpecificPatternSize();
		}
	}
}

void TransferController::specifyDepth(std::vector<unsigned int> &depth_v)
{
	transfer_direction = cfgs.direction_m[direction];

	DLOG(INFO) << "Specifying depth based on " << transfer_mode << " mode and " 
			   << direction << " direction";

	if (transfer_mode != DUPLEX)
	{
		depth_v = cfgs.depth_v;
		if (transfer_mode == NONSYM && transfer_direction == WRITE)
		{
			unsigned int depth_val_to_change = 16;
			for (auto &depth : depth_v)
			{
				if (depth == depth_val_to_change)
				{
					depth = 32;
					DLOG(WARNING) << "Changed depth 16 to 32 for NONSYM WRITE mode";
				}
			}
			DLOG(INFO) << "Depth values specified for NONSYM mode and WRITE direction";
		}
		DLOG(INFO) << "Depth values copied from config file";
	}
	else
	{
		depth_v = {2048}; // TODO: Fill in (2048????)
		DLOG(INFO) << "Depth values specified for DUPLEX mode";
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
	DLOG(INFO) << "Specified direction for " << mode;
}

void TransferController::runOnSpecificMemory(std::vector<std::string> &memory_v)
{
	std::vector<std::string> direction_v;
	specifyDirection(direction_v);

	for (const auto &direction : direction_v)
	{
		this->direction = direction;
		DLOG(INFO) << "Direction transfer mode set to: " << direction;
		for (const auto &memory : memory_v)
		{
			this->memory = memory;
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
		DLOG(INFO) << "Initialized memory vector for: " << mode;
	}
	runOnSpecificMemory(memory_v_for_specific_mode);
}

void TransferController::performTransferController()
{
	DLOG(INFO) << "Memory allocated for Results class";
	for (const auto &mode : cfgs.mode_v)
	{
		this->mode = mode;
		transfer_mode = cfgs.mode_m[mode];
		DLOG(INFO) <<  "Transfer mode set to: : " << mode;
		runOnSpecificMode();
	}
}