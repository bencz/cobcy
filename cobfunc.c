/* cobfunc.c
**
**   Defines cobol function library.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
/* #include <endian.h> */
#include <math.h>
#include <stdlib.h>
#ifndef NOCURSES
    #include <ncurses.h>
#endif
#include "cobfunc.h"

#ifndef min
#define min(a,b)	((a) < (b) ? (a) : (b))
#endif

static char _strbuf[201];	/* For ---toString routines. This is the
				   buffer */
static int __curses_enabled = 0;

static int _IsInSet (char c, const char *set)
{
    int i;

    for (i = 0; i < strlen(set); ++i)
	if (set[i] == c)
	    return (1);		/* YES */
    return (0);			/* NO */
}

int _Alphabetic (const char *str)
{
    int i;

    for (i = 0; i < strlen(str); i++)
	if (!(isalpha(str[i]) || str[i] == ' '))
	    return (0);
    return (1);
}

/*
** what = 1  : lowercase test
** what = 2  : uppercase test 
*/
int _AlphabeticCase(const char *str, int what)
{
int i;
    if (_Alphabetic(str)) {
	for (i = 0; i < strlen(str); i++) {
	    if (what == 2 && islower(str[i]))
		return (0);
	    else if (what == 1 && isupper(str[i]))
		return (0);
	}
    }
    else
	return (0);
    return (1);
}

void _RuntimeError (const char *message)
{
    printf("\nRuntime error: %s!\n\n", message);
#ifndef NOCURSES
    if (__curses_enabled)
	endwin();
#endif
    exit(1);
}

void _ReadStringVar(FILE * stream, char *var, const char *pic)
{
int si, pi, sl, pl, maxChars = 1;

    pl = strlen(pic);
    sl = 0;
    /* Find length of the string by excluding editing characters from pic */
    for (pi = 0; pi < pl; ++pi)
	if (!_IsInSet(pic[pi], ".,/bB0"))
	    ++sl;

    /* All string variables are spaces by default */
    memset(var, ' ', sl);

    /* Read characters until a count of chars is reached */
    si = 0;

    /* Iterate over picture counting all fillable positions */
    for (pi = 0; pi < pl; ++pi)
	if (!_IsInSet(pic[pi], ".,/bB0"))
	    ++ maxChars;

    fgets (var, maxChars, stream);
}

void _ReadIntegerVar(FILE * stream, long int *var, const char *pic)
{
char buffer[20];

    _ReadStringVar(stream, buffer, pic);
    *var = atol(buffer);
}

void _ReadFloatVar(FILE * stream, double *var, const char *pic)
{
char buffer[20];

    _ReadStringVar(stream, buffer, pic);
    *var = atof(buffer);
}

void _WriteStringVar(FILE * stream, const char *var, const char *pic)
{
    char strbuf [3];
    int i, j;
    if (pic == NULL) {
#ifndef NOCURSES
	if (__curses_enabled && stream == NULL) {
	    addstr (var);
	    refresh();
	}
	else
#endif
	    fprintf (stream, var);
	return;
    }
    for (i = 0, j = 0; i < strlen(pic); ++i) {
	switch (pic[i]) {
	    case 'x':
		sprintf (strbuf, "%c", var[j++]);
		break;
	    case 'b':
		strcpy (strbuf, " ");
		break;
	    default:
		sprintf (strbuf, "%c", pic[i]);
	}
#ifndef NOCURSES
	if (__curses_enabled && stream == NULL)
	    addstr (strbuf);
	else
#endif
	    fprintf (stream, strbuf);
    }
}

void _WriteIntegerVar(FILE * stream, long int var, const char *pic)
{
    char* pNumBuf;
    pNumBuf = _IntegerToString(var, pic);
#ifndef NOCURSES
    if (__curses_enabled && stream == NULL)
	addstr (pNumBuf);
    else
#endif
	fprintf (stream, pNumBuf);
}

void _WriteFloatVar(FILE * stream, double var, const char *pic)
{
    char* pNumBuf;
    pNumBuf = _FloatToString(var, pic);
#ifndef NOCURSES
    if (__curses_enabled && stream == NULL)
	addstr (pNumBuf);
    else
#endif
	fprintf (stream, pNumBuf);
}

