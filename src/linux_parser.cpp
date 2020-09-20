#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// UNTESTED: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  long memtotal, memfree;
  string line;
  string key;
  long value;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (!key.empty() && value) {
        if (key == "MemTotal")
          memtotal = value;
        else if (key == "MemFree")
          memfree = value;
      }
    }
  }
  return 1.0 - 1.0 * memfree / memtotal;
}

// UNTESTED: Read and return the system uptime
long LinuxParser::UpTime() {
  float uptime, idletime;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  std::string line;
  std::getline(stream, line);
  std::istringstream linestream(line);
  linestream >> uptime >> idletime;
  return (long)uptime;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  // std::ifstream stream(kProcDirectory + kStatFilename);
  // float

  return 0;
}

// Untested: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid ) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  std::string line, strunused_;
  long intunused_, utime_, stime_, cutime_, cstime_;
  std::getline(stream, line);
  std::istringstream linestream(line);
  line >> intunused >> strunused_ >> strunused_;
  for (int i = 4; i < 18; i++) {
    switch (i) {
      case 14: {
        line >> utime_;
        break;
      }
      case 15: {
        line >> stime_;
        break;
      }
      case 16: {
        line >> cutime_;
        break;
      }
      case 17: {
        line >> cstime_;
        break;
      }
      default:
        line >> intunused_;
    }
  }
  return utime_ + stime_ + cutime_ + cstime_;
}

// Untested: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  std::string line;
  std::string key;
  long user_, nice_, system_, idle_, iowaite_, irq_, softirq_, steal_, guest_,
      guest_nice_;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key="cpu") {
        linestream >> user_ >> nice_ >> system_ >> idle_ >> iowaite_ >> irq_ >>
            softirq_ >> steal_ >> guest_ >> guest_nice_;
        //long total_idle_ = idle_ + iowaite_;
        long total_nonidle_ =
            user_ + nice_ + system_ + irq_ + softirq_ + steal_;
        return total_nonidle_;
      }
    }
  }
  return 0;
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
    std::ifstream stream(kProcDirectory + kStatFilename);
  std::string line;
  std::string key;
  long user_, nice_, system_, idle_, iowaite_, irq_, softirq_, steal_, guest_,
      guest_nice_;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key="cpu") {
        linestream >> user_ >> nice_ >> system_ >> idle_ >> iowaite_ >> irq_ >>
            softirq_ >> steal_ >> guest_ >> guest_nice_;
        long total_idle_ = idle_ + iowaite_;
        //long total_nonidle_ =
        //    user_ + nice_ + system_ + irq_ + softirq_ + steal_;
        return total_idle_;
      }
    }
  }
  return 0;
 }

// UNTested: Read and return CPU utilization
vector<float> LinuxParser::CpuUtilization() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  std::string line;
  std::string key;
  long user_, nice_, system_, idle_, iowaite_, irq_, softirq_, steal_, guest_,
      guest_nice_;
  vector<float> cpu_util;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key.size() < 3) continue;
      if (key[0] == 'c' && key[1] == 'p') {
        linestream >> user_ >> nice_ >> system_ >> idle_ >> iowaite_ >> irq_ >>
            softirq_ >> steal_ >> guest_ >> guest_nice_;
        long total_idle_ = idle_ + iowaite_;
        long total_nonidle_ =
            user_ + nice_ + system_ + irq_ + softirq_ + steal_;
        cpu_util.emplace_back(1.0 * total_nonidle_ /
                              (total_idle_ + total_nonidle_));
      }
    }
  }
  return cpu_util;
}

// UNTESTED: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  std::string line;
  std::string key;
  long value, totalprocesses;
  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    linestream >> key >> value;
    if (key == "processes") {
      totalprocesses = value;
      break;
    }
  }
  return totalprocesses;
}

// UNTESTED: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  std::string line;
  std::string key;
  long value, procs_running;
  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    linestream >> key >> value;
    if (key == "procs_running") {
      procs_running = value;
      break;
    }
  }
  return procs_running;
}

// Untested: Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  string line;
  std::getline(stream, line);
  return line;
}

// Untested: Read and return the memory used by a process
long LinuxParser::Ram(int pid [[maybe_unused]]) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  std::string line;
  std::string key;
  long value;
  while (std::getline(stream, line)) {
    std::replace(line.begin(), line.end(), ':', ' ');
    std::istringstream linestream(line);
    linestream >> key >> value;
    if (key == "VmSize"): return value;
  }
  return 0;
}

// Untested: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  std::string line;
  std::string key;
  long value;
  while (std::getline(stream, line)) {
    std::replace(line.begin(), line.end(), ':', ' ');
    std::istringstream linestream(line);
    linestream >> key >> value;
    if (key == "Uid"): return std::to_string(value);
  }
  return string("failure for Uid");
}

// Untested: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  std::string string uid_;
  uid_ = LinuxParser::Uid(pid);
  std::ifstream stream(kPasswordPath);
  std::string line;
  std::string key, unused_;
  long value;
  while (std::getline(stream, line)) {
    if (str.find('|') != std::string::npos)
      std::replace(line.begin(), line.end(), ':', ' ');
    std::istringstream linestream(line);
    linestream >> key >> unused_ >> value >> value;
    if (key == uid_) return std::to_string(value);
  }
  return string("failure for user");
}

// Untested: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  std::string line, strunused_;
  long intunused_, uptime_;
  std::getline(stream, line);
  std::istringstream linestream(line);
  line >> intunused >> strunused_ >> strunused_;
  for (int i = 4; i < 22; i++) line >> intunused_;
  line >> uptime_;
  return uptime_ / sysconf(_SC_CLK_TCK);
}
