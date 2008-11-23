#include <sys/time.h>
#include <iostream>
#include <vector>

const long long TICKS_PER_SECOND = 1000000L;
using namespace std;

class Timer{
public:
  struct timeval tv;
  struct timezone tz;
  long long start_time;
  Timer() {}
  void start() {
    usec(start_time);
  }
  void lap(long long &us) {
    usec(us);
    us -= start_time;
  }
  long long lap() {
    long long us;
    usec(us);
    return us - start_time;
  }
  void usec(long long &us) {
    ::gettimeofday(&tv, &tz);
    us = tv.tv_sec * TICKS_PER_SECOND + tv.tv_usec;
  }
  /** Ask the OS nicely for a big time slice */
  void ask_for_timeslice() {
    usleep(0);
  }
};

int estimate_useful_window() 
{
  Timer tm;
  tm.ask_for_timeslice();
  int window = 1;
  
  while(1) {
    tm.start();
    for(int i = 0; i < window; i++) {}
    long long  base_line = tm.lap();
    if(base_line > 1000)
      return window;
  }
}

template <typename T>
double robust_timer(T const &t) {
  int  base_rate = 100000;
  double best = 0, sum = 0;
  vector<double> results;
  const int n_trials = 5;
  results.reserve(n_trials);
  for(int trials = 0; trials < n_trials; trials++) {
    Timer tm;
    tm.ask_for_timeslice();
    tm.start();
    for(int i = 0; i < base_rate; i++)
      t();
    double lap_time = double(tm.lap());
    double individual_time = lap_time/base_rate;
    sum += individual_time;
    results.push_back(individual_time);
    cout << individual_time << endl;
  }
  double resS = 0;
  double resN = 0;
  double ave = sum/n_trials;
  //cout << ave << endl;
  for(int i = 0; i < n_trials; i++) {
    double dt = results[i];
    if(dt <= ave) {
      resS += dt;
      resN += 1;
    }
  }
  return resS / resN;
}

struct nop{
  void operator()() const {}
};

int main(int argc, char** argv) {

  cout << "nop:" << robust_timer(nop()) << "us" << endl;
}
