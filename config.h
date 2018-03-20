#ifndef FIFO_CONFIG_H__
#define FIFO_CONFIG_H__
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

constexpr std::regex path_regex{"(\\.|\\.\\.)[a-zA-Z0-9/\\ _-]*/$"}; // TODO: make private in config class
constexpr int MAX_PATTERN_SIZE {1073741824};

class FifoTestConfig 
{
    public:
        FifoTestConfig (std::string path_to_cfg)
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
        unsigned char result_sep;
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
        std::vector<string> mode_default = {"32bit", "nonsym", "duplex"};
        std::vector<string> direction_default = {"read", "write"};
        std::vector<string> memory_default = {"blockram", "distributedram", "shiftregister"};
        std::vector<unsigned int> depth_default = {16, 64, 256, 1024};
        std::vector<string> patterns_default = {"counter_8bit", "counter_32bit", "walking_1"};

        void openConfigFile(std::string &cfg_path, libconfig::Config &cfg);
        void configureOutput(libconfig::Config &cfg);
        void configureOutputBitfiles(libconfig::Config &cfg);
        void configureOutputParameters(libconfig::Setting &output);
        void integerParams(const libconfig::Setting &params);

        template <class T>
        void vectorParser (std::vector<T> &parse_v, std::vector<T> &default_v, 
                           const libconfig::DSetting &setting, std::string &option)

};

template <class T>
void FifoTestConfig::vectorParser (std::vector<T> &parse_v, std::vector<T> &default_v, 
                                   const libconfig::Setting &setting, std::string &option)
{
    for (auto i=0; i<setting[option].getLength(); i++)
    {
        T set = setting[option][i];
        if (std::find(default_v.begin(), default_v.end(), set) != default_v.end())
        {
            parse_v.push_back(setting[option][i]);
        }
        else
        {
            LOG(FATAL) << set << " <- is not valid parameter for " << option << " option!";
        }
    }
    if (parse_v.size() == 0)
    {
        for (const auto &def : default_v)
        {
            parse_v.push_back(def);
        }
        LOG(WARNING) << "Overriding " << option << " parameter with "
                     << default_v.size() << " default attributes";

    else
    {
        std::stringstream parameters;
        parameters << "Values for '" << option << "' parameter has following parameters: ";
        for (const auto &par : parse_v)
        {
            parameters << par << " ";
        }
        LOG(INFO) << parameters.str();
    }
}

void FifoTestConfig::integerParams(const libconfig::Setting &params)
{
    iterations = params["iterations"];
    if (iterations <= 0)
    {
        iterations {1};
        LOG(ERROR) << "Iterations must be greater than 0. 
                       Setting default value: 1";
    }

    statistic_iter = params["statistic_iter"];
    if (statistic_iter <= 0)
    {
        statistic_iter {1};
        LOG(ERROR) << "Statistic iterations must be greater than 0. 
                       Setting default value: 1";
    }
}

void FifoTestConfig::configureParams(libconfig::Config &cfg)
{
    const libconfig::Setting &params = cfg.lookup("params");

    vectorParser(width_v, width_default, params, "width");
    vectorParser(direction_v, direction_default, params, "direction");
    vectorParser(memory_v, memory_default, params, "memory");
    vectorParser(depth_v, depth_default, params, "depth");
    vectorParser(patterns_v, patterns_default, params, "patterns");
    for (auto size=16; size<=MAX_PATTERN_SIZE; size+=size)
        pattern_sizes_default.push_back(size);
    vectorParser(pattern_sizes_v, pattern_sizes_default, params, "pattern_sizes");

    integerParams(params);
}

void FifoTestConfig::configureOutputParameters(libconfig::Setting &output)
{
    vectorParser(headers_v, headers_default, output, "headers");
    results_path = output["results_path"].c_str();
    if (std::regex_search(results_path, path_regex))
    {
        std::string resultfile_name = output["resultfile_name"].c_str();
        results_path += resultfile_name;
        LOG(INFO) << "Results will be saved in: " << results_path;
    } 
    else 
    {
        LOG(FATAL) << "Inappropiate path to results file. The path must be 
                       referred to current location"
    }

    result_sep = output["result_sep"].c_str();
    LOG(INFO) << "Separator in results file set to: " << result_sep;
}

void FifoTestConfig::configureOutputBitfiles(libconfig::Config &cfg)
{
    bitfiles_path = cfg.lookup("bitfiles_path").c_str();
    if (std::regex_search(bitfiles_path, path_regex))
    {
        LOG(INFO) << "Path to bitfiles: " << bitfiles_path;
    } 
    else 
    {
        LOG(FATAL) << "Inappropiate path to bitfiles. The path must be referred 
                       to current location";
    }
}

void FifoTestConfig::configureOutput(libconfig::Config &cfg)
{
    const libconfig::Setting &output = cfg.lookup("output");
    try
    {
        configureOutputBitfiles(cfg);
        configureOutputParameters(output);
    }
    catch (const libconfig::SettingTypeException &stexp)
    {
        LOG(FATAL) << "Setting type exception caught at: " << stexp.getPath();
    }
    LOG(INFO) << "No setting type exception occured in config file";
}

void FifoTestConfig::openConfigFile(std::string &cfg_path, libconfig::Config &cfg)
{
    try
    {
        LOG(INFO) << "Path to cfg file: " << cfg_path;
        cfg.readFile(cfg_path);
    }
    catch (const libconfig::FileIOException &fioex)
    {
        LOG(FATAL) << "I/O error while reading file. Closing program...";
    }
    catch(const libconfig::ParseException &pex)
    {
        LOG(FATAL) << "Parse error at " << pex.getFile() << ":" << pex.getLine()
                << " - " << pex.getError();
    }
    LOG(INFO) << "No config file I/O nor parsing errors occured";
}


#endif // FIFO_CONFIG_H__