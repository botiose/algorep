#pragma once

#include <mutex>
#include <string>

class LogFileManager {
public:
  LogFileManager(const int& nodeId);

  void
  append(const std::string& entry);

  void
  replace(const std::string& contents);

  void
  read(std::string& contents);

private:
  std::string m_logFilePath;

  std::mutex m_mutex;
};
