# snakemake-SPRINTER
a snakemake workflow to run SPRINTER on single-cell whole-genome DNA sequencing data with one non-barcoded bam file per cell

## Description
This snakemake workflow was written to prepare the necessary input for SPRINTER, a tool to identify replicating cells from single-cell whole-genome DNA sequencing data. From the DLP+ pipeline we receive a non-barcoded individual bam file per cell from the experiment. However, SPRINTER takes a single barcoded bam file including all cells as input. It is thus necessary to generate a barcode for each cell and merge the individual bam files into one file. This workflow takes the aligned bam files from the snakemake-illumina-alignment-scMulti pipeline as input. 

## Workflow
1. use aligned bam files as input
2. generate a 16-character long, random, and unique barcode made up of `[A, C, T, G]` for each cell
3. compile the C++ sript mark_and_merge_bams_samplesheet.cpp for merging the single bam files
4. use compiled C++ script to barcode and merge bam files of individual cells into one bam file and change chromosome names to chr1, chr2 etc. to be compatible with CHISEL
5. `sort` and `index` the merged bam file with samtools
6. run `chisel_rdr` command to calculate read depth ratio (RDR) values and prepare SPRINTER input .tsv file 
7. `gzip` the .tsv file and run SPRINTER on the prepared .tsv.gz file

## Usage
1. Prepare your input data using the snakemake-illumina-alignment-scMulti pipeline which outputs an aligned bam file for each cell in the experiment.
2. The makebarc.py script generates random and unique barcodes from `[A, C, G, T]` and a length currently set to 16 characters. This can be adjusted in the script. The output of the script is a .tsv file with the bam file and the matched newly generated barcode. 
3. The `mark_and_merge_bams_samplesheet` tool takes the single bams, the barcode sheet, and a reference sheet with the old and new chromosome names as input. If any changes are made to the C++ script, it needs to be recompiled using the compile_cpp rules in the assigned conda environment. Otherwise the precompiled tool can be used in the same conda environment specified by `envs/merge.yaml`. The tool adds the newly generated to the bam headers and changes the chromosome names according to the reference sheet (e.g. "1" -> "chr1"). 
