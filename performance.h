#ifndef FIFO_PERFORMANCE_H__
#define FIFO_PERFORMANCE_H__

#include <chrono>
#include <ctime>
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
        std::vector<std::string> headers_v;
        std::string results_path;
        std::string result_sep;

        // Parameters from 'params' scope
        std::vector<std::string> mode_v;
        std::vector<std::string> direction_v;
        std::vector<std::string> memory_v;
        std::vector<unsigned int> depth_v;
        std::vector<unsigned int> pattern_size_v;
        std::vector<std::string> pattern_v;
        std::map<std::string, unsigned int> pattern_m;
        std::map<std::string, unsigned int> mode_m;
        std::map<std::string, unsigned int> direction_m;
        unsigned int statistic_iter;
        unsigned int iterations;

    private:
        const std::regex path_regex{"(\\.|\\.\\.)[a-zA-Z0-9/\\ _-]*/$"};

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

class Results // TODO: add config object as pointer
{
    public:
        Results(char results_separator, okCFrontPanel *dev, std::string results_file_name) : // TODO: if config object passed, then delete separator and results_file_name
                rs{results_separator},
                dev{dev},
                results_file_name{results_file_name}
        {
            LOG(INFO) << "Results class initialized";
        };

        ~Results()
        {
            LOG(INFO) << "Destroying Results class";
        };

        const char rs;
        okCFrontPanel *dev;
        const std::string results_file_name;
        const int MEGA = 1000000;

        unsigned int depth, errors, pattern_size, stat_iteration;
        uint64_t fpga_counts;
        double fpga_time_total, fpga_time_periteravg;
        double fpga_speed, pc_speed;
        double pc_time_total, pc_time_periteravg;
        std::string direction, memory, pattern, width;
        std::chrono::duration<double, std::micro> pc_duration_total;

        void saveResultsToFile();

    private:
        void countPCTime();
        void countFPGATime();
        const std::string logTime();
        
};

const std::string Results::logTime() // TODO: #include<ctime>
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

    return buf;
}

void Results::countPCTime()
{
    pc_time_total = pc_duration_total.count();
    pc_time_periteravg = pc_time_total / iterations;// TODO: iterations from config file!!
    pc_speed = static_cast<double>(pattern_size) * MEGA / pc_time_periteravg;
    LOG(INFO) << "Counted PC time for single duration: " << pc_time_periteravg << " msec";
    LOG(INFO) << "Counted speed on PC side: " << pc_speed << " B/s";
}

void Results::countFPGATime()
{
    uint64_t number_of_counts;
    dev->UpdateWireOuts();
    number_of_counts = dev->GetWireOutValue(NUMBER_OF_COUNTS_A);
    number_of_counts += static_cast<uint64_t>(dev->GetWireOutValue(NUMBER_OF_COUNTS_B) << 32);
    if (direction_m[direction] == WRITE) errors = dev->GetWireOutValue(ERROR_COUNT); // TODO: direction_m from config file!

    fpga_time_total = number_of_counts / FIFO_CLOCK;

    fpga_time_periteravg = fpga_time_total / iterations;
    fpga_speed = static_cast<double>(pattern_size) * MEGA / fpga_time_periteravg; // TODO: logs!
    LOG(INFO) << "FPGA clock counts: " << number_of_counts;
    LOG(INFO) << "Counted FPGA total transfer time: " << fpga_time_total << " msec";
    LOG(INFO) << "Counted FPGA time for single duration: " << fpga_time_periteravg << " msec";
    LOG(INFO) << "Counted speed on FPGA side: " << fpga_speed << " B/s";
    LOG(WARNNG) << "Errors detected during transfer: " << errors;
}

void Results::saveResultsToFile()
{
    countPCTime();
    countFPGATime();
    std::fstream result_file;
    result_file.open(results_file_name, std::ios::out | std::ios::app);
    if (result_file.good())
    {
        result_file << logTime() << rs << width << rs << direction << rs;
        result_file << memory << rs << depth << rs << pattern_size << rs;
        result_file << pattern << rs << iterations << rs << stat_iteration << rs << fpga_counts << rs;
        result_file << fpga_time_total << rs << fpga_time_periteravg << rs;
        result_file << pc_time_total << rs << pc_time_periteravg << rs;
        result_file << pc_speed << rs << fpga_speed  << rs << errors;
        result_file << std::endl;
        result_file.close();
        LOG(INFO) << "All results saved to " << results_file_name;
    }
    else
    {
        LOG(FATAL) << "Unable to open " << results_file_name << " file during saving results";
    }
}

#endif // FIFO_PERFORMANCE_H__