#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
using std::string;
using std::to_string;
using std::vector;

// Untested: Return this process's ID
int Process::Pid() const { return pidid_; }

// Untested: Return this process's CPU utilization
float Process::CpuUtilization() const {
  long prev_actjif, prev_jif, actjif, jif;
  prev_actjif = LinuxParser::ActiveJiffies(pidid_);
  prev_jif = LinuxParser::Jiffies();
  usleep(50000);  // invertal for calculate the instantaneous cpu util
  actjif = LinuxParser::ActiveJiffies(pidid_);
  jif = LinuxParser::Jiffies();
  return 1.0 * (actjif - prev_actjif) / (jif - prev_jif);
}

// Untested: Return the command that generated this process
string Process::Command() const { return LinuxParser::Command(pidid_); }

// Untested: Return this process's memory utilization
string Process::Ram() const{ return to_string(LinuxParser::Ram(pidid_));}

// Untested: Return the user (name) that generated this process
string Process::User() const { return LinuxParser::User(pidid_); }

// Untested: Return the age of this process (in seconds)
long int Process::UpTime() const { return LinuxParser::UpTime(pidid_); }

// Untested: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return this->CpuUtilization() > a.CpuUtilization();
}