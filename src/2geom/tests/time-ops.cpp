#include <sys/time.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <assert.h>
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
    if(base_line > 10000 and window > 10)
      return window/10;
    window *= 2;
  }
}

template <typename T>
double robust_timer(T const &t) {
  static int  base_rate = estimate_useful_window();
  cout << "base line iterations:" << base_rate << endl;
  double best = 0, sum = 0;
  vector<double> results;
  const int n_trials = 20;
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
  sort(results.begin(), results.end());
  double ave = results[results.size()/2];//sum/n_trials; // median
  cout << "median:" << ave << endl;
  double least = ave;
  for(int i = 0; i < n_trials; i++) {
    double dt = results[i];
    if(dt <= ave*1.1) {
      resS += dt;
      resN += 1;
      if(least < dt)
	least = dt;
    }
  }

  double filtered_ave = resS / resN;
  assert (least > filtered_ave*0.7); // If this throws something was really screwy
  return filtered_ave;
}

struct nop{
  void operator()() const {}
};

#define degenerate_imported 1
#include "degenerate.cpp"
using namespace Geom;

struct add{
  SBasis a, b;
  void operator()() const {
    SBasis c = a + b;
  }
};

int main(int /*argc*/, char** /*argv*/) {
  
  cout << "nop:" << robust_timer(nop()) << "us" << endl;
  
  vector<SBasis> sbs;
  generate_random_sbasis(sbs);
  add A;
  A.a = sbs[0];
  A.b = sbs[1];
  cout << "add:" 
       << A.a.size() << "," 
       << A.b.size() << "; " 
       << robust_timer(A) << "us" << endl;
}