/* Result in _strbuf */
char* _IntegerToString(long int var, const char *pic)
{
char barenum[20];
int i, j = 0, nl, pl, fl = 0;
int sp = 0;

    sprintf(barenum, "%lu", var);
    nl = strlen(barenum);
    pl = strlen(pic);

    /* Calculate number of filler zeroes on the left */
    for (i = 0; i < pl; ++i)
	if (pic[i] == '9' || pic[i] == 'z' || pic[i] == '*')
	    ++fl;

    /* If the number does not fit, cut leftmost digits */
    if (fl < nl)
	j = nl - fl;

    for (i = 0; i < pl; ++i) {
	if (pic[i] == '9') {
	    if (fl <= nl)
		_strbuf[sp++] = barenum[j++];
	    else
		_strbuf[sp++] = ' ';
	    --fl;
	}
	else if (pic[i] == '.')
	    _strbuf[sp++] = '.';
	else if (pic[i] == ',')
	    _strbuf[sp++] = ',';
	else if (pic[i] == '/')
	    _strbuf[sp++] = '/';
	else if (pic[i] == 'b')
	    _strbuf[sp++] = ' ';
	else if (pic[i] == '0')
	    _strbuf[sp++] = '0';
	else if (pic[i] == '+' || pic[i] == 's' || pic[i] == '-') {
	    if (var > 0 && pic[i] == '-')
		_strbuf[sp++] = '+';
	    else if (var < 0)
		_strbuf[sp++] = '-';
	    else
		_strbuf[sp++] = ' ';
	}
	else if (pic[i] == 'z' || pic[i] == '*')
	    _strbuf[sp++] = '0';
	else if (pic[i] == '$')
	    _strbuf[sp++] = '$';
	else
	    _strbuf[sp++] = (fl - 1 <= nl) ? pic[i] : ' ';
    }
    _strbuf[sp] = '\x0';

    return (_strbuf);
}

/* Result in _strbuf */
char* _FloatToString(double var, const char *pic)
{
char format_s[10];
char barenum[20];
int fbp = 0, fap = 0, pl;
int i, j = 0;
int FoundPoint = 0;
int sp = 0;

    pl = strlen(pic);

    /* Calculate number of filler zeroes on the left */
    for (i = 0; i < pl; ++i) {
	if (pic[i] == '9' || pic[i] == '0' || pic[i] == 'z' || pic[i] == '*') {
	    if (FoundPoint)
		++fap;
	    else
		++fbp;
	}
	else if (pic[i] == 'v')
	    FoundPoint = 1;
    }

    sprintf(format_s, "%%%d.%df", fbp, fap);
    sprintf(barenum, format_s, var);

    /* 
       ** sprintf will print correct number of digits after point, ** but
       only as many as needed before the point. ** We thus need to reset
       the index variable j to the real starting point. */
    j = strlen(barenum) - (fbp + 1 + fap);

    for (i = 0; i < pl; ++i) {
	if (pic[i] == '9') {
	    if (j >= 0)
		_strbuf[sp++] = barenum[j];
	    else
		_strbuf[sp++] = ' ';
	    ++j;
	}
	else if (pic[i] == '.')
	    _strbuf[sp++] = '.';
	else if (pic[i] == ',')
	    _strbuf[sp++] = ',';
	else if (pic[i] == '/')
	    _strbuf[sp++] = '/';
	else if (pic[i] == 'b')
	    _strbuf[sp++] = ' ';
	else if (pic[i] == '0')
	    _strbuf[sp++] = '0';
	else if (pic[i] == '+' || pic[i] == 's' || pic[i] == '-') {
	    if (var > 0 && pic[i] == '-')
		_strbuf[sp++] = '+';
	    else if (var < 0)
		_strbuf[sp++] = '-';
	    else
		_strbuf[sp++] = ' ';
	}
	else if (pic[i] == 'z' || pic[i] == '*') {
	    _strbuf[sp++] = '0';
	    ++j;
	}
	else if (pic[i] == 'v') {
	    _strbuf[sp++] = '.';
	    ++j;
	}
	else if (pic[i] == '$')
	    _strbuf[sp++] = '$';
	else
	    _strbuf[sp++] = pic[i];
    }
    _strbuf[sp] = '\x0';

    return (_strbuf);
}

void _AssignVarString(char *var1, const char *var2, int p1, int p2)
{
    if (p2 == 0)
	p2 = strlen(var2);

    memset(var1, ' ', p1);
    var1[p1] = '\x0';
    strncpy(var1, var2, min(p1, p2));

    if (p1 > p2)
	var1[p2] = ' ';
}

double _RoundResult(double num, const char *pic)
{
    long int tnum;

    tnum = (long int) num;
    if ((num - tnum) < 0.5)
	return (tnum);
    else
	return (tnum + 1);
}

