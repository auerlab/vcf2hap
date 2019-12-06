#!/usr/bin/env python2.7

# This script is the model on which vcf2hap is designed

#usage python format_hap.py input.vcf output.hap sample_id
# this script takes a phased single sample vcf and output a file with two rows of haplotypes

import sys
import re

# haplotype lists
hap1=[sys.argv[3] + " HAPLO1 "]
hap2=[sys.argv[3] + " HAPLO2 "]

# read vcf file
with open(sys.argv[1]) as f:
        for line in f:
                cols = line.strip().split("\t")
                
                # skip header
                if not re.match("#", cols[0]):
                        # expected columns
                        #ref = cols[3]
                        #alt = cols[4]
                        #gt = cols[9]
                        
                        # skip indels
                        if len(cols[3]) != 1 or len(cols[4]) != 1:
                                continue
                        elif  cols[9] == ".|.":
                                continue
                        elif cols[9] == "0|0":
                                hap1.append(cols[3])
                                hap2.append(cols[3])
                        elif cols[9] == "0|1":
                                hap1.append(cols[3])
                                hap2.append(cols[4])
                        elif cols[9] == "1|0":
                                hap1.append(cols[4])
                                hap2.append(cols[3])
                        elif cols[9] == "1|1":
                                hap1.append(cols[4])
                                hap2.append(cols[4])
                        else:
                                raise ValueError("unknown genotype = " + cols[9])
f.close()

# write haplotype strings to file
with open(sys.argv[2], "w") as o:
        o.write("".join(hap1) + "\n")
        o.write("".join(hap2) + "\n")
o.close()
