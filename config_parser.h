#pragma once

#include <string>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>

struct Config {
    std::string xpm_path = "";
    int offset_x = 0;
    int offset_y = 0;
    std::string hotkey = "";
};

inline std::string getConfigDir() {
    const char* home = std::getenv("HOME");
    if (!home) return ".";
    return std::string(home) + "/.config/cppcrosshair";
}

inline Config loadConfig() {
    Config cfg;
    std::string dir = getConfigDir();
    std::string filepath = dir + "/config.ini";

    cfg.xpm_path = dir + "/crosshair.xpm";

    std::ifstream file(filepath);
    if (!file.is_open()) {
        return cfg;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Убираем пробелы
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        
        if (line.empty() || line[0] == '#' || line[0] == ';' || line[0] == '[') {
            continue;
        }

        size_t delimiter_pos = line.find('=');
        if (delimiter_pos != std::string::npos) {
            std::string key = line.substr(0, delimiter_pos);
            std::string value = line.substr(delimiter_pos + 1);

            if (key == "xpm_path") {
                cfg.xpm_path = value;
            } else if (key == "offset_x") {
                cfg.offset_x = std::stoi(value);
            } else if (key == "offset_y") {
                cfg.offset_y = std::stoi(value);
            } else if (key == "hotkey") {
                cfg.hotkey = value;
            }
        }
    }
    return cfg;
}

inline bool saveConfig(const Config& cfg) {
    std::string dir = getConfigDir();
    mkdir(dir.c_str(), 0755);

    std::string filepath = dir + "/config.ini";
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    file << "[Overlay]\n";
    file << "xpm_path = " << cfg.xpm_path << "\n";
    file << "offset_x = " << cfg.offset_x << "\n";
    file << "offset_y = " << cfg.offset_y << "\n";
    file << "hotkey = " << cfg.hotkey << "\n";
    
    return true;
}