static int _FieldsInSig(const char *sig)
{
    int i, IsToken = 0;
    int nTokens = 0;

    for (i = 0; i < strlen(sig); ++i) {
	if (sig[i] == ' ')
	    IsToken = 0;
	else {
	    if (!IsToken)
		++nTokens;
	    IsToken = 1;
	}
    }
    /* Each field is defined by NAME TYPE SIZE DECIMAL */
    return (nTokens / 4);
}

/*
** The syntax is same as strcpy, only that the source pointer
** is moved to the first char after read token. That's what
** char** is for.
*/
static void _TokenCopy(char *dest, const char **src)
{
    char c;
    const char *sptr;
    char *dptr;

    sptr = *src;		/* These are added just for readability, */
    dptr = dest;		/* like defines                  */

    while ((c = *sptr) != '\x0' && c == ' ')
	++sptr;

    while ((c = *sptr) != '\x0' && c != ' ') {
	*dptr = *sptr;
	++sptr;
	++dptr;
    }
    *dptr = '\x0';		/* Null terminate the string */
    *src = sptr;		/* Update the passed source pointer */
}

static DBF_Field *_SigToFields(const char *sig, int *lpnFields)
{
    DBF_Field *Fields;
    char buffer[50];
    const char *cursigpos;
    int i, nFields;

    nFields = _FieldsInSig(sig);
    *lpnFields = nFields;

    Fields = (DBF_Field *) calloc(1, nFields * sizeof(DBF_Field));

    cursigpos = sig;
    for (i = 0; i < nFields; ++i) {
	/** First token is the name of the field */
	_TokenCopy(buffer, &cursigpos);
	/* Assuming the last character to be set to 0 by calloc */
	strncpy(Fields[i].Name, buffer, DBF_FIELD_NAME_LENGTH - 1);

	/** Second token is the character denoting type of field */
	_TokenCopy(buffer, &cursigpos);
	Fields[i].Type = buffer[0];

	/** Third token is the size of the field */
	_TokenCopy(buffer, &cursigpos);
	Fields[i].FieldLength = atoi(buffer);

	/** Fourth token is the number of decimal places, if any */
	_TokenCopy(buffer, &cursigpos);
	Fields[i].DecimalPlaces = atoi(buffer);
    }

    /* Fields is freed in DBF_Close */
    return (Fields);
}

void _OpenSequentialFile(FILE ** fp, const char *filename, const char *mode)
{
char ErrorBuffer[80];

    if ((*fp = fopen(filename, mode)) == NULL) {
	sprintf(ErrorBuffer, "could not open file '%s'", filename);
	_RuntimeError(ErrorBuffer);
    }
}

void _OpenRelativeFile(DBF_FILE ** fp, const char *filename, const char *sig, const char *mode)
{
char ErrorBuffer[80];
DBF_Field *Fields;
int nFields;

    /* If opening for OUTPUT, remove the file first */
    if (mode[0] == 'w' && mode[1] != '+' && access(filename, 0) == 0)
	unlink(filename);

    /* If the file does not exist, use Create call */
    if (access(filename, 0) != 0) {
	/* Cannot open file for reading if it does not exist */
	if (mode[0] == 'r') {
	    /* Make it runtime error for now. Later change to ** setting
	       the status variable (how?) and gracefully leaving. */
	    sprintf(ErrorBuffer, "file '%s' does not exist", filename);
	    _RuntimeError(ErrorBuffer);
	}
	Fields = _SigToFields(sig, &nFields);
	if ((*fp = DBF_Create(filename, nFields, Fields)) == NULL) {
	    sprintf(ErrorBuffer, "could not create data file '%s'", filename);
	    _RuntimeError(ErrorBuffer);
	}
    }
    else {
	if ((*fp = DBF_Open(filename, mode)) == NULL) {
	    sprintf(ErrorBuffer, "could not open file '%s'", filename);
	    _RuntimeError(ErrorBuffer);
	}
    }
}

