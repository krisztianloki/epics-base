/* cvtFast.c */
/* share/src/libCom $Id$ */
/* Very efficient routines to convert numbers to strings
 *      Author: Bob Dalesio wrote cvtFloatToString (called FF_TO_STR)
 *			Code is same for cvtDoubleToString
 *		Marty Kraimer wrote cvtCharToString,cvtUcharToString
 *			cvtShortToString,cvtUshortToString,
 *			cvtLongToString, and cvtUlongToString
 *		Mark Anderson wrote cvtLongToHexString, cvtLongToOctalString,
 *			adopted cvt[Float/Double]ExpString and
 *			cvt[Float/Double]CompactString from fToEStr
 *			and fixed calls to gcvt
 *
 *      Date:            12 January 1993
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1991, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 * Modification Log:
 * -----------------
 * .01	mrk	12-09-92	Taken from dbAccess and made into library
 * .02  mda	01-12-93	Add cvt[Float/Double]ToExpString,
 *				cvt[Float/Double]ToCompactString,
 *				cvtLongToHex, cvtLongToOctal routines, fix
 *				calls to gcvt, etc.
 * .03	joh	03-30-93	added bit field extract/ insert routines
 * .04	mrk	01-28-94	replaced gcvt by e conversion
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>		/* XPG2/XPG3/POSIX.1/FIPS151-1/ANSI-C */
#include <cvtFast.h>

/*
 * This routine converts numbers less than 10,000,000. It defers to f_to_str for
 * numbers requiring more than 8 places of precision. There are only eight decimal
 */
static long	frac_multiplier[] =
	{1,10,100,1000,10000,100000,1000000,10000000,100000000};

