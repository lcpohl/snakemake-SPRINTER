#!/bin/bash
#SBATCH -J run_sprinter_SLX23302merged
#SBATCH --cpus-per-task=16
#SBATCH --mem=64G
#SBATCH --time=20:00:00
#SBATCH --mail-user=lara.pohl@cruk.cam.ac.uk
#SBATCH --mail-type=START,FAIL,END
#SBATCH -p epyc
#SBATCH -e /Users/pohl01/logs/err/%x_%J.err
#SBATCH -o /Users/pohl01/logs/out/%x_%J.out

export BATCH="SLX-23302"
export INDIR="/mnt/scratchc/fmlab/pohl01/sprinter/chisel_input/PEO1-FUCCI-${BATCH}/merged_test/"
export OUTDIR="/mnt/scratchc/fmlab/pohl01/sprinter/sprinter_result/PEO1-FUCCI-${BATCH}/merged_test/"

cd ${INDIR}

gzip -c rdr/rdr.tsv > sprinter_${BATCH}.input.tsv.gz
rm -rf rdr/ baf/ combo/ calls/ clones/ plots/

cd ${OUTDIR}

sprinter ${INDIR}sprinter_${BATCH}.input.tsv.gz \
         --refgenome /mnt/scratchc/fmlab/pohl01/sprinter/refgenome_data/hsa.GRCh37_g1kp2.fa \
         --minreads 100000 \
         --rtreads 200 \
         --cnreads 1000 \
         --minnumcells 15 \
         -j 12