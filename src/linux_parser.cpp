#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

#define MB_TO_KB 1024;
namespace {
// used internally for LinuxParser, extract the accumulated cpu util info, used
// for LinuxParser::CpuUtilization()
void CpuInfo(long& total_idle_, long& total_nonidle_) {
  std::ifstream stream(LinuxParser::kProcDirectory +
                       LinuxParser::kStatFilename);
  std::string line;
  std::string key;
  long user_, nice_, system_, idle_, iowaite_, irq_, softirq_, steal_, guest_,
      guest_nice_;

  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      // if (key.size() < 3) continue;
      // if (key[0] == 'c' && key[1] == 'p') {
      if (key == "cpu") {
        linestream >> user_ >> nice_ >> system_ >> idle_ >> iowaite_ >> irq_ >>
            softirq_ >> steal_ >> guest_ >> guest_nice_;
        total_idle_ = idle_ + iowaite_;
        total_nonidle_ = user_ + nice_ + system_ + irq_ + softirq_ + steal_;
      }
    }
  }
}
}  // namespace
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
  string os, kernel, verison;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel >> verison;
  }
  return kernel + " : " + verison;
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

// DONE: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  // https://github.com/hishamhm/htop/blob/8af4d9f453ffa2209e486418811f7652822951c6/linux/LinuxProcessList.c#L802-L833
  // https://github.com/hishamhm/htop/blob/1f3d85b6174f690a7e354bbadac19404d5e75e78/linux/Platform.c#L198-L208
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

// DONE: Read and return the system uptime
long LinuxParser::UpTime() {
  float uptime, idletime;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  std::string line;
  std::getline(stream, line);
  std::istringstream linestream(line);
  linestream >> uptime >> idletime;
  return (long)uptime;
}

// DONE: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  // https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
  // http://www.linuxhowtos.org/System/procstat.htm
  std::ifstream stream(kProcDirectory + kStatFilename);
  std::string line;
  std::string key;
  long user_, nice_, system_, idle_, iowaite_, irq_, softirq_, steal_, guest_,
      guest_nice_;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "cpu") {
        linestream >> user_ >> nice_ >> system_ >> idle_ >> iowaite_ >> irq_ >>
            softirq_ >> steal_ >> guest_ >> guest_nice_;
        long total_idle_ = idle_ + iowaite_;
        long total_nonidle_ =
            user_ + nice_ + system_ + irq_ + softirq_ +
            steal_;  // user_ and nice_ has accouneted the guest time
        return (total_idle_ + total_nonidle_);
      }
    }
  }
  return 0;  // return 0 if an error (for debug)
}

// DONE: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  // https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  std::string line, strunused_;
  long intunused_, utime_, stime_, cutime_, cstime_;
  std::getline(stream, line);
  std::istringstream linestream(line);
  linestream >> intunused_ >> strunused_ >> strunused_;
  for (int i = 4; i < 18; i++) {
    switch (i) {
      case 14: {
        linestream >> utime_;  // #14 utime - CPU time spent in user code,
                               // measured in clock ticks （jiffies)
        break;
      }
      case 15: {
        linestream >> stime_;  // #15 stime - CPU time spent in kernel code,
                               // measured in clock ticks （jiffies)
        break;
      }
      case 16: {
        linestream >>
            cutime_;  // #16 cutime - Waited-for children's CPU time spent in
                      // user code (in clock ticks) （jiffies)
        break;
      }
      case 17: {
        linestream >>
            cstime_;  // #17 cstime - Waited-for children's CPU time spent in
                      // kernel code (in clock ticks) （jiffies)
        break;
      }
      default:
        linestream >> intunused_;
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
      if (key == "cpu") {
        linestream >> user_ >> nice_ >> system_ >> idle_ >> iowaite_ >> irq_ >>
            softirq_ >> steal_ >> guest_ >> guest_nice_;
        // long total_idle_ = idle_ + iowaite_;
        long total_nonidle_ =
            user_ + nice_ + system_ + irq_ + softirq_ + steal_;
        return total_nonidle_;
      }
    }
  }
  return 0;  // for debug
}

// DONE: Read and return the number of idle jiffies for the system
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
      if (key == "cpu") {
        linestream >> user_ >> nice_ >> system_ >> idle_ >> iowaite_ >> irq_ >>
            softirq_ >> steal_ >> guest_ >> guest_nice_;
        long total_idle_ = idle_ + iowaite_;
        // long total_nonidle_ =
        //    user_ + nice_ + system_ + irq_ + softirq_ + steal_;
        return total_idle_;
      }
    }
  }
  return 0;
}

// DONE: Read and return CPU utilization
float LinuxParser::CpuUtilization() {
  // https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
  long prev_idle_, prev_nonidle_;
  long idle_, nonidle_;
  ::CpuInfo(prev_idle_, prev_nonidle_);
  usleep(
      50000);  // create time invertal for calculate the instantaneous cpu util
  ::CpuInfo(idle_, nonidle_);
  //std::cout << prev_nonidle_ << '\n';
  //std::cout << "idle: " << idle_ << ", nonidle: " << nonidle_ << '\n';
  float cpu_util = 1.0 * (nonidle_ - prev_nonidle_) /
                   (nonidle_ - prev_nonidle_ + idle_ - prev_idle_);
  return cpu_util;
}

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  std::string line;
  std::string key;
  long value;
  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    linestream >> key >> value;
    if (key == "processes") {
      return (int)value;
    }
  }
  return 0;
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  std::string line;
  std::string key;
  int value;
  while (std::getline(stream, line)) {
    std::istringstream linestream(line);
    linestream >> key >> value;
    if (key == "procs_running") {
      return value;
    }
  }
  return 0;
}

// DONE: Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  string line;
  std::getline(stream, line);
  return line;
  // return string();
}

// DONE: Read and return the memory used by a process
long LinuxParser::Ram(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  std::string line;
  std::string key;
  long value;
  while (std::getline(stream, line)) {
    std::replace(line.begin(), line.end(), ':', ' ');
    std::istringstream linestream(line);
    linestream >> key >> value;
    if (key == "VmSize") return value / MB_TO_KB;
  }
  return 0;
}

// Untested: Read and return the user ID associated with a process
int LinuxParser::Uid(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  std::string line;
  std::string key;
  long value;
  while (std::getline(stream, line)) {
    std::replace(line.begin(), line.end(), ':', ' ');
    std::istringstream linestream(line);
    linestream >> key >> value;
    if (key == "Uid") return value;
  }
  return 0;
}

// DONE: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  int uid_;
  uid_ = LinuxParser::Uid(pid);
  std::ifstream stream(kPasswordPath);
  std::string line;
  std::string key, unused_;
  long value;
  while (std::getline(stream, line)) {
    std::replace(line.begin(), line.end(), ':', ' ');
    std::istringstream linestream(line);
    linestream >> key >> unused_ >> value >> value;
    // std::cout<<"\n (key: "<< key << ", value:" << value << ") \n"; // For test
    if (value == uid_) return key;
  }
  return string("failure for LinuxParser::User");
}

// DONE: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  std::string line, strunused_;
  long intunused_, uptime_;
  std::getline(stream, line);
  std::istringstream linestream(line);
  linestream >> intunused_ >> strunused_ >> strunused_;
  for (int i = 4; i < 22; i++) linestream >> intunused_;
  linestream >> uptime_;
  return LinuxParser::UpTime() - uptime_ / sysconf(_SC_CLK_TCK);
}
