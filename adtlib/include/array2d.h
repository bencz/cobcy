/* array2d.h
**
**	Defines a 2-dimensional array template. Abstract and simple.
** If you want [] to check bounds, define CHECK_BOUNDS.
**
** includes:
**	mdefs.h		- for WORD
**	set.h		- base class
*/

#ifndef __ARRAY2D_H
#define __ARRAY2D_H

#include <mdefs.h>	 
#include <set.h>
#include <stdlib.h>
			  
typedef SetSizeType			Array2dSizeType;

template <class Array2dEl>
class Array2d : public Set<Array2dEl> {
private:
    Array2dSizeType			m_Width;
    Array2dSizeType			m_Height;
    
public:
    INLINE_CTOR			Array2d (WORD h = 0, WORD w = 0);
    INLINE_CTOR			Array2d (const Array2d<Array2dEl>& AnArray2d);
    INLINE_CTOR			Array2d (const Array2dEl * AnArray2d, 
    					Array2dSizeType AHeight, 
					Array2dSizeType AWidth);
			
    inline BOOL		operator== (const Array2d<Array2dEl>& toCompare) const;
    inline virtual Array2dEl *	operator[] (Array2dSizeType index) const;
    inline Array2d<Array2dEl>&	operator= (const Array2d<Array2dEl>& toBe);

    inline virtual void			ReadBinaryStream (istream& is);
    inline virtual void			WriteBinaryStream (ostream& os);
    inline virtual void			Resize (WORD h, WORD w);

    inline Array2dSizeType		Width (void) const
						{ return (m_Width);};
    inline Array2dSizeType		Height (void) const
    						{ return (m_Height);};
};

/*--------------------------------------------------------------------------*/

template <class Array2dEl>
inline Array2d<Array2dEl> :: Array2d 
(WORD h, WORD w) 
 : Set<Array2dEl> (h * w)
{   
    m_Width = w;
    m_Height = h; 
}

template <class Array2dEl>
inline Array2d<Array2dEl> :: Array2d
(const Array2d<Array2dEl>& AnArray2d) : Set<Array2dEl> (AnArray2d)
{   
    m_Width = AArray2d.m_Width;
    m_Height = AArray2d.m_Height;	   
};

template <class Array2dEl>
inline Array2d<Array2dEl> :: Array2d 
(const Array2dEl * AnArray2d, Array2dSizeType AHeight, Array2dSizeType AWidth) 
 : Set<Array2dEl> (AnArray2d, AHeight * AWidth)
{ 	   
    m_Width = AWidth;
    m_Height = AHeight;	   
};
 
template <class Array2dEl>
inline BOOL Array2d<Array2dEl> :: operator== 
(const Array2d<Array2dEl>& toCompare) const
{    
Array2dSizeType i;

    if (toCompare.m_Height != m_Height || toCompare.m_Width != m_Width)
	return (FALSE);
	    
    for (i = 0; i < m_Size; ++ i)
	if (m_Data[i] != toCompare.m_Data[i])
	    return (FALSE);

    return (TRUE); 
}	       

template <class Array2dEl>
inline Array2dEl * Array2d<Array2dEl> :: operator[]
(WORD index) const
{
#ifdef CHECK_BOUNDS
    if (index < m_Height)
#endif
	return (&m_Data [index * m_Width]);
       
    cerr << "Array2d: index out of range.\n";
    exit (1);
}

template <class Array2dEl>
inline Array2d<Array2dEl>& Array2d<Array2dEl> :: operator= 
(const Array2d<Array2dEl>& toBe)
{	
Array2dSizeType i;
			    
    Array2d<Array2dEl> :: Resize (toBe.m_Height, toBe.m_Width);
    for (i = 0; i < m_Size; ++ i)
	m_Data[i] = toBe.m_Data[i];

    return (*this);
}
 
template <class Array2dEl>
inline void Array2d<Array2dEl> :: ReadBinaryStream
(istream& is)
{	    
Array2dSizeType row, col, SavedWidth = 0, SavedHeight = 0;
  						
    ReadWord (is, &SavedHeight);
    ReadWord (is, &SavedWidth);
    Array2d<Array2dEl> :: Resize (SavedHeight, SavedWidth);
    for (row = 0; row < m_Height; ++ row)
	for (col = 0; col < m_Width; ++ col)
	    ReadRaw (is, &m_Data [row * m_Width + col], sizeof(Array2dEl));
} 

template <class Array2dEl>
inline void Array2d<Array2dEl> :: WriteBinaryStream
(ostream& os)
{	    
Array2dSizeType row, col;
  				       
    WriteWord (os, &m_Height);
    WriteWord (os, &m_Width);
    for (row = 0; row < m_Height; ++ row)
	for (col = 0; col < m_Width; ++ col)
	    WriteRaw (os, &m_Data [row * m_Width + col], sizeof(Array2dEl));
} 
       
template <class Array2dEl>
inline void Array2d<Array2dEl> :: Resize
(WORD h, WORD w)
{		
Array2dEl * NewData;
Array2dSizeType row, col;
    
    NewData = new Array2dEl [h * w];

    for (row = 0; row < m_Height; ++ row)
	for (col = 0; col < m_Width; ++ col)
	    NewData [row * w + col] = m_Data [row * m_Width + col];
    m_Height = h;
    m_Width = w;
    m_Size = m_Height * m_Width;
    
    delete [] m_Data;
    m_Data = NewData;
}

#endif

