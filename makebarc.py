import random
from itertools import islice
from more_itertools import unique_everseen
import functools
from pathlib import Path
import csv
import argparse

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("-i", "--input", help="path to input directory", required=True)
    parser.add_argument("-o", "--output", help="path to output file", required=True)
    args = parser.parse_args()

# still needs fixing on the arguments part
bamfiles = list(Path(args.input).glob('*.bam'))

def make_barcodes(bamfiles, length):
    num_barcodes = len(bamfiles)
    pickchar = functools.partial(random.choice, ['A', 'C', 'G', 'T'])
    def gen_barcodes():
        while True:
            yield ''.join([pickchar() for _ in range(length)])
    return list(islice(unique_everseen(gen_barcodes()), num_barcodes))

barcodes = make_barcodes(bamfiles, 16)
header = ['bamfile', 'barcode']

with open(args.output, 'w', newline='') as tsvfile:
    writer = csv.writer(tsvfile, delimiter='\t', lineterminator='\n')
    writer.writerow(header)
    for bamfile, barcode in zip(bamfiles, barcodes):
        writer.writerow([bamfile, barcode])