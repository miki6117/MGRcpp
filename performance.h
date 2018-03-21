#ifndef FIFO_PERFORMANCE_H__
#define FIFO_PERFORMANCE_H__
#include<regex>
#include<string>
#include<sstream>
#include<glog/logging.h>
#include<libconfig.h++>

enum Widths {BIT32, NONSYM, DUPLEX};
enum Directions {READ, WRITE};
enum Memories {BLOCKRAM, DISTRIBUTEDRAM, SHIFTREGISTER};
enum Patterns {COUNTER_8BIT, COUNTER_32BIT, WALKING_1};
enum Triggers {RESET, START_TIMER, STOP_TIMER, RESET_PATTERN};
enum Endpoints
{
    NUMBER_OF_COUNTS_A = 0x20,
    NUMBER_OF_COUNTS_B = 0x21,
    ERROR_COUNT = 0x22,
    PATTERN_TO_GENERATE = 0x01, // TODO: Really? Check!
    PIPE_IN = 0x80,
    PIPE_OUT = 0xa0,
    TRIGGER = 0x40
}; // TODO: enums in global header file!!

const std::regex path_regex{"(\\.|\\.\\.)[a-zA-Z0-9/\\ _-]*/$"}; // TODO: make private in config class
constexpr int MAX_PATTERN_SIZE {1073741824};

class FifoTestConfig 
{
    public:
        FifoTestConfig (const char *path_to_cfg)
        {
            // this -> path_to_cfg = path_to_cfg;
            LOG(INFO) << "Initialization FIFO config class";
            libconfig::Config cfg;
            openConfigFile(path_to_cfg, cfg);
            configureOutput(cfg);
            configureParams(cfg);
        }

        ~FifoTestConfig() 
        {
            LOG(INFO) << "Destroying FIFO config class";
        }
        
        // std::string resultfile_name;
        std::string results_path;
        std::string bitfiles_path;
        std::string result_sep;
        unsigned int iterations;
        unsigned int statistic_iter;

        std::vector<std::string> headers_v;

        std::vector<std::string> mode_v;
        std::vector<std::string> direction_v;
        std::vector<std::string> memory_v;
        std::vector<unsigned int> depth_v;
        std::vector<unsigned int> pattern_size_v;
        std::vector<std::string> pattern_v;
        std::map<std::string, unsigned int> pattern_m;
        std::map<std::string, unsigned int> mode_m;
        std::map<std::string, unsigned int> direction_m;

    private:
        std::vector<std::string> headers_default {"Time", "Mode", "Direction",
                "FifoMemoryType", "FifoDepth", "PatternSize", "DataPattern", 
                "Iterations", "StatisticalIter", "CountsInFPGA", "FPGA time(total) [us]", 
                "FPGA time(per iteration) [us]", "PC time(total) [us]", 
                "PC time(per iteration) [us]", "SpeedPC [B/s]", "SpeedFPGA [B/s]", "Errors"};
        std::vector<std::string> mode_default {"32bit", "nonsym", "duplex"};
        std::vector<std::string> direction_default {"read", "write"};
        std::vector<std::string> memory_default {"blockram", "distributedram", "shiftregister"};
        std::vector<unsigned int> depth_default {16, 64, 256, 1024};
        std::vector<std::string> pattern_default {"counter_8bit", "counter_32bit", "walking_1"};
        std::vector<unsigned int> pattern_size_default;

        void openConfigFile(const char *cfg_path, libconfig::Config &cfg);
        void configureOutput(libconfig::Config &cfg);
        void configureOutputBitfiles(libconfig::Config &cfg);
        void configureOutputParameters(const libconfig::Setting &output);
        void integerParams(const libconfig::Setting &params);
        void configureParams(libconfig::Config &cfg);

        template <class T>
        void vectorParser (std::vector<T> &parse_v, std::vector<T> &default_v, 
                           const libconfig::Setting &setting, const char *option);

};


#endif // FIFO_PERFORMANCE_H__