#ifndef FIFO_PERFORMANCE_H__
#define FIFO_PERFORMANCE_H__

#include <chrono>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <string>

#include <glog/logging.h>
#include <libconfig.h++>
#include <okFrontPanelDLL.h>

constexpr int MAX_PATTERN_SIZE {1073741824};
constexpr double FIFO_CLOCK    {100.8};

enum Modes      {BIT32, NONSYM, DUPLEX};
enum Directions {READ, WRITE};
enum Memories   {BLOCKRAM, DISTRIBUTEDRAM, SHIFTREGISTER};
enum Patterns   {COUNTER_8BIT, COUNTER_32BIT, WALKING_1, ASIC};
enum Triggers   {RESET, START_TIMER, STOP_TIMER, RESET_PATTERN};
enum Endpoints
{
	NUMBER_OF_COUNTS_A = 0x20,
	NUMBER_OF_COUNTS_B = 0x21,
	ERROR_COUNT = 0x22,
	PATTERN_TO_GENERATE = 0x00,
	PIPE_IN = 0x80,
	PIPE_OUT = 0xa0,
	TRIGGER = 0x40
};

namespace okdev
{
	void checkIfOpen(okCFrontPanel *dev);
	void openDevice(okCFrontPanel *dev);
	void setupFPGA(okCFrontPanel *dev, const std::string &path_to_bitfile);
}

class Configurations 
{
	public:
		Configurations(const char *path_to_cfg) :
		mode_m{{"32bit", BIT32}, {"nonsym", NONSYM}, {"duplex", DUPLEX}},
		direction_m{{"read", READ}, {"write", WRITE}},
		pattern_m{{"counter_8bit", COUNTER_8BIT}, {"counter_32bit", COUNTER_32BIT},
			{"walking_1", WALKING_1}},
		path_regex{"(\\.|\\.\\.)[a-zA-Z0-9/\\ _-]*/$"},
		headers_default{"Time", "Mode", "Direction",
			"FifoMemoryType", "FifoDepth", "PatternSize", "BlockSize", "DataPattern", 
			"Iterations", "StatisticalIter", "CountsInFPGA", "FPGA time(total) [us]", 
			"FPGA time(per iteration) [us]", "PC time(total) [us]", 
			"PC time(per iteration) [us]", "SpeedPC [B/s]", "SpeedFPGA [B/s]", "Errors"},
		mode_default{"32bit", "nonsym", "duplex"},
		direction_default{"read", "write"},
		memory_default{"blockram", "distributedram", "shiftregister"},
		depth_default{16, 64, 256, 1024, 2048},
		block_size_default{16, 64, 256, 1024},
		pattern_default{"counter_8bit", "counter_32bit", "walking_1"}
		{
			DLOG(INFO) << "Initialization Configuration class";
			libconfig::Config cfg;
			openConfigFile(path_to_cfg, cfg);
			configureOutput(cfg);
			configureParams(cfg);
			LOG(INFO) << "Configuration class fully initialized";
		}

		~Configurations()
		{
			DLOG(INFO) << "Destroying FIFO config class";
		}
		
		std::string bitfiles_path;

		// Parameters from 'output' scope
		std::string results_path;
		std::string result_sep;

		// Parameters from 'params' scope
		std::vector<std::string> mode_v;
		std::vector<std::string> direction_v;
		std::vector<std::string> memory_v;
		std::vector<unsigned int> depth_v;
		std::vector<unsigned int> pattern_size_v;
		std::vector<unsigned int> pattern_size_duplex_v;
		std::vector<unsigned int> block_size_v;
		std::vector<std::string> pattern_v;
		unsigned int statistic_iter;
		unsigned int iterations;

		// Default hashes for params
		std::map<std::string, unsigned int> mode_m;
		std::map<std::string, unsigned int> direction_m;
		std::map<std::string, unsigned int> pattern_m;
		
		void writeHeadersToResultFile();

	private:
		const std::regex path_regex;
		std::vector<std::string> headers_v;
		
		// Default values for paramaters
		std::vector<std::string> headers_default;
		std::vector<std::string> mode_default;
		std::vector<std::string> direction_default;
		std::vector<std::string> memory_default;
		std::vector<unsigned int> depth_default;
		std::vector<unsigned int> pattern_size_default;
		std::vector<unsigned int> pattern_size_duplex_default;
		std::vector<unsigned int> block_size_default;
		std::vector<std::string> pattern_default;

		template <class T>
		void vectorParser (std::vector<T> &parse_v, std::vector<T> &default_v, 
						   const libconfig::Setting &setting, const char *option);

		void integerParams(const libconfig::Setting &params);
		void configureParams(libconfig::Config &cfg);
		void configureOutputParameters(const libconfig::Setting &output);
		void configureOutputBitfiles(libconfig::Config &cfg);
		void configureOutput(libconfig::Config &cfg);
		void openConfigFile(const char *cfg_path, libconfig::Config &cfg);
};

