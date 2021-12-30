  
#include <iostream>
#include <string.h>
#include <vector>
#include <atomic>
#include <chrono>
#include <thread>
#include <regex>
#include <fstream>
#include <filesystem>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>

static std::atomic_bool g_monitoring(false);

std::string toString(const rapidjson::Value& json)
{
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    json.Accept(writer);
    return buffer.GetString();
}

void parseText(std::ifstream& ifs)
{
    for (std::string line; std::getline(ifs, line);)
    {
        std::cout << line << std::endl;
    }
}

void parseRegex(std::ifstream& ifs)
{
    std::regex rx("^\\[([A-Z])\\]:\\s+(\\d{2}-\\d{2}-\\d{4}\\s+\\d{2}:\\d{2}:\\d{2}\\.\\d+)\\s+\\[([^\\]]+)\\]:\\s+(.*)");

    for (std::string line; std::getline(ifs, line);)
    {
        //std::cout << line << std::endl;

        std::smatch m;
        std::regex_search(line, m, rx);
        if (m.size())
        {
            for (std::size_t i = 1; i < m.size(); ++i)
            {
                std::cout << m[i] << " ";
            }
            std::cout << std::endl;
        }
    }
}

void parseJson(std::ifstream& ifs)
{
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document d;
    d.ParseStream<rapidjson::kParseStopWhenDoneFlag>(isw);
    if (!d.HasParseError())
    {
        std::cout << toString(d) << std::endl;
    }
}

void keepMonitoringFile(const std::string& file_name)
{
    std::ifstream ifs(file_name);

    if(!ifs.is_open())
    {
        std::cout << "NOT OPEN" << std::endl;
        return;
    }

    ifs.seekg(0, std::ios::end);
    auto last_file_size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    while(g_monitoring)
    {
        if (!std::filesystem::exists(file_name))
        {
            std::cout << "File deleted" << std::endl;
            return;
        }
        ifs.clear();

        auto curr_pos = ifs.tellg();
        ifs.seekg(0, std::ios::end);
        auto file_size = ifs.tellg();
        if (file_size < last_file_size)
        {
            std::cout << "File recreated" << std::endl;
            return;
        }
        last_file_size = file_size;
        ifs.seekg(curr_pos, std::ios::beg);

        ifs.peek();
        if (ifs.fail())
        {
            std::cout << "FAILED" << std::endl;
        }
        while(!ifs.eof())
        {
            //parseJson(ifs);
            //parseRegex(ifs);
            parseText(ifs);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void startMonitorFile(const std::string& file_name)
{
    g_monitoring = true;

    while(g_monitoring)
    {
        if (!std::filesystem::exists(file_name))
        {
            std::cout << "File does not exist: " << file_name << std::endl;
            return;
        }
        keepMonitoringFile(file_name);
    }
}

void stopMonitorFile(const std::string& file_name)
{
    g_monitoring = false;
}

int main()
{
    startMonitorFile("/home/rafael/Dev/QLogViewer/log.txt");

    return 0;
}
// echo '{"LogLevel":"TEST","DateTime":"28-12-2021 18:04:02.00205"}' >> /home/rafael/Dev/QLogViewer/log.json
// echo '[T]: 28-12-2021 18:26:01.191 [test/cpp]: Test' >> /home/rafael/Dev/QLogViewer/log.txt

