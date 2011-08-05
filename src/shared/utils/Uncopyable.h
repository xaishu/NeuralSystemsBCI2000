//////////////////////////////////////////////////////////////////////
// $Id$
// Author: juergen.mellinger@uni-tuebingen.de
// Description: Inherit from this class to forbid copying and
//   assignment between instances of your derived class.
//
// (C) 2000-2008, BCI2000 Project
///////////////////////////////////////////////////////////////////////
#ifndef UNCOPYABLE_H
#define UNCOPYABLE_H

class Uncopyable
{
 protected:
  Uncopyable() {}
  ~Uncopyable() {}

 private:
  Uncopyable( const Uncopyable& );
  Uncopyable& operator=( const Uncopyable& );
};

#endif // UNCOPYABLE_H
