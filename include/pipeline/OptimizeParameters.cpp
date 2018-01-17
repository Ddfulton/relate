/*! \file OptimizeParameters.cpp 
 *  \brief Optimize parameters for painting
 *
 *  Input: data
 *  Output: optimal parameters for painting (theta and recombination_factor)
 */

#include <iostream>
#include <sys/time.h>
#include <sys/resource.h>

#include "cxxopts.hpp"
#include "data.hpp"
#include "anc.hpp"
#include "fast_painting.hpp"
#include "anc_builder.hpp"

int main(int argc, char* argv[]){

  //////////////////////////////////
  //Program options  
  cxxopts::Options options("Optimize Parameters");
  options.add_options()
    ("help", "Print help")
    ("bed", "Filename of bed file.", cxxopts::value<std::string>())
    ("bim", "Filename of bim file.", cxxopts::value<std::string>())
    ("fam", "Filename of fam file.", cxxopts::value<std::string>())
    ("o,output", "Filename of output without file extension", cxxopts::value<std::string>());
  
  options.parse(argc, argv);
  bool help = false;
  if(!options.count("bed") || !options.count("bim") || !options.count("fam") || !options.count("output")){
    std::cout << "Not enough arguments supplied." << std::endl;
    help = true;
  }
  if(options.count("help") || help){
    std::cout << options.help({""}) << std::endl;
    std::cout << "Optional. This will generate a matrix to see what parameter combination (in painting) is optimal for tree topology." << std::endl;
    exit(0);
  }
  
  std::cerr << "############" << std::endl;
  std::cerr << "Optimizing Parameters..." << std::endl;

  //////////////////////////////////
  //Parse Data
  Data data(options["bed"].as<std::string>(), options["bim"].as<std::string>(), options["fam"].as<std::string>(), 0); //struct data is defined in data.hpp

  int section_startpos = 0;
  int section_endpos = data.L-1;

  std::vector<double> rec_rate = data.r;

  ///////////////////////////////////////////// Build AncesTree //////////////////////////
  //input:  Data and distance matrix
  //output: AncesTree (tree sequence)

  std::vector<float> theta = {1e-4, 1e-3, 5e-3, 7.5e-3, 1e-2, 2.5e-2, 5e-2, 7.5e-2, 1e-1, 2e-1};
  std::vector<float> rec_factor = {10, 100, 500, 1000, 2500, 5000, 7500, 10000, 15000, 20000, 50000};

  std::ofstream os(options["output"].as<std::string>());

  for(int theta_index = 0; theta_index < (int) theta.size(); theta_index++){
    for(int rec_index = 0; rec_index < (int) rec_factor.size(); rec_index++){

      float mean_rec = 0.0;
      data.theta     = theta[theta_index];
      data.ntheta    = 1.0 - data.theta;

      for(int l = 0; l < (int)data.r.size(); l++){
        data.r[l] = rec_rate[l] * rec_factor[rec_index];
        mean_rec  += data.r[l];
      }

      AncesTreeBuilder ancbuilder(data);
      std::pair<int, int> num_notmapping = ancbuilder.OptimizeParameters(section_startpos, section_endpos, data);
      os << data.theta << " " << rec_factor[rec_index] << " " << mean_rec/data.r.size() << " " << num_notmapping.first << " " << num_notmapping.second << std::endl;

    }
  }

  /////////////////////////////////////////////
  //Resource Usage

  rusage usage;
  getrusage(RUSAGE_SELF, &usage);

  std::cerr << "CPU Time spent: " << usage.ru_utime.tv_sec << "." << std::setfill('0') << std::setw(6);
#ifdef __APPLE__
  std::cerr << usage.ru_utime.tv_usec << "s; Max Memory usage: " << usage.ru_maxrss/1000000.0 << "Mb." << std::endl;
#else
  std::cerr << usage.ru_utime.tv_usec << "s; Max Memory usage: " << usage.ru_maxrss/1000.0 << "Mb." << std::endl;
#endif
  std::cerr << "---------------------------------------------------------" << std::endl << std::endl;

  return 0;
}
