#define MAX_LINE_LEN    65536
#define BUFF_SIZE       16383

// Match this with ad2vcf and vcf-split
// Yes, we actually saw a few INFO fields over 512k in some dbGap BCFs
#define VCF_INFO_MAX_CHARS          1048576
#define VCF_FORMAT_MAX_CHARS        4096
#define VCF_SAMPLE_MAX_CHARS        2048

#include "vcf2hap-protos.h"
