#!/bin/bash
#SBATCH -J chisel_rdr_SLX-12
#SBATCH --cpus-per-task=16
#SBATCH --mem=256G
#SBATCH --time=120:00:00
#SBATCH --mail-user=lara.pohl@cruk.cam.ac.uk
#SBATCH --mail-type=START,FAIL,END
#SBATCH -p epyc
#SBATCH -e /Users/pohl01/logs/err/%x_%J.err
#SBATCH -o /Users/pohl01/logs/out/%x_%J.out

snakemake --cores 16 --snakefile workflow/Snakefile