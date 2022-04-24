#!/bin/sh -e

##########################################################################
#   This test utilizes randomly generated VCF data and haplotype output
#   generated at a time when vcf2hap was known to be working properly.
##########################################################################

cd ..
./cave-man-install.sh
cd Test
../vcf2hap --xz generand < generand.vcf.xz > test-results.hap
if diff -u generand-correct.hap test-results.hap; then
    printf "Test passed.\n"
else
    printf "Differences found, something is wrong.\n"
fi
