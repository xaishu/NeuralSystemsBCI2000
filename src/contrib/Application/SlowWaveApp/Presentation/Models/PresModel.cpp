/////////////////////////////////////////////////////////////////////////////
//
// File: PresModel.cpp
//
// Date: Oct 15, 2001
//
// Author: Juergen Mellinger
//
// Description:
//
// Changes: Feb 5, 2004, jm: Created file PresModel.cpp, introduced
//          factorization of ProcessTaskManager() into public Reset() and
//          NextTarget() functions.
//
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
//////////////////////////////////////////////////////////////////////////////
#ifdef __BORLANDC__
#include "PCHIncludes.h"
#pragma hdrstop
#endif // __BORLANDC__

#include "PresModel.h"

#include "StateAccessor.h"
#include "Views/PresView.h"
#include <cassert>

TPresModel::TPresModel( ParamList   *inParamList )
: curParamList( inParamList )
{
    assert( curParamList != NULL );
}

TPresModel::~TPresModel()
{
    for( std::list< TPresView* >::iterator i = views.begin();
                i != views.end(); ++i )
        delete *i;
}

TPresError
TPresModel::Initialize( ParamList   *inParamList, TPresBroadcaster *inBroadcaster )
{
    DoCleanup();
    for( std::list< TPresView* >::iterator i = views.begin();
                i != views.end(); ++i )
        delete *i;
    return DoInitialize( inParamList, inBroadcaster );
}


