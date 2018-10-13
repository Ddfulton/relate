#!/bin/bash

#$ -V
#$ -j y
#$ -N convert_from_gp
#$ -P myers.prjc -q short.qc

echo "***********************************************"
echo "SGE job ID: "$JOB_ID
echo "SGE task ID: "$SGE_TASK_ID
echo "Run on host: "`hostname`
echo "Operating system: "`uname -s`
echo "Username: "`whoami`
echo "Started at: "`date`
echo "***********************************************"

mv * ../data/
rm -rf *

cp ../../1000GP_Phase3/1000GP_Phase3.sample .
cp ../../1000GP_Phase3/genetic_map_chr${c}_combined_b37.txt .
cp ../../1000GP_Phase3/ancestral_type$c.txt .
cp ../../human_ancestor_GRCh37_e59/human_ancestor_$c.fa .
cp ../../genome_mask/PilotMask/20140520.chr$c.pilot_mask.fasta .
gunzip -c ../../1000GP_Phase3/1000GP_Phase3_chr$c.hap.gz > 1000GP_Phase3_chr$c.hap
gunzip -c ../../1000GP_Phase3/1000GP_Phase3_chr$c.legend.gz > 1000GP_Phase3_chr$c.legend
cp ../data/excluded.sample .

../bin/./ConvertFromGP \
  -h 1000GP_Phase3_chr$c.hap \
  -l 1000GP_Phase3_chr$c.legend \
  -m genetic_map_chr${c}_combined_b37.txt \
  -f human_ancestor_$c.fa \
  -s 1000GP_Phase3.sample \
  -x excluded.sample \
  -a ancestral_type$c.txt \
  -c 20140520.chr$c.pilot_mask.fasta 2>> ../log/convert_from_gp.log 

rm 1000GP_Phase3_chr$c.hap
rm 1000GP_Phase3_chr$c.legend
rm genetic_map_chr${c}_combined_b37.txt
rm ancestral_type$c.txt
rm human_ancestor_$c.fa
rm 20140520.chr$c.pilot_mask.fasta

#store the data
mkdir ../data_chr${c}
gzip -c sequences.txt > ../data_chr${c}/sequences.txt.gz
gzip -c pos.txt > ../data_chr${c}/pos.txt.gz
gzip -c recombination_rate.txt > ../data_chr${c}/recombination_rate.txt.gz
gzip -c prep.mut > ../data_chr${c}/prep.mut.gz
gzip -c flipped_snps.txt > ../data_chr${c}/flipped_snps.txt.gz
tar -cf ../data_chr${c}.tar ../data_chr${c}
rm -rf ../data_chr${c}

echo "***********************************************"
echo "Finished at: "`date`
echo "***********************************************"
exit 0
