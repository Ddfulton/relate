// Output: .arg and .mut file 

#include <iostream>
#include <sys/time.h>
#include <sys/resource.h>

#include "cxxopts.hpp"
#include "data.hpp"
#include "arg.hpp"
#include "arg_builder.hpp"

int BuildSingleTree(cxxopts::Options& options){

  bool help = false;
  if(!options.count("effectiveN") || !options.count("mutation_rate") || !options.count("first_section") || !options.count("output")){
    std::cout << "Not enough arguments supplied." << std::endl;
    std::cout << "Needed: effectiveN, mutation_rate, first_section, output." << std::endl;
    help = true;
  }
  if(options.count("help") || help){
    std::cout << options.help({""}) << std::endl;
    std::cout << "Use to build single tree at a snp." << std::endl;
    exit(0);
  }

  std::cerr << "------------------------------------------------------" << std::endl;
  std::cerr << "Building tree..." << std::endl;

  //////////////////////////////////
  //Parse Data

  int Ne = (int) options["effectiveN"].as<float>();
  double mutation_rate = options["mutation_rate"].as<float>();
  Data data("sequences.bin", "pos.bin", "recombination_rate.bin", "rpos.bin", Ne, mutation_rate); //struct data is defined in data.hpp
  int num_windows = (int) ((data.L+data.window_length-1)/data.window_length);

  ///////////////////////////////////////////// Build Arg //////////////////////////
  //input:  Data and distance matrix
  //output: Arg (tree sequence)

  Arg arg;
  arg.seq.emplace_back();
  ArgBuilder argbuilder(data);
  argbuilder.BuildTreeAtSNP(options["first_section"].as<int>(), data, (*arg.seq.begin()).tree);
  argbuilder.mutations.GetAge(arg);

  /////////////////////////////////////////// Dump Arg to File //////////////////////

  (*arg.seq.begin()).tree.WriteNewick(options["output"].as<std::string>() + ".newick");
  arg.Dump(options["output"].as<std::string>() + ".arg");
  argbuilder.mutations.DumpShortFormat(options["output"].as<std::string>() + ".mut", (*arg.seq.begin()).tree.nodes[0].SNP_begin, (*arg.seq.begin()).tree.nodes[0].SNP_end);

  /////////////////////////////////////////////
  //Resource Usage

  rusage usage;
  getrusage(RUSAGE_SELF, &usage);

  std::cerr << "CPU Time spent: " << usage.ru_utime.tv_sec << "." << std::setfill('0') << std::setw(6) << usage.ru_utime.tv_usec << "s; Max Memory usage: " << usage.ru_maxrss/1000.0 << "Mb." << std::endl;
  std::cerr << "------------------------------------------------------" << std::endl << std::endl;

  return 0;
}
