#include <fstream>
#include <streambuf>
#include <iostream>

#include "log-file-manager.hh"

#define LOG_FILE_PATH "etc/server/log/%02d.txt"

LogFileManager::LogFileManager(const int& nodeId) {
  char logFile[24];
  std::sprintf(logFile, LOG_FILE_PATH, nodeId);

  m_logFilePath = std::string(logFile);
}

void
writeWithMode(const std::string& filePath,
              const std::string& str,
              const std::ios_base::openmode& mode) {
  std::ofstream ofs(filePath, mode);

  std::cout << str << std::endl;

  ofs << str << std::endl;

  ofs.close();
}

void
LogFileManager::append(const std::string& entry) {
  std::unique_lock<std::mutex> lock(m_mutex);

  writeWithMode(m_logFilePath, entry, std::ios_base::app);
}

void
LogFileManager::replace(const std::string& contents) {
  std::unique_lock<std::mutex> lock(m_mutex);
  writeWithMode(m_logFilePath, contents, std::ios_base::out);
}

void
LogFileManager::read(std::string& contents) {
  std::ifstream ifs(m_logFilePath);

  if (ifs.good()) {
    ifs.seekg(0, std::ios::end);
    std::cout << ifs.tellg() << std::endl;
    contents.reserve(ifs.tellg());
    ifs.seekg(0, std::ios::beg);

    contents.assign((std::istreambuf_iterator<char>(ifs)),
                    std::istreambuf_iterator<char>());
  }
}
