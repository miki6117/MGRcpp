#include "performance.h"

void Configurations::writeHeadersToResultFile()
{
    std::fstream result_file;
    result_file.open(results_path, std::ios::out | std::ios::app);
    if (result_file.good())
    {
        for (const auto &header : headers_v)
        {
            result_file << header << result_sep;
        }
        result_file << std::endl;
        result_file.close();
        LOG(INFO) << "Headers have been written to: " << results_path;
    }
    else
    {
        LOG(FATAL) << "Unable to open " << results_path;
    }
}

template <class T>
void Configurations::vectorParser (std::vector<T> &parse_v, std::vector<T> &default_v, 
                                   const libconfig::Setting &setting, const char *option)
{
    for (auto i=0; i<setting[option].getLength(); i++)
    {
        T set = setting[option][i];
        if (std::find(default_v.begin(), default_v.end(), set) != default_v.end())
        {
            DLOG(INFO) << "Parsing " << setting[option][i] << " to parse_v";
            parse_v.push_back(setting[option][i]);
        }
        else
        {
            LOG(FATAL) << set << " <- is not a valid parameter for " << option << " option!";
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
    }
    else
    {
        std::stringstream parameters;
        parameters << "Following values for '" << option << "' parameter were parsed: ";
        for (const auto &par : parse_v)
        {
            parameters << par << " ";
        }
        LOG(INFO) << parameters.str();
    }
}

void Configurations::integerParams(const libconfig::Setting &params)
{
    iterations = params["iterations"];
    if (iterations <= 0)
    {
        iterations = 1;
        LOG(ERROR) << "Iterations must be greater than 0. "
                   << "Setting default value: 1";
    }

    statistic_iter = params["statistic_iter"];
    if (statistic_iter <= 0)
    {
        statistic_iter = 1;
        LOG(ERROR) << "Statistic iterations must be greater than 0. "
                   << "Setting default value: 1";
    }
}

void Configurations::configureParams(libconfig::Config &cfg)
{
    const libconfig::Setting &params = cfg.lookup("params");

    vectorParser(mode_v, mode_default, params, "mode");
    vectorParser(direction_v, direction_default, params, "direction");
    vectorParser(memory_v, memory_default, params, "memory");
    vectorParser(depth_v, depth_default, params, "depth");
    vectorParser(pattern_v, pattern_default, params, "pattern");
    for (unsigned int size=16; size<=MAX_PATTERN_SIZE; size+=size)
    {
        DLOG(INFO) << "Pushing back size to pattern_size_default: " << size;
        pattern_size_default.push_back(size);

    }
    vectorParser(pattern_size_v, pattern_size_default, params, "pattern_size");

    integerParams(params);
}

void Configurations::configureOutputParameters(const libconfig::Setting &output)
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
        LOG(FATAL) << "Inappropiate path to results file. The path must be "
                   << "referred to the current location";
    }

    result_sep = output["result_sep"].c_str();
    LOG(INFO) << "Separator in results file set to: " << result_sep;
}

void Configurations::configureOutputBitfiles(libconfig::Config &cfg)
{
    bitfiles_path = cfg.lookup("bitfiles_path").c_str();
    if (std::regex_search(bitfiles_path, path_regex))
    {
        LOG(INFO) << "Path to bitfiles: " << bitfiles_path;
    } 
    else 
    {
        LOG(FATAL) << "Inappropiate path to bitfiles. The path must be referred "
                   << "to the current location";
    }
}

void Configurations::configureOutput(libconfig::Config &cfg)
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
    LOG(INFO) << "No setting type exception occurred in config file "
              << "while reading 'output' settings";
}

void Configurations::openConfigFile(const char *cfg_path, libconfig::Config &cfg)
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
    LOG(INFO) << "No config file I/O nor parsing errors occurred";
}
