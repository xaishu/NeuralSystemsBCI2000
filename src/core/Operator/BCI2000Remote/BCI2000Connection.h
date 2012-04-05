//////////////////////////////////////////////////////////////////////
// $Id$
// Authors juergen.mellinger@uni-tuebingen.de
// Description: A wrapper class representing a telnet connection to
//   BCI2000. Does not depend on the BCI2000 framework except for
//   src/shared/utils/SockStream.
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
///////////////////////////////////////////////////////////////////////
#ifndef BCI2000_CONNECTION_H
#define BCI2000_CONNECTION_H

#include "SockStream.h"
#include <string>
#include <vector>

class BCI2000Connection
{
  static double DefaultTimeout() { return 5.0; } // s
  static const char* DefaultTelnetAddress() { return "localhost:3999"; }

 public:
  BCI2000Connection()
  : mTerminateOperator( false ),
    mWindowVisible( true ),
    mTimeout( DefaultTimeout() ),
    mTelnetAddress( DefaultTelnetAddress() )
  {}
  virtual ~BCI2000Connection() { Disconnect(); }

  // ==Properties==
  // The timeout to use for the connection to be established, and for a command to finish.
  double Timeout() const { return mTimeout; }
  BCI2000Connection& Timeout( double inTimeout ) { mTimeout = inTimeout; return *this; }
  // Path to operator module, or batch file. When empty, a connection to an already running
  // module is attempted. When a batch file is used, it must forward its command line options
  // to the operator module.
  const std::string& OperatorPath() const { return mOperatorPath; }
  BCI2000Connection& OperatorPath( const std::string s ) { mOperatorPath = s; return *this; }
  // Properties of the main BCI2000 window.
  bool WindowVisible() const { return mWindowVisible; }
  BCI2000Connection& WindowVisible( bool visible );
  const std::string& WindowTitle() const { return mWindowTitle; }
  BCI2000Connection& WindowTitle( const std::string& );
  // Network address to connect to, in form of <ip>:<port>. When OperatorPath is
  // nonempty, the IP address must be "localhost".
  const std::string& TelnetAddress() const { return mTelnetAddress; }
  BCI2000Connection& TelnetAddress( const std::string s ) { mTelnetAddress = s; return *this; }
  // Result or error message resulting from a method call.
  const std::string& Result() const { return mResult; }

  // ==Methods==
  // Connect to BCI2000. When OperatorPath is not empty, this starts up the
  // Operator module before connecting.
  bool Connect();
  // Connect to the same running BCI2000 instance that the argument object
  // is connected to.
  bool Connect( const BCI2000Connection& );
  // Disconnect from an existing connection. Terminates the running Operator module
  // if it was started by the previous Connect() call.
  bool Disconnect();
  // Execute a BCI2000 scripting command. If the command executed a shell
  // command, returns the command's exit code, and 0 otherwise.
  int Execute( const std::string& );

 private:
  BCI2000Connection( const BCI2000Connection& );
  BCI2000Connection& operator=( const BCI2000Connection& );

 private:
  bool StartExecutable( const std::string& path, const std::string& options );

 private:
  bool mTerminateOperator,
       mWindowVisible;
  double mTimeout;
  client_tcpsocket mSocket;
  sockstream mConnection;
  std::string mOperatorPath,
              mTelnetAddress,
              mWindowTitle;
 protected:
  std::string mResult;
};

#endif // BCI2000_CONNECTION_H