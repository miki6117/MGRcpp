#ifndef FIFO_PERFORMANCE_H__
#define FIFO_PERFORMANCE_H__
// TODO: Change some logs to DLOG
#include <chrono>
#include <ctime>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

#include <glog/logging.h>
#include <libconfig.h++>
#include <okFrontPanelDLL.h>

constexpr int MAX_PATTERN_SIZE {1073741824};
constexpr double FIFO_CLOCK    {100.8};

enum Widths     {BIT32, NONSYM, DUPLEX};
enum Directions {READ, WRITE};
enum Memories   {BLOCKRAM, DISTRIBUTEDRAM, SHIFTREGISTER};
enum Patterns   {COUNTER_8BIT, COUNTER_32BIT, WALKING_1};
enum Triggers   {RESET, START_TIMER, STOP_TIMER, RESET_PATTERN};
enum Endpoints
{
    NUMBER_OF_COUNTS_A = 0x20,
    NUMBER_OF_COUNTS_B = 0x21,
    ERROR_COUNT = 0x22,
    PATTERN_TO_GENERATE = 0x01, // TODO: Really? Check!
    PIPE_IN = 0x80,
    PIPE_OUT = 0xa0,
    TRIGGER = 0x40
};


class Configurations 
{
    public:
        Configurations (const char *path_to_cfg)
        {
            LOG(INFO) << "Initialization Configuration class";
            libconfig::Config cfg;
            openConfigFile(path_to_cfg, cfg);
            configureOutput(cfg);
            configureParams(cfg);
            LOG(INFO) << "Configuration class fully initialized";
        }

        ~Configurations()
        {
            LOG(INFO) << "Destroying FIFO config class";
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
        std::vector<std::string> pattern_v;
        unsigned int statistic_iter;
        unsigned int iterations;

        // Default hashes for params
        std::map<std::string, unsigned int> mode_m 
            {{"32bit", BIT32}, {"nonsym", NONSYM}, {"duplex", DUPLEX}};
        std::map<std::string, unsigned int> direction_m 
            {{"read", READ}, {"write", WRITE}};
        std::map<std::string, unsigned int> pattern_m 
            {{"counter_8bit", COUNTER_8BIT}, {"counter_32bit", COUNTER_32BIT}, 
             {"walking_1", WALKING_1}};
        
        void writeHeadersToResultFile();

    private:
        const std::regex path_regex{"(\\.|\\.\\.)[a-zA-Z0-9/\\ _-]*/$"};
        std::vector<std::string> headers_v;
        
        // Default values for paramaters
        std::vector<std::string> headers_default {"Time", "Mode", "Direction",
                "FifoMemoryType", "FifoDepth", "PatternSize", "DataPattern", 
                "Iterations", "StatisticalIter", "CountsInFPGA", "FPGA time(total) [us]", 
                "FPGA time(per iteration) [us]", "PC time(total) [us]", 
                "PC time(per iteration) [us]", "SpeedPC [B/s]", "SpeedFPGA [B/s]", "Errors"};
        std::vector<std::string> mode_default {"32bit", "nonsym", "duplex"};
        std::vector<std::string> direction_default {"read", "write"};
        std::vector<std::string> memory_default {"blockram", "distributedram", "shiftregister"};
        std::vector<unsigned int> depth_default {16, 64, 256, 1024};
        std::vector<unsigned int> pattern_size_default;
        std::vector<std::string> pattern_default {"counter_8bit", "counter_32bit", "walking_1"};

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
        Results(okCFrontPanel *dev, Configurations &cfgs) : dev{dev}, cfgs{cfgs}
        {
            LOG(INFO) << "Results class initialized";
        };

        ~Results()
        {
            LOG(INFO) << "Destroying Results class";
        };

        unsigned int depth, errors, pattern_size, stat_iteration;
        std::string mode, direction, memory, pattern;
        std::chrono::duration<double, std::micro> pc_duration_total;

        void saveResultsToFile();

    private:
        const int MEGA = 1000000;
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

class TransferTest
{
    public:
        TransferTest (okCFrontPanel *dev, Configurations &cfgs) : dev{dev}, cfgs{cfgs}
        {
            LOG(INFO) << "TransferTest class initialized";
        }
        ~TransferTest();

        void performTransferTest();
    
    private:
        okCFrontPanel *dev;
        Configurations &cfgs;
        Results *r;

        void runTestBasedOnParameters();
        void checkIfOpen();
        void setupFPGA();
        void runonSpecificDepth(std::vector<unsigned int> &depth_v);
        void specifyDepth();
        void runOnSpecificMemory(std::vector<std::string> &memory_v);
        void runOnSpecificMode();
};

#endif // FIFO_PERFORMANCE_H__