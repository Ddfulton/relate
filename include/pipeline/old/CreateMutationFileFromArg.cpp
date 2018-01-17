#include <iomanip>
#include <sys/time.h>
#include <sys/resource.h>
#include <ctime>
#include <string>

#include "cxxopts.hpp"
#include "collapsed_matrix.hpp"
#include "data.hpp"
#include "arg.hpp"
#include "arg_builder.hpp"

int main(int argc, char* argv[]){

  //////////////////////////////////
  //Program options
  cxxopts::Options options("CreateMutationsFileFromArg");
  options.add_options()
    ("help", "Print help")
    ("s,seq", "Filename of sequences",  cxxopts::value<std::string>())
    ("p,pos", "Filename of pos",  cxxopts::value<std::string>())
    ("a,arg", "Filename of arg",  cxxopts::value<std::string>())
    ("o,output", "Filename of output without file extension", cxxopts::value<std::string>());

  options.parse(argc, argv);
  bool help = false;
  if(!options.count("seq") || !options.count("pos")  || !options.count("arg") || !options.count("output")){
    std::cout << "Not enough arguments supplied." << std::endl;
    help = true;
  }
  if(options.count("help") || help){
    std::cout << options.help({""}) << std::endl;
    exit(0);
  }

  std::cerr << "############" << std::endl;
  std::cerr << "Create mutation file from Arg..." << std::endl;

  Data data;
  data.ReadSequence(options["seq"].as<std::string>());
  data.ReadPosition(options["pos"].as<std::string>());

  //Read in arg
  Arg arg;
  arg.ReadArgLong(options["arg"].as<std::string>());

  ArgBuilder argbuilder(data);
  Leaves sequences_carrying_mutation;
  sequences_carrying_mutation.member.resize(data.N);

  std::vector<char> is_mapping(data.L, '1');

  //for every snp, find branch on which mutation maps.
  CorrTrees::iterator it_arg = arg.seq.begin();
  int count_tree = 0;
  for(int snp = 0; snp < data.L; snp++){

    if(it_arg != std::prev(arg.seq.end(),1)){
      if((*std::next(it_arg,1)).pos <= data.pos[snp]){
        it_arg++;
        count_tree++;
      }
    }

    sequences_carrying_mutation.num_leaves = 0; //this stores the number of nodes with a mutation at this snp.
    for(int i = 0; i < data.N; i++){
      if(data.sequence[snp][i] == '1'){
        sequences_carrying_mutation.member[i] = 1; //member stores a sequence of 0 and 1, where 1 at position i means that i carries a mutation.
        sequences_carrying_mutation.num_leaves++;
      }else{
        sequences_carrying_mutation.member[i] = 0;
      }
    }
    if(argbuilder.IsSNPMapping((*it_arg).tree, sequences_carrying_mutation, snp) > 1){
      is_mapping[snp] = '0';
    } 
    argbuilder.mutations.info[snp].tree = count_tree;

  }
  argbuilder.mutations.GetAge(arg);

  ///////////////////////////////////////// Combine Mutation Files /////////////////////////

  int i;
  std::string line2;
  std::ifstream is_prep("../data/prep.mut");
  if(is_prep.fail()){
    std::cerr << "Error while reading prep." << std::endl;
    exit(1);
  } 
  
  getline(is_prep,line2);
  std::ofstream os(options["output"].as<std::string>() + ".mut");
  if(os.fail()){
    std::cerr << "Error while opening file." << std::endl;
    exit(1);
  } 

  //first line
  i = 0;
  while(line2[i] != ';') i++;
  i++;
  while(line2[i] != ';') i++;
  i++;
  while(line2[i] != ';') i++;
  os << line2.substr(0,i) << ";";
  os << "tree_index;branch_indices;is_mapping;is_flipped;(age_begin,age_end);";
  os << line2.substr(i+1,line2.size()-1) << "\n";

  //read in end_chunk lines
  //need to record num_flips and num_non_mapping (3rd,4th entry)

  std::vector<char>::iterator it_is_mapping = is_mapping.begin();
  for(std::vector<SNPInfo>::iterator it = argbuilder.mutations.info.begin(); it != argbuilder.mutations.info.end(); it++){

    getline(is_prep, line2);

    i = 0;
    while(line2[i] != ';') i++;
    i++;
    while(line2[i] != ';') i++;
    i++;
    while(line2[i] != ';') i++;

    os << line2.substr(0,i) << ";";

    /////////

    os << (*it).tree << ";";
    std::deque<int>::iterator it_branch = (*it).branch.begin();
    if((*it).branch.size() > 0){
      os << *it_branch;
      it_branch++;
    }
    for(; it_branch != (*it).branch.end(); it_branch++){
      os << " " << *it_branch;
    }
    if(*it_is_mapping == '0'){
      os << ";1;"; 
    }else{
      os << ";0;";
    }
    os << (*it).flipped << ";(" << (*it).age_begin << "," << (*it).age_end << ");";

    /////////

    os << line2.substr(i+1,line2.size()-1) << "\n";
    it_is_mapping++;

  }



  os.close();

}
