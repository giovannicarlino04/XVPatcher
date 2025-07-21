// log.h
#ifndef LOG_H
#define LOG_H

#include <fstream>
#include <string>
#include <chrono>
#include <ctime>

// Declare the logFile object here, so it's shared across files
extern std::ofstream logFile;  

// Function to open the log file (can be called from main.cpp)
bool OpenLogFile(const std::string& filename);

// Function to close the log file (can be called from main.cpp)
void CloseLogFile();

// Helper function to get the current date and time as a string
std::string GetCurrentDateTime();

#endif // LOG_H
