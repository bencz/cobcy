/* symconst.h
**
**	Defines a constant data type. This never enters the symbol table.
*/

#ifndef __SYMCONST_H
#define __SYMCONST_H

#include <mdefs.h>
#include <streamab.h>

#include "semtypes.h"

class CobolConstant : public Streamable {
protected:
    union {
        char *		cval;
	long int	ival;
	double		fval;
    }				data;
    enum {
        CC_Undefined,
        CC_String,
	CC_Integer,
	CC_Float
    }				CurKind;

public:
				CobolConstant (void);
				CobolConstant (char* cval);
				CobolConstant (StackEntry* cval);
				CobolConstant (long int ival);
				CobolConstant (double fval);
    virtual		       ~CobolConstant (void);
    CobolConstant&		operator= (char * cval);
    CobolConstant&		operator= (long int ival);
    CobolConstant&		operator= (double fval);
    CobolConstant&		operator= (StackEntry * se);
    virtual void		WriteTextStream (ostream& os);
    void			GenWrite (ostream& os, char * stream);
    inline BOOL			IsNumeric (void) const;
    inline BOOL			IsInteger (void) const;
};

/*---------------------------------------------------------------------------*/

inline BOOL CobolConstant :: IsNumeric (void) const
{
    return (CurKind == CC_Integer || CurKind == CC_Float);
}

inline BOOL CobolConstant :: IsInteger (void) const
{
    return (CurKind == CC_Integer);
}

#endif

