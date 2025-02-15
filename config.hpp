#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "include/json.hpp"

struct TriggerConfig
{
    std::string triggerKey; // 被触发的键
    std::string action;     // "click" 或 "hold"
};

struct ListenerConfig
{
    std::string listenKey;               // 监听的键
    std::vector<TriggerConfig> triggers; // 触发的键列表
};

class Config
{
public:
    std::vector<ListenerConfig> listeners;
    std::string pauseKey; // 新增：快捷键启停功能的按键

    explicit Config(const std::string &filename) : config_file(filename)
    {
        ensureConfigExists(); // 确保配置文件存在
        load();
    }

    void load()
    {
        std::ifstream file(config_file);
        if (!file.is_open())
        {
            std::cerr << "Error: Could not open config file: " << config_file << std::endl;
            return;
        }

        try
        {
            nlohmann::json j;
            file >> j;

            // 读取 pauseKey 字段，若不存在则使用默认值 "P"
            pauseKey = j.value("pauseKey", "P");

            listeners.clear();

            // 安全检查 "listeners" 是否为数组
            if (j.contains("listeners") && j["listeners"].is_array())
            {
                for (const auto &listener : j["listeners"])
                {
                    ListenerConfig lc;
                    lc.listenKey = listener.value("listenKey", ""); // 默认空字符串

                    for (const auto &trigger : listener.value("triggers", nlohmann::json::array()))
                    {
                        TriggerConfig tc;
                        tc.triggerKey = trigger.value("triggerKey", "");
                        tc.action = trigger.value("action", "click");
                        lc.triggers.push_back(tc);
                    }

                    listeners.push_back(lc);
                }
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error reading config: " << e.what() << std::endl;
            listeners.clear(); // 解析失败时清空，防止使用未定义值
        }
    }

    // 新增 print() 方法，打印配置内容，包括 pauseKey
    void print() const
    {
        std::cout << "Loaded Config:\n";
        std::cout << "Pause Key: " << pauseKey << "\n";
        for (const auto &listener : listeners)
        {
            std::cout << "- Listening on: " << listener.listenKey << "\n";
            for (const auto &trigger : listener.triggers)
            {
                std::cout << "  - Triggers: " << trigger.triggerKey
                          << " (" << trigger.action << ")\n";
            }
        }
    }

private:
    std::string config_file;

    // 确保 config.json 存在，若不存在则创建一个默认配置
    void ensureConfigExists()
    {
        std::ifstream file_check(config_file);
        if (!file_check.good()) // 文件不存在时创建
        {
            std::ofstream file(config_file);
            if (file.is_open())
            {
                nlohmann::json j;
                j["pauseKey"] = "P";                   // 默认暂停键为 "P"
                j["listeners"] = nlohmann::json::array(); // 默认空数组
                file << j.dump(4);                        // 格式化 JSON 输出
                std::cout << "Created new config file: " << config_file << std::endl;
            }
        }
    }
};

#endif // CONFIG_HPP
