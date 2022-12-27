#include <iostream>
#include <fstream>
#include <dirent.h>
#include <chrono>
#include <ctime>
#include <cstring>

#include "logger.h"


// Constructor, which init fields with data from config 
void Logger::Init(ConfigValues& values) {
    firstDirPath = values.strData.front();
    values.strData.pop();
    secondDirPath = values.strData.front();
    values.strData.pop();
    refreshDuration = values.uintData.front();
    values.uintData.pop();
}


// Method, which provides executing directory 
bool Logger::GetAllDirsAndFiles(const std::string& directoryPath, std::queue<std::string>& dirsAndFiles) {
    std::string CUR_DIR = ".", PARENT_DIR = "..";

    // get files and dirs
    DIR *dir;
    struct dirent *ent;
    dirsAndFiles = std::queue<std::string>();

    if ((dir = opendir(firstDirPath.c_str())) == NULL) 
        return false;

    while ((ent = readdir(dir)) != NULL)
        if ((strcmp(ent->d_name, CUR_DIR.c_str()) != 0) && (strcmp(ent->d_name, PARENT_DIR.c_str()) != 0))
            dirsAndFiles.push(ent->d_name);

    closedir(dir);
    return true;    
}


// Method, which generates log string
std::string Logger::GenerateLogString(std::queue<std::string>& dirsAndFiles) {
    std::string outputStr;

    if (dirsAndFiles.size() == 0)
        outputStr = "No files and directories in '" + firstDirPath + "'";
    else {
        outputStr = "";
        bool isFirstFile = true;
        while (dirsAndFiles.empty() == false) {
            if (!isFirstFile)
                outputStr += " ";
            else
                isFirstFile = false;
            outputStr += "'" + dirsAndFiles.front() + "'";
            dirsAndFiles.pop();
        }
    }

    return outputStr;
}

// Method, which provides logging
bool Logger::Log(void) {
    // create/open log filestream
    std::ofstream logFile(secondDirPath + "/" + LOGGER_FILE_NAME, std::ios_base::app);
        
    if (!logFile.is_open())
        return false;

    // get files and dirs
    std::queue<std::string> dirsAndFiles;
    std::string outputStr;

    if (GetAllDirsAndFiles(firstDirPath, dirsAndFiles) == true) {
        // print date and time
        auto current = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(current);
        outputStr = GenerateLogString(dirsAndFiles);

        logFile << std::endl << std::ctime(&currentTime);
        logFile << outputStr << std::endl;

        // close file stream and leave
        logFile.close();
        return true;
    }
    else {
        logFile.close();
        return false;
    }
}
