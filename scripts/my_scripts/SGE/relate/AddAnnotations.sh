#!/bin/bash

#$ -V
#$ -j y
#$ -P myers.prjc -q short.qc

echo "***********************************************"
echo "SGE job ID: "$JOB_ID
echo "SGE task ID: "$SGE_TASK_ID
echo "Run on host: "`hostname`
echo "Operating system: "`uname -s`
echo "Username: "`whoami`
echo "Started at: "`date`
echo "***********************************************"

c=$((${SGE_TASK_ID}))

cp ../human_ancestor_GRCh37_e59/human_ancestor_${c}.fa data/chr_${c}/
#gunzip -c data/chr_${c}/chr${c}.haps.gz > data/chr_${c}/chr${c}.haps
gunzip -c result/relate_chr${c}.mut.gz > result/relate_chr${c}.mut  

bin/RelateFileFormats \
  --mode GenerateSNPAnnotations \
  --haps data/chr_${c}/chr${c}.haps \
  --sample data/chr_${c}/chr${c}.sample \
  --ancestor data/chr_${c}/human_ancestor_${c}.fa \
  --poplabels data/1000GP_Phase3_sub.sample \
  --mut result/relate_chr${c}.mut \
  -o result/relate_chr${c}_annot 

#rm data/chr_${c}/chr${c}.haps
rm data/chr_${c}/human_ancestor_${c}.fa

echo "***********************************************"
echo "Finished at: "`date`
echo "***********************************************"
exit 0

