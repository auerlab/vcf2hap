/***************************************************************************
 *  Description:
 *      Generate a .hap file suitable for haplohseq from a phased VCF
 *      Based on format_hap.py and roughly 19x faster
 *
 *  Returns:
 *      Standard sysexits
 *
 *  History: 
 *  Date        Name        Modification
 *  2019-11-15  J Bacon     Begin
 ***************************************************************************/

#include <stdio.h>
#include <sysexits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <vcfio.h>
#include "vcf2hap.h"

void    usage(const char *arg0)

{
    fprintf(stderr, "Usage: %s [--xz] sample-id [< VCF-input] [> HAP-output]\n", arg0);
    exit(EX_USAGE);
}


int     main(int argc,const char *argv[])

{
    size_t  bytes_read;
    int     ch;
    FILE    *vcf_stream = stdin,
	    *hap_stream1 = stdout,
	    *hap_stream2;
    char    buff[BUFF_SIZE+1],
	    *cwd;
    const char  *sample_id;
    vcf_call_t  vcf_call;
    
    switch(argc)
    {
	case    2:
	    sample_id = argv[1];
	    vcf_stream = stdin;
	    break;
	
	case    3:
	    if ( strcmp(argv[1], "--xz") == 0 )
	    {
		if ( (vcf_stream = popen("unxz -c", "r")) == NULL )
		{
		    fprintf(stderr, "%s: Cannot create unxz pipe: %s\n",
			    argv[0], strerror(errno));
		    exit(EX_NOINPUT);
		}
		sample_id = argv[2];
	    }
	    else
	    {
		usage(argv[0]);
		return EX_USAGE;    // Never reached, just to silence warning
	    }
	    break;
	
	default:
	    usage(argv[0]);
	    return EX_USAGE;
    }
    
    /* Create a temporary file for haplo2 in the current directory */
    cwd = getcwd(NULL, 0);
    setenv("TMPDIR", cwd, 1);
    free(cwd);
    hap_stream2 = tmpfile();

    /* Add headers */
    fprintf(hap_stream1, "%s HAPLO1 ", sample_id);
    fprintf(hap_stream2, "%s HAPLO2 ", sample_id);
    
    while ( (ch = getc(vcf_stream)) != EOF )
    {
	if ( ch == '#' )
	{
	    tsv_skip_rest_of_line(vcf_stream);
	}
	else
	{
	    ungetc(ch, vcf_stream);
	    switch(vcf_read_ss_call(vcf_stream, &vcf_call,
				    VCF_SAMPLE_MAX_CHARS))
	    {
		case    VCF_OK:
		    
		    /*
		     *  Using strstr assumes that strings like "0/0" only occur once
		     *  per field.
		     *  Might be better to locate the GT field in FORMAT and check
		     *  only that position.
		     */
		    
		    if ( (*VCF_REF(&vcf_call) == '\0') || (*VCF_ALT(&vcf_call) == '\0') ||
			strstr(VCF_SINGLE_SAMPLE(&vcf_call), ".|.") != NULL )
		    {
			/* Ignore lines with no data */
		    }
		    else if ( strstr(VCF_SINGLE_SAMPLE(&vcf_call), "0|0") != NULL )
		    {
			putc(*VCF_REF(&vcf_call), hap_stream1);
			putc(*VCF_REF(&vcf_call), hap_stream2);
		    }
		    else if ( strstr(VCF_SINGLE_SAMPLE(&vcf_call), "0|1") != NULL )
		    {
			putc(*VCF_REF(&vcf_call), hap_stream1);
			putc(*VCF_ALT(&vcf_call), hap_stream2);
		    }
		    else if ( strstr(VCF_SINGLE_SAMPLE(&vcf_call), "1|0") != NULL )
		    {
			putc(*VCF_ALT(&vcf_call), hap_stream1);
			putc(*VCF_REF(&vcf_call), hap_stream2);
		    }
		    else if ( strstr(VCF_SINGLE_SAMPLE(&vcf_call), "1|1") != NULL )
		    {
			putc(*VCF_ALT(&vcf_call), hap_stream1);
			putc(*VCF_ALT(&vcf_call), hap_stream2);
		    }
		    break;
		    
		case    VCF_READ_EOF:
		    break;
		
		default:
		    fprintf(stderr, "%s: Error reading VCF call.\n", argv[0]);
		    exit(EX_DATAERR);
	    }
	}
    }
    fclose(vcf_stream);
    
    /* Separate line for each haplo */
    putc('\n', hap_stream1);
    putc('\n', hap_stream2);
    
    /* Append haplo2 line to haplo1 file */
    rewind(hap_stream2);
    while ( (bytes_read = fread(buff, 1, BUFF_SIZE, hap_stream2)) != 0 )
	fwrite(buff, 1, bytes_read, hap_stream1);
    
    fclose(hap_stream2);
    fclose(hap_stream1);
    return EX_OK;
}
