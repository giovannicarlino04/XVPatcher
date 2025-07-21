// log.cpp
#include "log.h"
#include <iostream>

std::ofstream logFile;  // Define the logFile object here

// Function to open the log file
bool OpenLogFile(const std::string& filename) {
    logFile.open(filename, std::ios::out | std::ios::trunc);  // Open in truncation mode
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file!" << std::endl;
        return false;
    }
    return true;
}

// Function to close the log file
void CloseLogFile() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

// Helper function to get the current date and time as a string
std::string GetCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_time_t);
    char buf[80];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &now_tm);
    return std::string(buf);
}
