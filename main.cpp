#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>

#if defined(_WIN32) || defined(_WIN64)
#define OS_WIN
#include <windows.h>
#else
#define OS_MAC
#endif

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
#ifdef OS_WIN
    SetConsoleOutputCP(CP_UTF8);
#endif

    fs::path folderPath;
    if (argc > 1) {
        folderPath = argv[1];
    } else {
        folderPath = fs::current_path();
    }
    if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
        std::cerr << "Invalid folder: " << folderPath.u8string() << std::endl;
        return 1;
    }
    std::stringstream result;
    for (auto &p: fs::recursive_directory_iterator(folderPath)) {
        if (p.is_regular_file()) {
            auto filePath = p.path();
            auto fileName = filePath.filename().string();
            std::ifstream ifs(filePath, std::ios::binary);
            if (!ifs.is_open()) {
                std::cerr << "Failed to open file: " << filePath.u8string() << std::endl;
                continue;
            }
            std::stringstream fileContent;
            fileContent << ifs.rdbuf();
            result << fileName << ":\n";
            result << "```\n";
            result << fileContent.str();
            result << "\n```\n\n";
        }
    }
    std::string finalText = result.str();
#ifdef OS_WIN
    {
        FILE *pipe = _popen("clip", "w");
        if (!pipe) {
            std::cerr << "Failed to run 'clip' command" << std::endl;
            return 1;
        }
        fputs(finalText.c_str(), pipe);
        _pclose(pipe);
    }
#elif defined(OS_MAC)
    {
        FILE* pipe = popen("pbcopy", "w");
        if (!pipe) {
            std::cerr << "Failed to run 'pbcopy' command" << std::endl;
            return 1;
        }
        fputs(finalText.c_str(), pipe);
        pclose(pipe);
    }
#endif
    std::cout << "Copied to clipboard." << std::endl;
    return 0;
}
