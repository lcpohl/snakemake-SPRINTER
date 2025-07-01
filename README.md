# snakemake-SPRINTER
a snakemake workflow to run SPRINTER on single-cell whole genome sequencing data with one bam file per cell

## Workflow
1. use aligned bam files as input
2. generate a 16-character long random and unique barcode made up of A, C, T, and G for each cell
3. use compiled C++ script to merge bam files of individual cells into one bam file and change chromosome names to chr1, chr2 etc. to be CHISEL compatible
4. sort and index merged bam file with samtools
5. run chisel_rdr command to prepare SPRINTER input .tsv file 
6. gzip tsv file
7. run SPRINTER on the prepared .tsv.gz file

