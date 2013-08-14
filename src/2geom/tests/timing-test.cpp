#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <time.h>
#include <sched.h>
#include <math.h>

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
    if(base_line > 1 and window > 100)
      return window;
    window *= 2;
  }
}

template <typename T>
string robust_timer(T &t) {
  static int  base_rate = estimate_useful_window();
  //cout << "base line iterations:" << base_rate << endl;
  double sum = 0;
  vector<double> results;
  const int n_trials = 20;
  results.reserve(n_trials);
  for(int trials = 0; trials < n_trials; trials++) {
    Timer tm;
    tm.ask_for_timeslice();
    tm.start();
    int iters = 0;
    while(tm.lap() < 10000) {
      for(int i = 0; i < base_rate; i++)
	t();
      iters+=base_rate;
    }
    base_rate = iters;
    double lap_time = double(tm.lap());
    double individual_time = lap_time/base_rate;
    sum += individual_time;
    results.push_back(individual_time);
    //cout << individual_time << endl;
  }
  double resS = 0;
  double resN = 0;
  sort(results.begin(), results.end());
  double ave = results[results.size()/2];//sum/n_trials; // median
  //cout << "median:" << ave << endl;
  double least = ave;
  double resSS = 0;
  for(int i = 0; i < n_trials; i++) {
    double dt = results[i];
    if(dt <= ave*1.1) {
      resS += dt;
      resN += 1;
      resSS += dt*dt;
      if(least < dt)
	least = dt;
    }
  }

  double filtered_ave = resS / resN;
  double stddev = sqrt((resSS - 2*resS*filtered_ave + resN*filtered_ave*filtered_ave)/(resN-1)); // sum(x-u)^2 = sum(x^2-2xu+u*u)
  assert (least > filtered_ave*0.7); // If this throws something was really screwy
  std::basic_stringstream<char> ss;
  ss << filtered_ave << " +/-" << stddev << "us";
  return ss.str();
}

struct nop{
  void operator()() const {}
};

#define degenerate_imported 1
#include "degenerate.cpp"
using namespace Geom;

template <typename T>
struct copy{
  T a, b;
  void operator()() {
    T c = a;
  }
};

template <typename T>
struct add{
  T a, b;
  void operator()() {
    T c = a + b;
  }
};

template <typename T>
struct add_mutate{
  T a, b;
  void operator()() {
    a += b;
  }
};

template <typename T>
struct scale{
  T a;
  double b;
  void operator()() {
    T c = a * b;
  }
};

template <typename T>
struct scale_mutate{
  T a;
  double b;
  void operator()() {
    a *= b;
  }
};

template <typename T>
struct mult{
  T a, b;
  void operator()() {
    T c = a * b;
  }
};

template <typename T>
struct mult_mutate{
  T a, b, c;
  void operator()() {
    c = a;
    c *= b;
  }
};

template <typename T>
void basic_arith(T const & a, T const & b) {
  {
    ::copy<T> A;
    A.a = a;
    A.b = b;
    cout << "copy:" 
	 << robust_timer(A) << endl;
  }
  {
    add<T> A;
    A.a = a;
    A.b = b;
    cout << "add:" 
	 << robust_timer(A) << endl;
  }
  {
    add_mutate<T> A;
    A.a = a;
    A.b = b;
    cout << "add_mutate:" 
	 << robust_timer(A) << endl;
  }
  {
    ::scale<T> A;
    A.a = a;
    A.b = 1;
    cout << "scale:" 
	 << robust_timer(A) << endl;
  }
  {
    scale_mutate<T> A;
    A.a = a;
    A.b = 1;
    cout << "scale_mutate:" 
	 << robust_timer(A) << endl;
  }
  {
    mult<T> A;
    A.a = a;
    A.b = b;
    cout << "mult:" 
	 << robust_timer(A) << endl;
  }
  {
    mult_mutate<T> A;
    A.a = a;
    A.b = b;
    cout << "mult_mutate:" 
	 << robust_timer(A) << endl;
  }
  
}

#include <valarray>
#include <2geom/orphan-code/sbasisN.h>
#include <2geom/piecewise.h>
int main(int /*argc*/, char** /*argv*/) {
  
  {
    nop N;
    cout << "nop:" << robust_timer(N) << endl;
  }

  vector<SBasis> sbs;
  valarray<double> va(4), vb(4);
  generate_random_sbasis(sbs);
  cout << "double\n";
  basic_arith(sbs[0][0][0], sbs[1][0][0]);
  cout << "valarray\n";
  basic_arith(va, vb);
  //cout << "Linear\n";
  //basic_arith(sbs[0][0], sbs[1][0]);
  cout << "SBasis\n";
  basic_arith(sbs[0], sbs[1]);
  cout << "pw<SBasis>\n";
  basic_arith(Piecewise<SBasis>(sbs[0]), Piecewise<SBasis>(sbs[1]));
  /*cout << "SBasisN<1>\n";
  SBasisN<1> sbnA = sbs[0];
  SBasisN<1> sbnB = sbs[0];
  basic_arith(sbnA, sbnB);*/
}
