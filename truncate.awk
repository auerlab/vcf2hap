#############################################################################
#   Description:
#       Reduce a large VCF file by limiting the number of entries for
#       each chromosome.
#
#   History: 
#   Date        Name        Modification
#   2019-12-05  Jason Bacon Begin
#############################################################################

BEGIN {
    last = 0;
}
{
    if ( $1 == last )
    {
	if ( count < 100 )
	{
	    print $0;
	}
	++count;
    }
    else
    {
	last = $1;
	count = 0;
    }
}
