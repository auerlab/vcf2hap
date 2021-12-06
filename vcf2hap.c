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
#include <biolibc/vcf.h>
#include <xtend/dsv.h>
#include "vcf2hap.h"

void    usage(const char *argv[])

{
    fprintf(stderr, "\nUsage: %s [--version]\n", argv[0]);
    fprintf(stderr, "Usage: %s [--xz] sample-id [< VCF-input] [> HAP-output]\n", argv[0]);
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
    bl_vcf_t  vcf_call;
    
    if ( (argc == 2) && (strcmp(argv[1],"--version")) == 0 )
    {
	printf("vcf2hap %s\n", VERSION);
	return EX_OK;
    }
    
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
		usage(argv);
		return EX_USAGE;    // Never reached, just to silence warning
	    }
	    break;
	
	default:
	    usage(argv);
    }
    
    bl_vcf_init(&vcf_call, BL_VCF_INFO_MAX_CHARS, BL_VCF_FORMAT_MAX_CHARS,
		  BL_VCF_SAMPLE_MAX_CHARS);
    
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
	    switch(bl_vcf_read_ss_call(vcf_stream, &vcf_call, BL_VCF_FIELD_ALL))
	    {
		case    BL_READ_OK:
		    
		    /*
		     *  Using strstr assumes that strings like "0/0" only occur once
		     *  per field.
		     *  Might be better to locate the GT field in FORMAT and check
		     *  only that position.
		     */
		    
		    if ( (*BL_VCF_REF(&vcf_call) == '\0') || (*BL_VCF_ALT(&vcf_call) == '\0') ||
			strstr(BL_VCF_SINGLE_SAMPLE(&vcf_call), ".|.") != NULL )
		    {
			/* Ignore lines with no data */
		    }
		    else if ( strstr(BL_VCF_SINGLE_SAMPLE(&vcf_call), "0|0") != NULL )
		    {
			putc(*BL_VCF_REF(&vcf_call), hap_stream1);
			putc(*BL_VCF_REF(&vcf_call), hap_stream2);
		    }
		    else if ( strstr(BL_VCF_SINGLE_SAMPLE(&vcf_call), "0|1") != NULL )
		    {
			putc(*BL_VCF_REF(&vcf_call), hap_stream1);
			putc(*BL_VCF_ALT(&vcf_call), hap_stream2);
		    }
		    else if ( strstr(BL_VCF_SINGLE_SAMPLE(&vcf_call), "1|0") != NULL )
		    {
			putc(*BL_VCF_ALT(&vcf_call), hap_stream1);
			putc(*BL_VCF_REF(&vcf_call), hap_stream2);
		    }
		    else if ( strstr(BL_VCF_SINGLE_SAMPLE(&vcf_call), "1|1") != NULL )
		    {
			putc(*BL_VCF_ALT(&vcf_call), hap_stream1);
			putc(*BL_VCF_ALT(&vcf_call), hap_stream2);
		    }
		    break;
		    
		case    BL_READ_EOF:
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
