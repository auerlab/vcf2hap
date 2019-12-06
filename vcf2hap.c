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

#define MAX_LINE_LEN    4095
#define BUFF_SIZE       16383

void    usage(char *arg0)

{
    fprintf(stderr, "Usage: %s sample-id [< VCF-input] [> HAP-output]\n", arg0);
    exit(EX_USAGE);
}


int     main(int argc,char *argv[])

{
    extern int  errno;
    size_t  c,
	    bytes_read;
    FILE    *infile = stdin,
	    *outfile1 = stdout,
	    *outfile2;
    char    vcf_line[MAX_LINE_LEN+1],
	    buff[BUFF_SIZE+1],
	    *fields[10],
	    *p,
	    *cwd,
	    *sample_id;
    
    if ( argc != 2 )
	usage(argv[0]);
    
    /* Create a temporary file for haplo2 in the current directory */
    cwd = getcwd(NULL, 0);
    setenv("TMPDIR", cwd, 1);
    free(cwd);
    outfile2 = tmpfile();

    /* Add headers */
    sample_id = argv[1];
    fprintf(outfile1, "%s HAPLO1 ", sample_id);
    fprintf(outfile2, "%s HAPLO2 ", sample_id);
    
    while ( fgets(vcf_line, MAX_LINE_LEN, infile) != NULL )
    {
	if ( ! (vcf_line[0] == '#') )
	{
	    /* Split line into separate fields */
	    for (c = 0, p = vcf_line; c < 10; ++c)
	    {
		fields[c] = strsep(&p, "\t\n");
		//printf("%zu '%s'\n", c, fields[c]);
	    }
	    
	    /*
	     *  Using strstr assumes that strings like "0/0" only occur once
	     *  per field.
	     *  Might be better to locate the GT field in FORMAT and check
	     *  only that position.
	     */
	    
	    if ( (*fields[3] == '\0') || (*fields[4] == '\0') ||
		strstr(fields[9], "./.") != NULL )
	    {
		/* Ignore lines with no data */
	    }
	    else if ( strstr(fields[9], "0/0") != NULL )
	    {
		putc(*fields[3], outfile1);
		putc(*fields[3], outfile2);
	    }
	    else if ( strstr(fields[9], "0/1") != NULL )
	    {
		putc(*fields[3], outfile1);
		putc(*fields[4], outfile2);
	    }
	    else if ( strstr(fields[9], "1/0") != NULL )
	    {
		putc(*fields[4], outfile1);
		putc(*fields[3], outfile2);
	    }
	    else if ( strstr(fields[9], "1/1") != NULL )
	    {
		putc(*fields[4], outfile1);
		putc(*fields[4], outfile2);
	    }
	}
    }
    fclose(infile);
    
    /* Separate line for each haplo */
    putc('\n', outfile1);
    putc('\n', outfile2);
    
    /* Append haplo2 line to haplo1 file */
    rewind(outfile2);
    while ( (bytes_read = fread(buff, 1, BUFF_SIZE, outfile2)) != 0 )
	fwrite(buff, 1, bytes_read, outfile1);
    
    fclose(outfile2);
    fclose(outfile1);
    return EX_OK;
}
