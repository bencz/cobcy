/* cobfunc.c
**
**	Defines cobol function library.
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "cobfunc.h"

#ifndef min
#   define min(a,b)	((a) < (b) ? (a) : (b))
#endif

void _RuntimeError (char * message)
{
    printf ("\nRuntime error: %s!\n\n", message);
    exit (1);
}

void _ReadStringVar (FILE * stream, char * var, char * pic)
{
int i, sl;
char c;

    sl = strlen (pic);
    memset (var, ' ', sl);
    for (i = 0; i < sl; ++ i) {
       c = fgetc (stream);
       if (c == '\n' || c == '\x0' || feof(stream))
	  break;
       var[i] = c;
    }
}

void _ReadIntegerVar (FILE * stream, long int var, char * pic)
{
int i;
char buffer[20];
    memset (buffer, ' ', strlen (pic));
    for (i = 0; i < strlen (pic); ++ i)
       buffer[i] = fgetc (stream);
    var = atol (buffer);
}

void _ReadFloatVar (FILE * stream, double var, char * pic)
{
int i;
char buffer[20];
    memset (buffer, ' ', strlen (pic));
    for (i = 0; i < strlen (pic); ++ i)
       buffer[i] = fgetc (stream);
    var = atof (buffer);
}

void _WriteStringVar (FILE * stream, char * var, char * pic)
{
int i;
    for (i = 0; i < strlen(pic); ++ i) {
       if (pic[i] == 'x')
          fprintf (stream, "%c", var[i]);
       else
          fprintf (stream, "%c", pic[i]);
    }
}

void _WriteIntegerVar (FILE * stream, long int var, char * pic)
{
char buffer[20];
int i, j = 0, nl, pl, fl = 0;

    sprintf (buffer, "%lu", var);
    nl = strlen (buffer);
    pl = strlen (pic);

    /* Calculate number of filler zeroes on the left */
    for (i = 0; i < pl; ++ i)
       if (pic[i] == '9' || pic[i] == 'z' || pic[i] == '*')
	  ++ fl;

    /* If the number does not fit, cut leftmost digits */
    if (fl < nl)
       j = nl - fl;

    for (i = 0; i < pl; ++ i) {
       if (pic[i] == '9') {
	  if (fl <= nl)
             fprintf (stream, "%c", buffer[j++]);
	  else
             fprintf (stream, " ");
	  -- fl;
       }
       else if (pic[i] == '+' || pic[i] == 's' || pic[i] == '-') {
          if (var > 0 && pic[i] == '-')
	     fprintf (stream, "+");
	  else if (var < 0)
	     fprintf (stream, "-");
	  else
	     fprintf (stream, " ");
       }
       else if (pic[i] == 'z' || pic[i] == '*')
	  fprintf (stream, "0");
       else if (pic[i] == '$')
	  fprintf (stream, "$");
       else
          fprintf (stream, "%c", (fl - 1 <= nl) ? pic[i] : ' ');
    }
}

void _WriteFloatVar (FILE * stream, double var, char * pic)
{
char format_s [10];
char buffer [20];
int fbp = 0, fap = 0, pl;
int i, j = 0;
int FoundPoint = 0;

    pl = strlen (pic);

    /* Calculate number of filler zeroes on the left */
    for (i = 0; i < pl; ++ i) {
       if (pic[i] == '9' || pic[i] == 'z' || pic[i] == '*') {
	  if (FoundPoint)
	     ++ fap;
	  else
	     ++ fbp;
       }
    }

    sprintf (format_s, "%%%d.%df", fbp, fap);
    sprintf (buffer, format_s, var);
    
    for (i = 0; i < pl; ++ i) {
       if (pic[i] == '9')
          fprintf (stream, "%c", buffer[j++]);
       else if (pic[i] == '+' || pic[i] == 's' || pic[i] == '-') {
          if (var > 0 && pic[i] == '-')
	     fprintf (stream, "+");
	  else if (var < 0)
	     fprintf (stream, "-");
	  else
	     fprintf (stream, " ");
       }
       else if (pic[i] == 'z' || pic[i] == '*')
	  fprintf (stream, "0");
       else if (pic[i] == 'v') {
	  fprintf (stream, ".");
	  ++ j;
       }
       else if (pic[i] == '$')
	  fprintf (stream, "$");
       else
          fprintf (stream, "%c", (buffer[i] != ' ') ? pic[i] : ' ');
    }
}

void _AssignVarString (char * var1, char * var2, int p1, int p2)
{
    if (p2 == 0)
       p2 = strlen (var2);

    memset (var1, ' ', p1);
    var1[p1] = '\x0';
    strncpy (var1, var2, min (p1, p2));
    
    if (p1 > p2)
       var1[p2] = ' ';
}

long int _RoundResult (double num)
{
long int tnum;
    tnum = (long int) num;
    if ((num - tnum) < 0.5)
       return (tnum);
    else
       return (tnum + 1);
}
