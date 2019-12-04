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
    fprintf(stderr, "Usage: %s file.vcf file.hap sample-id\n", arg0);
    exit(EX_USAGE);
}


int     main(int argc,char *argv[])

{
    extern int  errno;
    size_t  c,
	    bytes_read;
    FILE    *infile,
	    *outfile1,
	    *outfile2;
    char    vcf_line[MAX_LINE_LEN+1],
	    buff[BUFF_SIZE+1],
	    *fields[10],
	    *p,
	    *cwd;
    
    if ( argc != 4 )
	usage(argv[0]);
    
    if ( (infile = fopen(argv[1], "r")) == NULL )
    {
	fprintf(stderr, "Cannot open %s: %s\n", argv[1], strerror(errno));
	exit(EX_NOINPUT);
    }
    
    if ( (outfile1 = fopen(argv[2], "w")) == NULL )
    {
	fprintf(stderr, "Cannot open %s: %s\n", argv[2], strerror(errno));
	exit(EX_NOINPUT);
    }

    /* Create a temporary file for haplo2 in the current directory */
    cwd = getcwd(NULL, 0);
    setenv("TMPDIR", cwd, 1);
    free(cwd);
    outfile2 = tmpfile();

    /* Add headers */
    fprintf(outfile1, "%s HAPLO1 ", argv[3]);
    fprintf(outfile2, "%s HAPLO2 ", argv[3]);
    
    while ( fgets(vcf_line, MAX_LINE_LEN, infile) != NULL )
    {
	if ( ! (vcf_line[0] == '#') )
	{
	    /* Split line into separate fields */
	    for (c = 0, p = vcf_line; c < 10; ++c)
	    {
		fields[c] = strsep(&p, "\t\n");
		// printf("%zu '%s'\n", c, fields[c]);
	    }
	    
	    if ( (*fields[3] == '\0') || (*fields[4] == '\0') ||
		strcmp(fields[9], ".|.") == 0 )
	    {
		/* Ignore lines with no data */
	    }
	    else if ( strcmp(fields[9], "0|0") == 0 )
	    {
		putc(*fields[3], outfile1);
		putc(*fields[3], outfile2);
	    }
	    else if ( strcmp(fields[9], "0|1") == 0 )
	    {
		putc(*fields[3], outfile1);
		putc(*fields[4], outfile2);
	    }
	    if ( strcmp(fields[9], "1|0") == 0 )
	    {
		putc(*fields[4], outfile1);
		putc(*fields[3], outfile2);
	    }
	    if ( strcmp(fields[9], "1|1") == 0 )
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
