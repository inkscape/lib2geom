#include <sys/time.h>
#include <iostream>
#include <vector>
#include <time.h>
#include <sched.h>

const long long US_PER_SECOND = 1000000L;
const long long NS_PER_US = 1000L;
using namespace std;

class Timer{
public:
  Timer() {}
  // note that CPU time is tracked per-thread, so the timer is only useful
  // in the thread it was start()ed from.
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
    clock_gettime(clock, &ts);
    us = ts.tv_sec * US_PER_SECOND + ts.tv_nsec / NS_PER_US;
  }
  /** Ask the OS nicely for a big time slice */
  void ask_for_timeslice() {
    sched_yield();
  }
private:
  long long start_time;
  struct timespec ts;
#ifdef _POSIX_THREAD_CPUTIME
  static const clockid_t clock = CLOCK_THREAD_CPUTIME_ID;
#else
# ifdef CLOCK_MONOTONIC
  static const clockid_t clock = CLOCK_MONOTONIC;
# else
  static const clockid_t clock = CLOCK_REALTIME;
# endif
#endif
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
