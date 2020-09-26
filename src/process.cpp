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

//DONE: intialize a process
Process::Process(int id): pidid_(id) {
    prev_actjif_ = LinuxParser::ActiveJiffies(pidid_);
    prev_jif_ = LinuxParser::Jiffies();
}

// DONE: Return this process's ID
int Process::Pid() const { return pidid_; }

// DONE: Return this process's CPU utilization (tested with CPU stress test)
float Process::CpuUtilization() const {
  return cpu_util_;
}
// Update the process calculation
void Process::Update(){
    long actjif, jif;
    actjif = LinuxParser::ActiveJiffies(pidid_);
    jif = LinuxParser::Jiffies();
    cpu_util_ = 1.0 * (actjif - prev_actjif_) / (jif - prev_jif_);
    prev_actjif_ = actjif;
    prev_jif_ = jif;
}

// DONE: Return the command that generated this process
string Process::Command() const { return LinuxParser::Command(pidid_); }

// Done: Return this process's memory utilization
string Process::Ram() const{ return to_string(LinuxParser::Ram(pidid_));}

// DONE: Return the user (name) that generated this process
string Process::User() const { return LinuxParser::User(pidid_); }

// DONE: Return the age of this process (in seconds)
long int Process::UpTime() const { return LinuxParser::UpTime(pidid_); }

// DONE: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return this->CpuUtilization() < a.CpuUtilization();
}