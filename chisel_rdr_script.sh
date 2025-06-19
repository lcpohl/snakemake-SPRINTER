#!/bin/bash
#SBATCH -J chisel_rdr_SLX23302
#SBATCH --cpus-per-task=16
#SBATCH --mem=32G
#SBATCH --time=20:00:00
#SBATCH --mail-user=lara.pohl@cruk.cam.ac.uk
#SBATCH --mail-type=START,FAIL,END
#SBATCH -p epyc
#SBATCH -e /Users/pohl01/logs/err/%x_%J.err
#SBATCH -o /Users/pohl01/logs/out/%x_%J.out

cd /mnt/scratchc/fmlab/pohl01/sprinter/chisel_input/PEO1-FUCCI-SLX-23302/merged_test/

export TUM="/mnt/scratchc/fmlab/pohl01/sprinter/chisel_prep/PEO1-FUCCI-SLX-23302/SLX-23302_merged_test_sorted.bam"

export REF="/mnt/scratchc/fmlab/pohl01/sprinter/refgenome_data/hsa.GRCh37_g1kp2.fa"

chisel_rdr -t ${TUM} -r ${REF} -b 50000 -m 100000 -j 15
           