/**
 * @file   log-file-manager.hh
 * @author Otiose email
 * @date   Thu Oct 28 09:05:39 2021
 * 
 * @brief  Defines the LogFileManager class.
 * 
 * This class ecapsulates all operations having to do with the log file of the 
 * given node.
 * 
 */
#pragma once

#include <mutex>
#include <string>

class LogFileManager {
public:
  /** 
   * @brief LogFileManager constructor.
   * 
   * @param[in] nodeId node id of the current node
   */
  LogFileManager(const int& nodeId);

  /** 
   * @brief Appends a line with the given string to the log file.
   * 
   * @param[in] entry string to append
   */
  void
  append(const std::string& entry);

  /** 
   * @brief Replaces the contents of the log file with the given string.
   * 
   * @param[in] contents new contents of the log file
   */
  void
  replace(const std::string& contents);

  /** 
   * @brief Reads the log file and stores the contents in the given string.
   * 
   * @param[out] contents contents of the log file
   */
  void
  read(std::string& contents);

private:
  std::string m_logFilePath;

  std::mutex m_mutex;
};
