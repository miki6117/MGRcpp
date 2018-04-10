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
    PATTERN_TO_GENERATE = 0x00,
    PIPE_IN = 0x80,
    PIPE_OUT = 0xa0,
    TRIGGER = 0x40
};


class Configurations 
{
    public:
        Configurations(const char *path_to_cfg)
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
        
        // Default values for paramaters TODO: change to const
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
        std::vector<unsigned int> pattern_size_duplex_default;
        std::vector<unsigned int> block_size_default {16, 64, 256, 1024};
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

class TransferController
{
    public:
        TransferController(okCFrontPanel *dev, Configurations &cfgs) : dev{dev}, cfgs{cfgs}
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
        Results *r;
        unsigned int transfer_mode;

        void runTestBasedOnParameters();
        void checkIfOpen();
        void runOnSpecificPattern();
        void runOnSpecificPatternSize();
        void setupFPGA();
        void runOnSpecificDepth(std::vector<unsigned int> &depth_v);
        void specifyDepth(std::vector<unsigned int> &depth_v);
        void specifyDirection(std::vector<std::string> &direction_v);
        void runOnSpecificMemory(std::vector<std::string> &memory_v);
        void runOnSpecificMode();
};

class ITimer
{
    public:
        ITimer(okCFrontPanel *dev, Results *r, Configurations &cfgs) :
        dev{dev}, r{r}, cfgs{cfgs}
        {
            DLOG(INFO) << "Timer interface initialized";
        }
        virtual ~ITimer() {}
        void performTimer(unsigned char *data);
    
        bool checkForErrors;
        okCFrontPanel *dev;
        Results *r;
        Configurations &cfgs;
        std::chrono::time_point<std::chrono::system_clock> timer_start, timer_stop;

        virtual void timer(unsigned char *data) = 0;
        void generateData(unsigned char *data);
    private:
        void determineRegisterParameters(unsigned int mode, unsigned int &register_size, uint64_t &max_register_size);
        void performActionOnGeneratedData(const unsigned char &data_char, unsigned char *data, int index);
};

class Read : public ITimer
{
    public:
        Read(okCFrontPanel *dev, Results *r, Configurations &cfgs) :
        ITimer(dev, r, cfgs)
        {
            checkForErrors = true;
            DLOG(INFO) << "Read class initialized";
        }

    private:
        virtual void timer(unsigned char *data);

};

class Write : public ITimer
{
    public:
        Write(okCFrontPanel *dev, Results *r, Configurations &cfgs) :
        ITimer(dev, r, cfgs)
        {
            checkForErrors = false;
            DLOG(INFO) << "Write class initialized";
        }

    private:
        virtual void timer(unsigned char *data);

};

class Duplex : public ITimer
{
    public:
        Duplex(okCFrontPanel *dev, Results *r, Configurations &cfgs) :
        ITimer(dev, r, cfgs)
        {
            checkForErrors = false;
            DLOG(INFO) << "Duplex class initialized";
        }

    private:
        virtual void timer(unsigned char *data);

};

#endif // FIFO_PERFORMANCE_H__