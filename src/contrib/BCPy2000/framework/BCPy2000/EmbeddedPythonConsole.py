#   $Id$
#   
#   This file is part of the BCPy2000 framework, a Python framework for
#   implementing modules that run on top of the BCI2000 <http://bci2000.org/>
#   platform, for the purpose of realtime biosignal processing.
# 
#   Copyright (C) 2007-8  Thomas Schreiner, Jeremy Hill
#                         Christian Puzicha, Jason Farquhar
#
#   The BCPy2000 framework is free software: you can redistribute it
#   and/or modify it under the terms of the GNU General Public License
#   as published by the Free Software Foundation, either version 3 of
#   the License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
import sys, time

# Embedded calls don't provide a shell and thus no sys.argv, but IPython needs it to
# avoid a bug on importing.
if not hasattr(sys, 'argv'): sys.argv = [""]
import IPython

# If this is the very first time IPython is run, it will create a user _ipython
# directory if none existed before, and then <groan> wait for return to be pressed.
# We can't hack around this in the postinstall script because the user who installs
# might be different from the user who uses. The following trick (lifted from Laurent
# Dufrechou's IPython/gui/wx/ipshell_nonblocking.py) just replaces the console input
# function called by user_setup(). Hopefully there will be no side effects...
IPython.iplib.raw_input = lambda x:None

def Shell(): # call the returned instance to start the shell
	return IPython.Shell.IPShellEmbed(rc_override={})
	
def ReplaceStreams():
	# save previous stream objects
	global previous
	previous = {'stdin':sys.stdin, 'stdout':sys.stdout, 'stderr':sys.stderr}
	
	############################################################################
	# replace stdout
	############################################################################
	enc = 'cp437'
	try:
		# In its later versions, pyreadline.unicode_helper.pyreadline_codepage is set
		# from sys.stdout.encoding *once* when pyreadline.unicode_helper is imported
		# for the first time. If sys.stdout changes, the codepage setting doesn't get
		# updated and this causes a bug. So we have to set it explictly. In earlier
		# versions, pyreadline.unicode_helper doesn't exist, so we have to just try:
		import pyreadline.unicode_helper
		pyreadline.unicode_helper.pyreadline_codepage = enc
	except:
		pass
	sys.stdout = IPython.genutils.Term.cout
	
	if previous['stdout'].fileno() > 0  and not previous['stdout'].isatty():
		sys.stdout = tee((sys.stdout, previous['stdout']))
	
	############################################################################
	# replace stdin
	############################################################################
	sys.stdin = IPython.rlineimpl._rl.rl
	sys.stdin.encoding = enc
	
	############################################################################
	# define traceback behaviour in case of exceptions (after the IPython shell
	# starts, this replacement would happen anyway, but it's required now in
	# case of syntax errors)
	############################################################################
	
	sys.excepthook = IPython.ultraTB.ColorTB()
	
	############################################################################
	# replace stderr (make this the last thing, in case errors occur above)
	############################################################################
	sys.stderr = sys.stdout


############################################################################
# helper class to deal with the simultaneous console + log-file case
############################################################################
class tee:
	def __init__(self, streamlist):
		self.streamlist = streamlist
	def flush(self):
		for x in self.streamlist:
			if hasattr(x, 'flush'): x.flush()
	def write(self, s):
		for x in self.streamlist:
			if hasattr(x, 'write'): x.write(s)
			if sys._getframe(1).f_code.co_name == 'raw_input': break
			# Ideally, it would be nice to include IPython's full input and output in the log.
			# However, the *only* thing that IPython seems to pump into the ordinary sys.stdout is
			# the annoying garbage-encoded prompt. If we can't have the meaningful stuff, let's at
			# least use this very nasty hack to cut out the garbage. IPython has its own sophisticated
			# input and output logging mechanism, but that's separate from the sys.stdout and
			# sys.stderr messages coming from the other threads, which is the main thing we want to
			# log here. If you want to log something interactively, issue a print statement.

############################################################################
# extremely annoying section to help stop the IPython thread from vying for
# input with pdb, when pdb is active in another thread
############################################################################

suspend_attrname = 'JEZ_WOULD_LIKE_YOU_TO_WAIT'
debugger_attrname = 'UNTIL_THIS_DEBUGGER_HAS_FINISHED'

def IPythonGiveWayToPDB(*pargs,**kwargs):
	global __IPYTHON__
	previous_chain = getattr(__IPYTHON__, suspend_attrname, None)
	while getattr(getattr(__IPYTHON__, debugger_attrname, None), 'stack', []) != []:
		time.sleep(0.010)
	if previous_chain != None: __IPYTHON__.hooks.generate_prompt.chain = previous_chain
	return __IPYTHON__.hooks.generate_prompt(0)

def SuspendIPython():
	global __IPYTHON__
	if hasattr(__IPYTHON__, suspend_attrname): return
	previous_chain = list(__IPYTHON__.hooks.generate_prompt.chain)
	setattr(__IPYTHON__, suspend_attrname, previous_chain)
	__IPYTHON__.set_hook('generate_prompt', IPythonGiveWayToPDB)

def ReleaseIPython():
	global __IPYTHON__
	if hasattr(__IPYTHON__, suspend_attrname): delattr(__IPYTHON__, suspend_attrname)
	
class Tracer (IPython.Debugger.Tracer):
	def __init__(self, *pargs, **kwargs):
		IPython.Debugger.Tracer.__init__(self, *pargs, **kwargs)
		global __IPYTHON__
		setattr(__IPYTHON__, debugger_attrname, self.debugger)
		
	def __call__(self):
		time.sleep(0.100)
		SuspendIPython() # Here we go. This is why we had to subclass the Tracer.
		self.debugger.set_trace(sys._getframe().f_back)

def WaitForShell(timeout=0.1):
	# The core calls this immediately after starting the shell, in order
	# to ensure that the shell is already running when it instantiates the
	# Tracer object. This makes a difference, according to the documentation
	# for IPython.Debugger.Tracer
	global __IPYTHON__
	t = time.time() + timeout
	while(time.time() < t):
		try: __IPYTHON__; return True
		except: time.sleep(0.010)
	return False

############################################################################
# code to be run on import
############################################################################

ReplaceStreams()
