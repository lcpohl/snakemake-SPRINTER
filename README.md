# snakemake-SPRINTER
a snakemake workflow to run SPRINTER on single-cell whole-genome DNA sequencing data with one non-barcoded bam file per cell

## Description
This snakemake workflow was written to prepare the necessary input for SPRINTER, a tool to identify replicating cells from single-cell whole-genome DNA sequencing data. From the DLP+ pipeline we receive a non-barcoded individual bam file per cell from the experiment. This is the correct input format for scAbsolute. However, SPRINTER takes a single barcoded bam file including all cells as input. It is thus necessary to generate a barcode for each cell and merge the individual bam files into one file. This workflow takes the aligned bam files from the snakemake-illumina-alignment-scMulti pipeline as input. 

## Workflow
1. use aligned bam files as input
2. generate a 16-character long, random, and unique barcode made up of `[A, C, T, G]` for each cell
3. compile the C++ sript mark_and_merge_bams_samplesheet.cpp for merging the single bam files
4. use compiled C++ script to barcode and merge bam files of individual cells into one bam file and change chromosome names to chr1, chr2 etc. to be compatible with CHISEL
5. `sort` and `index` the merged bam file with samtools
6. run `chisel_rdr` command to calculate read depth ratio (RDR) values and prepare SPRINTER input .tsv file 
7. `gzip` the .tsv file and run SPRINTER on the prepared .tsv.gz file

## Usage
1. Prepare your input data using the snakemake-illumina-alignment-scMulti pipeline which outputs an aligned bam file for each cell in the experiment. The bam files should be placed in `results/singlebams` in a separate directory using the sample name. 
2. The sample name (SLX) and path to the reference genome need to be adjusted in the configuration file `config/main.yaml`. The profile is currently set to run on the rocm partition of the cluster, this can be adjusted in `profiles/single_cell_WGS/config.yaml`.
3. The makebarc.py script generates random and unique barcodes from `[A, C, G, T]` and a length currently set to 16 characters. This can be adjusted in the script. The output of the script is a .tsv file with the bam file and the matched newly generated barcode. 
4. The `mark_and_merge_bams_samplesheet` tool takes the single bams, the barcode sheet, and a reference sheet with the old and new chromosome names as input. If any changes are made to the C++ script, it needs to be recompiled using the compile_cpp rules in the assigned conda environment. Otherwise the precompiled tool can be used in the same conda environment specified by `envs/merge.yaml`. The tool adds the newly generated to the bam headers and changes the chromosome names according to the reference sheet (e.g. "1" -> "chr1"). It then merges the bam files into one bam file. Depending on the size of the dataset this step is going to take a couple of hours.
5. The new merged bam file is then sorted and indexed with the respetive `samtools` functions.
6. The sorted and indexed bam file can then be used as input to the CHISEL command `chisel_rdr`. This command also takes the reference genome specified in the configuration file as input. `chisel_rdr` creates multiple empty directories. The relevant output for the pipeline is the `rdr/rdr.tsv` file. 
7. The `run_sprinter` rule then uses `gzip` on the rdr.tsv file and uses the .tsv.gz file as input for SPRINTER, alongside the reference genome. SPRINTER also has a couple of parameters that can be tuned to improve the prediction. `minreads` is the minimum number of total sequencing reads that a cell needs to have in order to enter the SPRINTER analysis (default: 100,000). `rtreads` is the target number of reads used for replication-timing analyses (default: 200). `cnreads` is the target number of reads used for the copy-number analyses (default: 1000), this can be adjusted to suit the size of the copy-number aberrations that you want to investigate. `minnumcells` defines the minimum expected number of cells to define clones (default: 15). Further parameters for tuning and more detailed information can be found on the [SPRINTER repository](https://github.com/zaccaria-lab/SPRINTER).