class Results
{
	public:
		Results(okCFrontPanel *dev, Configurations &cfgs) :
		dev{dev}, cfgs{cfgs}, MEGA{1000000}
		{
			DLOG(INFO) << "Results class initialized";
		};

		~Results()
		{
			DLOG(INFO) << "Destroying Results class";
		};

		unsigned int block_size, depth, errors, pattern_size, stat_iteration;
		std::string mode, direction, memory, pattern;
		std::chrono::duration<double, std::micro> pc_duration_total;

		void saveResultsToFile();

	private:
		const int MEGA;
		double fpga_time_total, fpga_time_periteravg;
		double pc_time_total, pc_time_periteravg;
		double fpga_speed, pc_speed;
		uint64_t fpga_counts;
		okCFrontPanel *dev;
		Configurations &cfgs;

		const std::string logTime();
		void countPCTime();
		void countFPGATime();
};

class TransferController
{
	public:
		TransferController(okCFrontPanel *dev, Configurations &cfgs) :
		dev{dev}, cfgs{cfgs}
		{
			DLOG(INFO) << "TransferController class initialized";
		}
		~TransferController()
		{
			DLOG(INFO) << "Destroying TransferController class";
		}

		void performTransferController();
	
	private:
		okCFrontPanel *dev;
		Configurations &cfgs;

		unsigned int transfer_direction;
		unsigned int transfer_mode;

		unsigned int block_size, depth, errors, pattern_size, stat_iteration;
		std::string mode, direction, memory, pattern;
		std::chrono::duration<double, std::micro> pc_duration_total;

		void saveResults();
		void performReadTimer();
		void performWriteTimer();
		void performDuplexTimer();
		void runTestBasedOnParameters();
		void runOnSpecificPattern();
		void runOnSpecificPatternSize();
		void setupFPGA();
		void runOnSpecificDepth(std::vector<unsigned int> &depth_v);
		void specifyDepth(std::vector<unsigned int> &depth_v);
		void specifyDirection(std::vector<std::string> &direction_v);
		void runOnSpecificMemory(std::vector<std::string> &memory_v);
		void runOnSpecificMode();
};

class DataGenerator
{
	public:
		DataGenerator(unsigned int mode, unsigned int pattern, unsigned int pattern_size) :
		mode{mode}, pattern{pattern}, pattern_size{pattern_size}, errors{0}
		{
			DLOG(INFO) << "DataGenerator class initialized";
		}

		unsigned int checkArrayForErrors(unsigned char *data);
		void fillArrayWithData(unsigned char *data);

	private:
		bool check_for_errors;
		unsigned char *data;
		unsigned int mode, pattern, pattern_size;
		unsigned int errors, register_size;
		uint64_t max_register_size;

		void performActionOnGeneratedData(const unsigned char &data_char, unsigned int index);
		void asic();
		void walking1();
		void counter32Bit();
		void counter8Bit();
		void determineRegisterParameters();
		void generateData();
};

class ITimer
{
	public:
		ITimer(okCFrontPanel *dev, unsigned int mode, unsigned int pattern, bool check_for_errors) :
		dev{dev}, mode{mode}, pattern{pattern}, check_for_errors{check_for_errors}
		{
			DLOG(INFO) << "Timer interface initialized";
		}
		virtual ~ITimer() {}
	
		okCFrontPanel *dev;

		bool check_for_errors;
		unsigned int errors;
		std::chrono::duration<double, std::micro> pc_duration_total;
		std::chrono::time_point<std::chrono::system_clock> timer_start, timer_stop;

		void performActionOnData(unsigned char *data, unsigned int pattern_size);
		void prepareForTransfer();

		virtual void performTimer(unsigned int pattern_size, unsigned int iterations) = 0;

	private:
		unsigned int mode, pattern;

};

class Read : public ITimer
{
	public:
		Read(okCFrontPanel *dev, unsigned int mode, unsigned int pattern) :
		ITimer(dev, mode, pattern, true)
		{
			DLOG(INFO) << "Read class initialized";
		}

		virtual void performTimer(unsigned int pattern_size, unsigned int iterations);
};

class Write : public ITimer
{
	public:
		Write(okCFrontPanel *dev, unsigned int mode, unsigned int pattern) :
		ITimer(dev, mode, pattern, false)
		{
			DLOG(INFO) << "Write class initialized";
		}

		virtual void performTimer(unsigned int pattern_size, unsigned int iterations);
};

class Duplex : public ITimer
{
	public:
		Duplex(okCFrontPanel *dev, unsigned int mode, unsigned int pattern, unsigned int block_size) :
		ITimer(dev, mode, pattern, false), block_size{block_size}
		{
			DLOG(INFO) << "Duplex class initialized";
		}

		virtual void performTimer(unsigned int pattern_size, unsigned int iterations);

	private:
		unsigned int block_size;
		void checkIfReceivedEqualsSend(unsigned char *send_data, unsigned char *received_data);
};

#endif // FIFO_PERFORMANCE_H__