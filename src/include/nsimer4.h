
//NSIMer 4
//Simulation of NSIM model on CPU/GPU.
//Generation of GPU code is automatic based on update function defn, but
//initially just do on GPU?

#pragma once

//"natural" time is in milliseconds?

#include <sys/types.h>
#include <vector>
#include <cstdlib>
#include <cstdio>

typedef double float64_t;
//typedef long int int64_t; //defined in sys/types

typedef std::vector<float64_t> realvect;
typedef std::vector<int64_t> intvect;
typedef std::vector<std::string> stringvect;

template <typename T>
struct valcoll
{
  stringvect names;
  std::vector< T > vals;
};



//Instance of nsim4...?
struct nsim4circuit
{
  //All things naturally have access to these?
  float64_t dt=1e-1; //dt of each timestep.
  unsigned long long int t_now; //#timesteps since start.

  
  std::vector< realvect > realvars;
  std::vector< intvect > intvars;
};
