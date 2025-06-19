#!/bin/bash
#SBATCH -J sort_index_SLX23302
#SBATCH --cpus-per-task=16
#SBATCH --mem=32G
#SBATCH --time=24:00:00
#SBATCH --mail-user=lara.pohl@cruk.cam.ac.uk
#SBATCH --mail-type=START,FAIL,END
#SBATCH -p epyc
#SBATCH -e /Users/pohl01/logs/err/%x_%J.err
#SBATCH -o /Users/pohl01/logs/out/%x_%J.out

#samtools sort /mnt/scratchc/fmlab/kasper01/shared_data/lara_data/SLX-23302_merged_test.bam \
#    -o /mnt/scratchc/fmlab/pohl01/sprinter/chisel_prep/PEO1-FUCCI-SLX-23302/SLX-23302_merged_test_sorted.bam \
#    -@ 15

samtools index /mnt/scratchc/fmlab/pohl01/sprinter/chisel_prep/PEO1-FUCCI-SLX-23302/SLX-23302_merged_test_sorted.bam \
    -o /mnt/scratchc/fmlab/pohl01/sprinter/chisel_prep/PEO1-FUCCI-SLX-23302/SLX-23302_merged_test_sorted.bam.bai \
    -@ 15