#ifdef __STDC__
int cvtFloatToString(
	float flt_value,
	char  *pstr_value,
	unsigned short precision)
{
#else
int cvtFloatToString(flt_value,pstr_value,precision)
	float flt_value;
	char  *pstr_value;
	unsigned short precision;
{
#endif /*__STDC__*/
        unsigned short	got_one,i;
	long		whole,iplace,number,fraction,fplace;
	float		ftemp;
	char		*startAddr;

	/* can this routine handle this conversion */
	if (precision > 8 || flt_value > 10000000.0 || flt_value < -10000000.0) {
		sprintf(pstr_value,"%12.5e\0",(double)flt_value);
		return((int)strlen(pstr_value));
	}
	startAddr = pstr_value;

	/* determine the sign */
        if (flt_value < 0){
                *pstr_value++ = '-';
                flt_value = -flt_value;
        };

 	/* remove the whole number portion */
	whole = flt_value;
	ftemp = flt_value - whole;

	/* multiplier to convert fractional portion to integer */
	fplace = frac_multiplier[precision];
	fraction = ftemp * fplace * 10;	
	fraction = (fraction + 5) / 10;		/* round up */

	/* determine rounding into the whole number portion */
	if ((fraction / fplace) >= 1){
		whole++;
		fraction -= fplace;
	}

	/* whole numbers */
        got_one = 0;
        for (iplace = 1000000; iplace >= 1; iplace /= 10){
                if (whole >= iplace){
                        got_one = 1;
                        number = whole / iplace;
                        whole = whole - (number * iplace);
                        *pstr_value = number + '0';
                        pstr_value++;
                }else if (got_one){
                        *pstr_value = '0';
                        pstr_value++;
                }
        }
        if (!got_one){
                *pstr_value = '0';
                pstr_value++;
        }

        /* fraction */
        if (precision > 0){
		/* convert fractional portional to ASCII */
                *pstr_value = '.';
                pstr_value++;
                for (fplace /= 10, i = precision; i > 0; fplace /= 10,i--){
                        number = fraction / fplace;
                        fraction -= number * fplace;
                        *pstr_value = number + '0';
                        pstr_value++;
                }
        }
        *pstr_value = 0;

        return((int)(pstr_value - startAddr));
}


#ifdef __STDC__
int cvtDoubleToString(
	double flt_value,
	char  *pstr_value,
	unsigned short precision)
{
#else
int cvtDoubleToString(flt_value,pstr_value,precision)
	double flt_value;
	char  *pstr_value;
	unsigned short precision;
{
#endif /*__STDC__*/
        unsigned short	got_one,i;
	long		whole,iplace,number,fraction,fplace;
	double		ftemp;
	char		*startAddr;

	/* can this routine handle this conversion */
	if (precision > 8 || flt_value > 10000000.0 || flt_value < -10000000.0) {
		sprintf(pstr_value,"%12.5e\0",flt_value);
		return((int)strlen(pstr_value));
	}
	startAddr = pstr_value;

	/* determine the sign */
        if (flt_value < 0){
                *pstr_value++ = '-';
                flt_value = -flt_value;
        };

 	/* remove the whole number portion */
	whole = flt_value;
	ftemp = flt_value - whole;

	/* multiplier to convert fractional portion to integer */
	fplace = frac_multiplier[precision];
	fraction = ftemp * fplace * 10;	
	fraction = (fraction + 5) / 10;		/* round up */

	/* determine rounding into the whole number portion */
	if ((fraction / fplace) >= 1){
		whole++;
		fraction -= fplace;
	}

	/* whole numbers */
        got_one = 0;
        for (iplace = 1000000; iplace >= 1; iplace /= 10){
                if (whole >= iplace){
                        got_one = 1;
                        number = whole / iplace;
                        whole = whole - (number * iplace);
                        *pstr_value = number + '0';
                        pstr_value++;
                }else if (got_one){
                        *pstr_value = '0';
                        pstr_value++;
                }
        }
        if (!got_one){
                *pstr_value = '0';
                pstr_value++;
        }

        /* fraction */
        if (precision > 0){
		/* convert fractional portional to ASCII */
                *pstr_value = '.';
                pstr_value++;
                for (fplace /= 10, i = precision; i > 0; fplace /= 10,i--){
                        number = fraction / fplace;
                        fraction -= number * fplace;
                        *pstr_value = number + '0';
                        pstr_value++;
                }
        }
        *pstr_value = 0;

        return((int)(pstr_value - startAddr));
}



/*
 * cvtFloatToExpString
 *
 * converts floating point numbers to E-format NULL terminated strings
 */

static float round_up[] = {.5, .05, .005,.0005,.00005,.000005,.0000005,
	.00000005,.000000005,.0000000005,.00000000005,.000000000005,
	.0000000000005,.00000000000005,.000000000000005,.0000000000000005};

#ifdef __STDC__
int cvtFloatToExpString(
  float			f_value,
  char			*pstr_value,
  unsigned short	f_precision)
{
#else
int cvtFloatToExpString(f_value,pstr_value,f_precision)
  float			f_value;
  char			*pstr_value;
  unsigned short	f_precision ;
{
#endif /*__STDC__*/
	register float	place,divisor;
	register short	e,i;
	short		number;
	register float	flt_value = f_value;
	register unsigned short	precision = f_precision;
	char			*startAddr;

	/* fix e_resolution to 1 */
#define e_resolution 1
#define MAX_OKAY_E_VALUE 1000000000000000.0

	/* check upper bound, use sprintf if this routine can't handle it */
	if (f_value >= MAX_OKAY_E_VALUE || f_value <= -MAX_OKAY_E_VALUE){
return((int)sprintf(pstr_value,"% 1.*e",f_precision,f_value));
	}

	startAddr = pstr_value;

	if (flt_value < 0){
		*pstr_value = '-';
		pstr_value++;
		flt_value = -flt_value;
	}else if (flt_value == 0.0){
		*pstr_value = '0';
		pstr_value++;
		/* fraction */
		*pstr_value = '.';
		pstr_value++;
		for (place = .1; precision > 0; place /= 10.0, precision--){
			*pstr_value = '0';
			pstr_value++;
		}
		*pstr_value = 'e';
		pstr_value++;
		*pstr_value = '+';
		pstr_value++;
		*pstr_value = '0';
		pstr_value++;
		*pstr_value = '0';
		pstr_value++;
		*pstr_value = '\0';
		return(pstr_value - startAddr);

	}


	/* determine which 3rd power of 10 to use */
	for (i=0,divisor=1; i<e_resolution; divisor*=10,i++);
	for (e = 12, place = 1000000000000.0;
	    flt_value < place;
	    e -= e_resolution, place /= divisor);
	if (e < -99){
		return((int)sprintf(pstr_value,"% 1.*e",f_precision,f_value));
	}

	/* whole numbers */
	flt_value = flt_value/place + round_up[precision];
	for (place = 100.0; place >= 1.0; place /= 10.0){
		if (flt_value >= place){
			number = flt_value / place;
			flt_value = flt_value - (number * place);
			*pstr_value = number + '0';
			pstr_value++;
		}
	}

	/* fraction */
	*pstr_value = '.';
	pstr_value++;
	for (place = .1; precision > 0; place /= 10.0, precision--){
		number =  flt_value / place ; 
		flt_value = flt_value - (number * place);
		*pstr_value = number + '0';
		pstr_value++;
	}

	/* exponent portion */
	*pstr_value = 'e';
	pstr_value++;
	if (e < 0){
		*pstr_value = '-';
		e = -e;
	}else{
		*pstr_value = '+';
	}
	pstr_value++;
	number = e / 10;
	*pstr_value = number + '0';
	pstr_value++;
	e -= number * 10;
	*pstr_value = e + '0';
	pstr_value++;
	*pstr_value = 0;

	return(pstr_value - startAddr);
}



/*
 * cvtFloatToCompactString
 *
 * Converts floating point numbers to %g format NULL terminated strings,
 * resulting in the most "compact" expression of the value
 * ("f" notation if 10-4 < |value| < 10+4, otherwise "e" notation)
 */
#ifdef __STDC__
int cvtFloatToCompactString(
  float			f_value,
  char			*pstr_value,
  unsigned short	f_precision )
{
#else
int cvtFloatToCompactString(f_value,pstr_value,f_precision)
  float			f_value;
  char			*pstr_value;
  unsigned short	f_precision ;
{
#endif /*__STDC__*/
  if ((f_value < 1.e4 && f_value > 1.e-4) ||
		(f_value > -1.e4 && f_value < -1.e-4) || f_value == 0.0) {
    return(cvtFloatToString(f_value,pstr_value,f_precision));
  } else {
    return(cvtFloatToExpString(f_value,pstr_value,f_precision));
  }
}



/*
 * cvtDoubleToExpString
 *
 * converts double precision floating point numbers to E-format NULL
 *	terminated strings
 */

#ifdef __STDC__
int cvtDoubleToExpString(
  double		f_value,
  char			*pstr_value,
  unsigned short	f_precision )
{
#else
int cvtDoubleToExpString(f_value,pstr_value,f_precision)
  double		f_value;
  char			*pstr_value;
  unsigned short	f_precision ;
{
#endif /*__STDC__*/
	register float	place,divisor;
	register short	e,i;
	short		number;
	register double	flt_value = f_value;
	register unsigned short	precision = f_precision;
	char			*startAddr;

	/* fix e_resolution to 1 */
#define e_resolution 1
#define MAX_OKAY_E_VALUE 1000000000000000.0

	/* check upper bound, use sprintf if this routine can't handle it */
	if (f_value >= MAX_OKAY_E_VALUE || f_value <= -MAX_OKAY_E_VALUE){
		return((int)sprintf(pstr_value,"% 1.*e",f_precision,f_value));
	}

	startAddr = pstr_value;

	if (flt_value < 0){
		*pstr_value = '-';
		pstr_value++;
		flt_value = -flt_value;
	}else if (flt_value == 0.0){
		*pstr_value = '0';
		pstr_value++;
		/* fraction */
		*pstr_value = '.';
		pstr_value++;
		for (place = .1; precision > 0; place /= 10.0, precision--){
			*pstr_value = '0';
			pstr_value++;
		}
		*pstr_value = 'e';
		pstr_value++;
		*pstr_value = '+';
		pstr_value++;
		*pstr_value = '0';
		pstr_value++;
		*pstr_value = '0';
		pstr_value++;
		*pstr_value = '\0';
		return(pstr_value - startAddr);

	}


	/* determine which 3rd power of 10 to use */
	for (i=0,divisor=1; i<e_resolution; divisor*=10,i++);
	for (e = 12, place = 1000000000000.0;
	    flt_value < place;
	    e -= e_resolution, place /= divisor);
	if (e < -99){
		return((int)sprintf(pstr_value,"% 1.*e",f_precision,f_value));
	}

	/* whole numbers */
	flt_value = flt_value/place + round_up[precision];
	for (place = 100.0; place >= 1.0; place /= 10.0){
		if (flt_value >= place){
			number = flt_value / place;
			flt_value = flt_value - (number * place);
			*pstr_value = number + '0';
			pstr_value++;
		}
	}

	/* fraction */
	*pstr_value = '.';
	pstr_value++;
	for (place = .1; precision > 0; place /= 10.0, precision--){
		number =  flt_value / place ; 
		flt_value = flt_value - (number * place);
		*pstr_value = number + '0';
		pstr_value++;
	}

	/* exponent portion */
	*pstr_value = 'e';
	pstr_value++;
	if (e < 0){
		*pstr_value = '-';
		e = -e;
	}else{
		*pstr_value = '+';
	}
	pstr_value++;
	number = e / 10;
	*pstr_value = number + '0';
	pstr_value++;
	e -= number * 10;
	*pstr_value = e + '0';
	pstr_value++;
	*pstr_value = 0;

	return(pstr_value - startAddr);
}



/*
 * cvtDoubleToCompactString
 *
 * Converts double precision floating point numbers to %g format NULL
 *	terminated strings, resulting in the most "compact" expression
 *	of the value ("f" notation if 10-4 < |value| < 10+4, otherwise
 *	"e" notation)
 */
#ifdef __STDC__
int cvtDoubleToCompactString(
  double		f_value,
  char			*pstr_value,
  unsigned short	f_precision )
{
#else
int cvtDoubleToCompactString(f_value,pstr_value,f_precision)
  double		f_value;
  char			*pstr_value;
  unsigned short	f_precision ;
{
#endif /*__STDC__*/
  if ((f_value < 1.e4 && f_value > 1.e-4) ||
		(f_value > -1.e4 && f_value < -1.e-4) || f_value == 0.0) {
    return(cvtDoubleToString(f_value,pstr_value,f_precision));
  } else {
    return(cvtDoubleToExpString(f_value,pstr_value,f_precision));
  }
}





/* Convert various integer types to ascii */

static char digit_to_ascii[10]={'0','1','2','3','4','5','6','7','8','9'};

#ifdef __STDC__
int cvtCharToString(
	char source,
	char *pdest)
{
#else
int cvtCharToString(source,pdest)
	char source;
	char *pdest;
{
#endif /*__STDC__*/
    unsigned char val,temp;
    char	  digit[3];
    int		  i,j;
    char	  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    if(source<0) {
	if(source == CHAR_MIN) {
	    sprintf(pdest,"%d",CHAR_MIN);
	    return((int)strlen(pdest));
	}
	*pdest++ = '-';
	source = -source;
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/10;
	digit[i] = digit_to_ascii[val - temp*10];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}


#ifdef __STDC__
int cvtUcharToString(
    unsigned char source,
    char	  *pdest)
{
#else
int cvtUcharToString(source,pdest)
    unsigned char source;
    char	  *pdest;
{
#endif /*__STDC__*/
    unsigned char val,temp;
    char	  digit[3];
    int		  i,j;
    char	  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/10;
	digit[i] = digit_to_ascii[val - temp*10];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}


#ifdef __STDC__
int cvtShortToString(
    short source,
    char  *pdest)
{
#else
int cvtShortToString(source,pdest)
    short source;
    char  *pdest;
{
#endif /*__STDC__*/
    short val,temp;
    char  digit[6];
    int	  i,j;
    char  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    if(source<0) {
	if(source == SHRT_MIN) {
	    sprintf(pdest,"%d",SHRT_MIN);
	    return((int)(strlen(pdest)));
	}
	*pdest++ = '-';
	source = -source;
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/10;
	digit[i] = digit_to_ascii[val - temp*10];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}


#ifdef __STDC__
int cvtUshortToString(
    unsigned short source,
    char	  *pdest)
{
#else
int cvtUshortToString(source,pdest)
    unsigned short source;
    char	  *pdest;
{
#endif /*__STDC__*/
    unsigned short val,temp;
    char	  digit[5];
    int		  i,j;
    char	  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/10;
	digit[i] = digit_to_ascii[val - temp*10];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}


#ifdef __STDC__
int cvtLongToString(
    long source,
    char  *pdest)
{
#else
int cvtLongToString(source,pdest)
    long source;
    char  *pdest;
{
#endif /*__STDC__*/
    long  val,temp;
    char  digit[11];
    int	  i,j;
    char  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    if(source<0) {
	if(source == LONG_MIN) {
	    sprintf(pdest,"%d",LONG_MIN);
	    return((int)strlen(pdest));
	}
	*pdest++ = '-';
	source = -source;
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/10;
	digit[i] = digit_to_ascii[val - temp*10];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}


#ifdef __STDC__
int cvtUlongToString(
    unsigned long source,
    char	  *pdest)
{
#else
int cvtUlongToString(source,pdest)
    unsigned long source;
    char	  *pdest;
{
#endif /*__STDC__*/
    unsigned long val,temp;
    char	  digit[10];
    int		  i,j;
    char	  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/10;
	digit[i] = digit_to_ascii[val - temp*10];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}


/* Convert hex digits to ascii */

static char hex_digit_to_ascii[16]={'0','1','2','3','4','5','6','7','8','9',
		'a','b','c','d','e','f'};


#ifdef __STDC__
int cvtLongToHexString(
    long source,
    char  *pdest)
{
#else
int cvtLongToHexString(source,pdest)
    long source;
    char  *pdest;
{
#endif /*__STDC__*/
    long  val,temp;
    char  digit[10];
    int	  i,j;
    char  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    if(source<0) {
	if(source == LONG_MIN) {
	    sprintf(pdest,"%x",LONG_MIN);
	    return((int)strlen(pdest));
	}
	*pdest++ = '-';
	source = -source;
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/16;
	digit[i] = hex_digit_to_ascii[val - temp*16];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}


#ifdef __STDC__
int cvtLongToOctalString(
    long source,
    char  *pdest)
{
#else
int cvtLongToOctalString(source,pdest)
    long source;
    char  *pdest;
{
#endif /*__STDC__*/
    long  val,temp;
    char  digit[16];
    int	  i,j;
    char  *startAddr = pdest;

    if(source==0) {
	*pdest++ = '0';
	*pdest = 0;
	return((int)(pdest-startAddr));
    }
    if(source<0) {
	if(source == LONG_MIN) {
	    sprintf(pdest,"%o",LONG_MIN);
	    return((int)strlen(pdest));
	}
	*pdest++ = '-';
	source = -source;
    }
    val = source;
    for(i=0; val!=0; i++) {
	temp = val/8;
	/* reuse digit_to_ascii since octal is a subset of decimal */
	digit[i] = digit_to_ascii[val - temp*8];
	val = temp;
    }
    for(j=i-1; j>=0; j--) {
	*pdest++ = digit[j];
    }
    *pdest = 0;
    return((int)(pdest-startAddr));
}




/*
 *
 * cvtBitsToUlong()
 *
 * extract a bit field from the source unsigend long
 */
#ifdef __STDC__
unsigned long cvtBitsToUlong(
unsigned long   src,
unsigned        bitFieldOffset,
unsigned        bitFieldLength)
{
#else
unsigned long cvtBitsToUlong(src,bitFieldOffset,bitFieldLength)
unsigned long   src;
unsigned        bitFieldOffset;
unsigned        bitFieldLength;
{
#endif /*__STDC__*/
        unsigned long   mask;

        src = src >> bitFieldOffset;

        mask = (1<<bitFieldLength)-1;

        src = src & mask;

        return src;
}



/*
 *
 * cvtUlongToBits()
 *
 * insert a bit field from the source unsigend long
 * into the destination unsigned long
 */
#ifdef __STDC__
unsigned long cvtUlongToBits(
unsigned long   src,
unsigned long   dest,
unsigned        bitFieldOffset,
unsigned        bitFieldLength)
{
#else
unsigned long cvtUlongToBits(src,dest,bitFieldOffset,bitFieldLength)
unsigned long   src;
unsigned long   dest;
unsigned        bitFieldOffset;
unsigned        bitFieldLength;
{
#endif /*__STDC__*/
        unsigned long   mask;

        mask = (1<<bitFieldLength)-1;
        mask = mask << bitFieldOffset;
        src = src << bitFieldOffset;
        dest = (dest & ~mask) | (src & mask);

        return dest;
}

/* needed if not standard C */
#ifndef __STDC__
/*
 * 
 * strtoul 
 * 
 * obtained from libiberty directory
 * 
 * needed because the standard libc.a doesn't have it
 *
 */

/*
 * strtol : convert a string to long.
 *
 * Andy Wilson, 2-Oct-89.
 */

#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include "ansidecl.h"

#ifndef ULONG_MAX
#define	ULONG_MAX	((unsigned long)(~0L))		/* 0xFFFFFFFF */
#endif

extern int errno;

unsigned long
strtoul(s, ptr, base)
     CONST char *s; char **ptr; int base;
{
  unsigned long total = 0, tmp = 0;
  unsigned digit;
  CONST char *start=s;
  int did_conversion=0;
  int negate = 0;

  if (s==NULL)
    {
      errno = ERANGE;
      if (!ptr)
	*ptr = (char *)start;
      return 0L;
    }

  while (isspace(*s))
    s++;
  if (*s == '+')
    s++;
  else if (*s == '-')
    s++, negate = 1;
  if (base==0 || base==16) /*  the 'base==16' is for handling 0x */
    {
      /*
       * try to infer base from the string
       */
      if (*s != '0')
        tmp = 10;	/* doesn't start with 0 - assume decimal */
      else if (s[1] == 'X' || s[1] == 'x')
	tmp = 16, s += 2; /* starts with 0x or 0X - hence hex */
      else
	tmp = 8;	/* starts with 0 - hence octal */
      if (base==0)
	base = (int)tmp;
    }

  while ( digit = *s )
    {
      if (digit >= '0' && digit < ('0'+base))
	digit -= '0';
      else
	if (base > 10)
	  {
	    if (digit >= 'a' && digit < ('a'+(base-10)))
	      digit = digit - 'a' + 10;
	    else if (digit >= 'A' && digit < ('A'+(base-10)))
	      digit = digit - 'A' + 10;
	    else
	      break;
	  }
	else
	  break;
      did_conversion = 1;
      tmp = (total * base) + digit;
      if (tmp < total)	/* check overflow */
	{
	  errno = ERANGE;
	  if (ptr != NULL)
	    *ptr = (char *)s;
	  return (ULONG_MAX);
	}
      total = tmp;
      s++;
    }
  if (ptr != NULL)
    *ptr = (char *) ((did_conversion) ? (char *)s : (char *)start);
  return negate ? -total : total;
}
#endif	/*  __STDC__ */
