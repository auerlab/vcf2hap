/***************************************************************************
 *  Description:
 *      Generate a .hap file (for haplohseq) from a VCF
 *
 *  Arguments:
 *      1.  .vcf input file
 *      2.  .hap output
 *      3.  Sample ID (usually encoded in .vcf filename)
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

#define MAX_LINE_LEN    65536
#define BUFF_SIZE       16383

void    usage(char *arg0)

{
    fprintf(stderr, "Usage: %s [--xz] sample-id [< VCF-input] [> HAP-output]\n", arg0);
    exit(EX_USAGE);
}


int     main(int argc,char *argv[])

{
    size_t  c,
	    bytes_read;
    FILE    *vcf_stream = stdin,
	    *hap_stream1 = stdout,
	    *hap_stream2;
    char    vcf_line[MAX_LINE_LEN+1],
	    buff[BUFF_SIZE+1],
	    *fields[10],
	    *p,
	    *cwd,
	    *sample_id;
    
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
    
    while ( fgets(vcf_line, MAX_LINE_LEN, vcf_stream) != NULL )
    {
	if ( ! (vcf_line[0] == '#') )
	{
	    /* Split line into separate fields */
	    for (c = 0, p = vcf_line; c < 10; ++c)
	    {
		fields[c] = strsep(&p, "\t\n");
		//fprintf(stderr, "%zu '%s'\n", c, fields[c]);
	    }
	    
	    /*
	     *  Using strstr assumes that strings like "0/0" only occur once
	     *  per field.
	     *  Might be better to locate the GT field in FORMAT and check
	     *  only that position.
	     */
	    
	    if ( (*fields[3] == '\0') || (*fields[4] == '\0') ||
		strstr(fields[9], ".|.") != NULL )
	    {
		/* Ignore lines with no data */
	    }
	    else if ( strstr(fields[9], "0|0") != NULL )
	    {
		putc(*fields[3], hap_stream1);
		putc(*fields[3], hap_stream2);
	    }
	    else if ( strstr(fields[9], "0|1") != NULL )
	    {
		putc(*fields[3], hap_stream1);
		putc(*fields[4], hap_stream2);
	    }
	    else if ( strstr(fields[9], "1|0") != NULL )
	    {
		putc(*fields[4], hap_stream1);
		putc(*fields[3], hap_stream2);
	    }
	    else if ( strstr(fields[9], "1|1") != NULL )
	    {
		putc(*fields[4], hap_stream1);
		putc(*fields[4], hap_stream2);
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