void _OpenIndexedFile(DBF_FILE ** fp, const char *filename,
		      const char *sig, const char *mode)
{
char ErrorBuffer[80];
DBF_Field *Fields;
int nFields;

    /* If opening for OUTPUT, remove the file first */
    if (mode[0] == 'w' && mode[1] != '+' && access(filename, 0) == 0)
	unlink(filename);

    /* If the file does not exist, use Create call */
    if (access(filename, 0) != 0) {
	/* Cannot open file for reading if it does not exist */
	if (mode[0] == 'r') {
	    /* Make it runtime error for now. Later change to ** setting
	       the status variable (how?) and gracefully leaving. */
	    sprintf(ErrorBuffer, "file '%s' does not exist", filename);
	    _RuntimeError(ErrorBuffer);
	}
	Fields = _SigToFields(sig, &nFields);
	if ((*fp = DBF_Create(filename, nFields, Fields)) == NULL) {
	    sprintf(ErrorBuffer, "could not create data file '%s'", filename);
	    _RuntimeError(ErrorBuffer);
	}
    }
    else {
	if ((*fp = DBF_Open(filename, mode)) == NULL) {
	    sprintf(ErrorBuffer, "could not open data file '%s'", filename);
	    _RuntimeError(ErrorBuffer);
	}
    }
}

void _CloseSequentialFile(FILE * fp)
{
    fclose(fp);
}

void _CloseRelativeFile(DBF_FILE ** fp)
{
    DBF_Close(*fp);
    *fp = NULL;
}

void _CloseIndexedFile(DBF_FILE ** fpd)
{
    DBF_Close(*fpd);
    *fpd = NULL;
}

void _SeekRelativeFile(DBF_FILE * fp, long int record)
{
    DBF_SeekToRecord(fp, record);
}

void _SeekIndexedFile(DBF_FILE * fp, NDX_FILE * fpd, const char *key)
{
int index;
    index = NDX_LookupKey (fpd, key);
    if (index >= 0)
	DBF_SeekToRecord(fp, index);
}

void _OpenIndexFile(NDX_FILE ** ifd, const char *filename,
		    const char *sig, const char *mode)
{
char ErrorBuffer[80];
DBF_Field *Fields;
int nFields;

    if (mode[0] == 'w' && access (filename,0) == 0)
	unlink (filename);

    /* If the file does not exist, use Create call */
    if (access(filename, 0) != 0) {
	/* Cannot open file for reading if it does not exist */
	if (mode[0] == 'r') {
	    /* Make it runtime error for now. Later change to 
	    ** setting the status variable (how?) and gracefully leaving. 
	    */
	    sprintf(ErrorBuffer, "file '%s' does not exist", filename);
	    _RuntimeError(ErrorBuffer);
	}
	Fields = _SigToFields (sig, &nFields);
	if ((*ifd = NDX_Create (filename, Fields->Name, 
	    	Fields->Type, Fields->FieldLength)) == NULL) 
	{
	    sprintf(ErrorBuffer, "could not create data file '%s'", filename);
	    _RuntimeError(ErrorBuffer);
	}
	free (Fields);
    }
    else {
	if ((*ifd = NDX_Open(filename, mode)) == NULL) {
	    sprintf(ErrorBuffer, "could not open data file '%s'", filename);
	    _RuntimeError(ErrorBuffer);
	}
    }
}

void _CloseIndexFile (NDX_FILE ** ifd)
{
    NDX_Close (ifd);
}

void _EnableCurses (void)
{
    __curses_enabled = 1;
}

static int InitColorPair (int fore, int back)
{
#ifndef NOCURSES
    short i, oldfore, oldback;
    for (i = 1; i < COLOR_PAIRS; ++ i) {
	if (pair_content (i, &oldfore, &oldback) != 0) {
	    if (oldfore == fore && oldback == back)
		return (i);
	}
	else
	    break;
    }
    init_pair (i, fore, back);
    return (i);
#endif
}

static void EraseScreen (void)
{
#ifndef NOCURSES
    int x, y;
    for (y = 0; y < LINES; ++ y)
	for (x = 0; x < COLS; ++ x)
	    mvaddch (y, x, ' ');
#endif
}

void _DisplayScreen (const _SScreenField* fields, int screenSize)
{
#ifndef NOCURSES
    int i, colorPair;
    for (i = 0; i < screenSize; ++ i) {
	if (fields[i].column >= 0)
	    move (fields[i].line, fields[i].column);
	if (has_colors() && fields[i].foreground >= 0) {
	    colorPair = InitColorPair (fields[i].foreground, fields[i].background);
	    attron (COLOR_PAIR (colorPair));
	}
	if (fields[i].blankScreen)
	    EraseScreen();
	if (fields[i].data.cval == NULL)
	    continue;
	if (fields[i].picture == NULL || fields[i].dataType == _dt_Null)
	    addstr (fields[i].data.cval);
	else if (fields[i].dataType == _dt_String)
	    addstr (fields[i].data.cval);
	else if (fields[i].dataType == _dt_Float)
	    addstr (_FloatToString (*fields[i].data.fval, fields[i].picture));
	else if (fields[i].dataType == _dt_Integer)
	    addstr (_IntegerToString (*fields[i].data.ival, fields[i].picture));
	else
	    _RuntimeError ("Invalid field type");
    }
    refresh();
#endif
}

