//Make overlapping chunks with fixed number of SNPs from data set

#include <iostream>
#include <sys/time.h>
#include <sys/resource.h>

#include "cxxopts.hpp"
#include "data.hpp"

int MakeChunks(cxxopts::Options& options, int chunk_size = 0){

  bool help = false;
  if(!options.count("haps") || !options.count("sample") || !options.count("map")){
    std::cout << "Not enough arguments supplied." << std::endl;
    std::cout << "Needed: haps, sample, map. Optional: memory, dist." << std::endl;
    help = true;
  }
  if(options.count("help") || help){
    std::cout << options.help({""}) << std::endl;
    std::cout << "Use to make smaller chunks from the data." << std::endl;
    exit(0);
  }

  std::cerr << "---------------------------------------------------------" << std::endl;
  std::cerr << "Parsing data.." << std::endl;

  //////////////////////////////////
  //Parse Data

  Data data;
  /*
  if(options.count("chunk_size")){

    if(options.count("dist")){
      data.MakeChunks(options["haps"].as<std::string>(), options["sample"].as<std::string>(), options["map"].as<std::string>(), options["dist"].as<std::string>(), options["chunk_size"].as<int>());
    }else{
      data.MakeChunks(options["haps"].as<std::string>(), options["sample"].as<std::string>(), options["map"].as<std::string>(), "unspecified", options["chunk_size"].as<int>());
    }

  }else{

  }
  */

  if(options.count("memory")){
    if(options.count("dist")){
      data.MakeChunks(options["haps"].as<std::string>(), options["sample"].as<std::string>(), options["map"].as<std::string>(), options["dist"].as<std::string>(), options["memory"].as<float>());
    }else{
      data.MakeChunks(options["haps"].as<std::string>(), options["sample"].as<std::string>(), options["map"].as<std::string>(), "unspecified", options["memory"].as<float>());
    }
  }else{
    if(options.count("dist")){
      data.MakeChunks(options["haps"].as<std::string>(), options["sample"].as<std::string>(), options["map"].as<std::string>(), options["dist"].as<std::string>());
    }else{
      data.MakeChunks(options["haps"].as<std::string>(), options["sample"].as<std::string>(), options["map"].as<std::string>(), "unspecified");
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

