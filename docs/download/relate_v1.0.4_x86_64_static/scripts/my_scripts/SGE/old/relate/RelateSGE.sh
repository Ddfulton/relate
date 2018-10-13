#!/bin/bash

#Treat expanding unset parameters as error 
set -u
#Exit with status 1 if a command throws an error
set -e

if [ $# -le 4 ]
  then
    echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
    echo "Not enough arguments supplied. Execute as"
    echo "./RelateSGE [path_to_relate] [chr id] [mu] [Ne] [output filename without file extension]"
    exit 1;
fi

######################################################################################################

#### copy binaries to directory
PATH_TO_RELATE=$1
#mkdir -p bin
#cp ${PATH_TO_RELATE}/bin/* bin

#### chromosome index
c=$2

#### create dir for logfiles
#rm -rf log
mkdir -p log
mkdir -p log/log_chr${c}

#### create data file
mkdir -p data

#### delete existing res files
rm -rf results_c*
mkdir -p result

output=$5"_chr"$c

#### DO NOT CHANGE THE FOLLOWING PARAMETERS
window_length=400
batch_windows=10
chunk_size=200000

######################################################################################################

#### Divide data into chunks of 500k SNPs (the chunks are overlapping to avoid edge effects)
#-sync y causes qsub to wait for the job to complete before exiting. 
qsub -sync y \
     -hold_jid move_files_chr${c} \
     -N make_chunks_chr$c \
     -v chunk_size=${chunk_size} \
     -wd ${PWD}/data/ \
     -e ../log/make_chunks.log \
     -o ../log/make_chunks.log \
     relate/MakeChunks.sh

#### Set parameters
L=0
for num in $(head -1 "data/sequences.txt");
do
  N=$L
  L=$num 
done
mu=$3 #mutation rate
Ne=$4 #population size

#### DO NOT CHANGE THE FOLLOWING PARAMETERS
num_chunks=$(((${L}-1+${chunk_size})/${chunk_size}))
prev_chunk=-1

#### function to check if data exists
check_file_existence (){
  if [ ! -f $1 ]; then
      echo "File " + $1 + " not found!"
      exit 1
  fi
}


######################################################################################################
#### Build trees for each chunk

## paint all sequences against each other

for chunk in `seq 0 $(($num_chunks - 1))`;
do

  file_seq="data/sequences_${chunk}.txt"
  file_pos="data/pos_${chunk}.txt"
  file_rec="data/recombination_rate_${chunk}.txt"

  check_file_existence $file_seq
  check_file_existence $file_pos
  check_file_existence $file_rec

  parameters=($(head -1 $file_seq))
  N=${parameters[0]}
  L=${parameters[1]}
  num_windows=$((($L+$(($window_length-1)))/$window_length))
  num_batched_windows=$(($num_windows/$batch_windows + 1))

  echo "********************************"
  echo "Parameters passed to scripts:"
  echo "N="$N "L="$L "Ne="$Ne "mu="$mu
  echo "Number of windows: "$num_windows

  ## paint all sequences against each other
  #make sure that only 5 paintings exist at a time. Chunk 5 is painted only after chunk 0 is done etc.
  qsub -hold_jid remove_painting_chr${c}_$(($chunk - 5)) \
       -N paint_chr${c}_${chunk} \
       -v window_length=$window_length,chunk=$chunk \
       -e log/paint_c${chunk}.log \
       -o log/paint_c${chunk}.log \
       relate/Paint.sh

  ## build tree topologies
  qsub -hold_jid paint_chr${c}_${chunk} \
       -N build_topology_chr${c}_${chunk} \
       -wd ${PWD}/results_c${chunk} \
       -t 1-$num_batched_windows \
       -v chunk=$chunk,batch_windows=$batch_windows,output=${output} \
       -e \$TASK_ID_build_c${chunk}.log \
       -o \$TASK_ID_build_c${chunk}.log \
       relate/BuildTopology.sh 

  ## find equivalent branches in adjacent trees 
  qsub -hold_jid build_topology_chr${c}_${chunk} \
       -N find_equivalent_branches_chr${c}_${chunk} \
       -wd ${PWD}/results_c${chunk}/ \
       -v chunk=$chunk,output=${output} \
       -e ../log/find_equivalent_branches_c${chunk}.log \
       -o ../log/find_equivalent_branches_c${chunk}.log \
       relate/FindEquivalentBranches.sh

  ## infer branch lengths
  qsub -hold_jid find_equivalent_branches_chr${c}_${chunk} \
       -N infer_branch_lengths_chr${c}_${chunk} \
       -wd ${PWD}/results_c${chunk}/ \
       -t 1-$num_batched_windows \
       -v Ne=$Ne,mu=$mu,c=${c},chunk=$chunk,num_batched_windows=$num_batched_windows,batch_windows=$batch_windows,num_windows=$num_windows,output=${output} \
       -e \$TASK_ID_infer_branch_length_c${chunk}.log \
       -o \$TASK_ID_infer_branch_length_c${chunk}.log \
       relate/InferBranchLengths.sh

   ## combine args into one file
  qsub -hold_jid infer_branch_lengths_chr${c}_${chunk} \
       -N combine_args_chr${c} \
       -wd ${PWD}/results_c${chunk}/ \
       -v Ne=$Ne,chunk=${chunk},c=${c},num_windows=${num_windows},output=${output} \
       -e ../log/combine_args_c${chunk}.log \
       -o ../log/combine_args_c${chunk}.log \
       relate/CombineArgs.sh

  qsub -hold_jid infer_branch_lengths_chr${c}_${chunk} \
       -N remove_painting_chr${c}_${chunk} \
       -wd ${PWD}/results_c${chunk}/ \
       -e ../log/remove_painting_chr${c}_${chunk}.log \
       -o ../log/remove_painting_chr${c}_${chunk}.log \
       relate/remove_bins.sh

  prev_chunk=$chunk

done

parameters=($(head -1 "data/sequences.txt"))
N=${parameters[0]}
L=${parameters[1]}

#-sync y causes qsub to wait for the job to complete before exiting. 
#finalize results
qsub -sync y \
     -hold_jid combine_args_chr${c} \
     -N finalize_chr${c} \
     -v chunk_size=${chunk_size},output=${output} \
     -e log/combine_args.log \
     -o log/combine_args.log \
     relate/Finalize.sh

mv log/*.log log/log_chr${c}
for chunk in `seq 0 $(($num_chunks - 1))`;
do
  mv results_c${chunk}/*.log log/log_chr${c}
done
tar -cf log/log_chr${c}.tar log/log_chr${c}
rm -rf log/log_chr${c}
rm -rf delete_tmp