void __AddCharacterToField (_SScreenField* pField, int position, int c)
{
char* pBuffer;
size_t buflen, piclen;
char picAtPos;

    picAtPos = pField->picture[position];

    if (pField->dataType == _dt_String)
	pBuffer = pField->data.cval;
    else if (pField->dataType == _dt_Float)
	pBuffer = _FloatToString (*pField->data.fval, pField->picture);
    else
	pBuffer = _IntegerToString (*pField->data.ival, pField->picture);

    buflen = strlen (pBuffer);
    piclen = strlen (pField->picture);

    /* Filter out nonprintable characters */
    if (picAtPos == 'X' && !isalnum(c))
	return;
    /* Force numbers at numeric positions */
    if (picAtPos == '9' && !isdigit(c) && c != '-')
	return;
    /* Don't insert anything if not an editable position */
    if (picAtPos != 'X' && picAtPos != '9')
	return;
    /* Don't insert if there is no space */
    if (piclen <= buflen)
	return;

    memmove (pBuffer + position + 1, pBuffer + position, buflen - position);
    pBuffer [buflen + 1] = 0;

    pBuffer [position] = c;

    if (pField->dataType == _dt_Float)
	*pField->data.fval = atof (pBuffer);
    else if (pField->dataType == _dt_Integer)
	*pField->data.ival = atoi (pBuffer);
}

void __DeleteCharacterInField (_SScreenField* pField, int position)
{
char* pBuffer;
size_t buflen, piclen;
char picAtPos;
int i;

    picAtPos = pField->picture[position];

    if (pField->dataType == _dt_String)
	pBuffer = pField->data.cval;
    else if (pField->dataType == _dt_Float)
	pBuffer = _FloatToString (*pField->data.fval, pField->picture);
    else
	pBuffer = _IntegerToString (*pField->data.ival, pField->picture);

    buflen = strlen (pBuffer);
    piclen = strlen (pField->picture);

    if (position < buflen)
	for (i = position; i < buflen - 1; ++ i)
	    if (pField->picture[i] == pField->picture[i + 1])	/* Only copy same-type characters */
		pBuffer[i] = pBuffer[i + 1];

    if (pField->dataType == _dt_Float)
	*pField->data.fval = atof (pBuffer);
    else if (pField->dataType == _dt_Integer)
	*pField->data.ival = atoi (pBuffer);
}

void _AcceptScreen (_SScreenField* fields, int screenSize)
{
#ifndef NOCURSES
int maxFieldPos = 0;
int curFieldPos = 0;
int curField = 0;
int key = 0;

    cbreak();
    noecho();
    keypad (stdscr, TRUE);

    do {
	_DisplayScreen (fields, screenSize);
	key = mvgetch (fields[curField].line, fields[curField].column + curFieldPos);
	switch (key) {
	    case KEY_LEFT:
		if (curFieldPos > 0)
		    -- curFieldPos;
		break;
	    case KEY_RIGHT:
		if (curFieldPos < maxFieldPos)
		    ++ curFieldPos;
		break;
	    case KEY_DOWN:
	    case 0x09:
		curField = (curField + 1) % screenSize;
		curFieldPos = 0;
		if (fields[curField].picture != NULL)
		    maxFieldPos = strlen (fields[curField].picture);
		else
		    maxFieldPos = 0;
		break;
	    case KEY_ENTER:
		if (curField < screenSize - 1)
		    ++ curField;
		curFieldPos = 0;
		break;
	    case KEY_UP:
		if (curField == 0)
		    curField = screenSize - 1;
		else
		    -- curField;
		curFieldPos = 0;
		break;
	    case 0x014A:	// Del key
		if (fields[curField].dataType != _dt_Null)
		    __DeleteCharacterInField (&fields[curField], curFieldPos);
		break;
	    default:
		if (fields[curField].dataType != _dt_Null) {
		    if (isprint (key) && fields[curField].picture != NULL && curFieldPos < maxFieldPos) {
			__AddCharacterToField (&fields[curField], curFieldPos, key);
			++ curFieldPos;
		    }
		}
		break;
	}
    }
    while (key != KEY_ENTER);
#endif
}

