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

chr=${SGE_TASK_ID}
../bin/RelateCoalescentRate --mode CoalescentRateForSection --pos pos_chr${chr}_${pop}.txt --mut relate_chr${chr}_${pop}_pre.mut -i relate_chr${chr}_${pop} -o relate_${pop}_chr${chr} 

../bin/RelateMutationRate --mode WithContextForChromosome --samples ../1000GP_Phase3_${pop}.sample --mask ../../../genome_mask/PilotMask/20140520.chr${chr}.pilot_mask.fasta --ancestor ../../../human_ancestor_GRCh37_e59/human_ancestor_${chr}.fa --mut relate_chr${chr}_${pop}_pre.mut -i relate_chr${chr}_${pop} -o relate_${pop}_chr${chr} 

echo "***********************************************"
echo "Finished at: "`date`
echo "***********************************************"
exit 0

