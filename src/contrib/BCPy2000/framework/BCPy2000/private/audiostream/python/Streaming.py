import os,sys
import time
import numpy
import scipy.signal
import SigTools
import WavTools
import BCI2000Tools.DataFiles  # adds self.load and self.dump methods

def trapseq_onreset(self, discard=None, remember=None, persistence=None, detrend=None, bci=None):
	if discard == None: discard = getattr(self, 'discard', 0)
	if remember == None: remember = getattr(self, 'remember', 10)
	if persistence == None: persistence = getattr(self, 'persistence', 1.0)
	if detrend == None: detrend = getattr(self, 'detrend', None)
	if bci == None: bci = getattr(self, 'bci', None)
			
	self.discard = discard
	self.remember = remember   # keep how many individual epochs in memory at any one time?
	self.persistence = persistence  # for running mean
	self.bci = bci
	self.detrend = detrend  # None,  'constant' or 'linear'

	self.avg = SigTools.running_mean(persistence=self.persistence)
	self.ndelivered = 0
	self.recent = []
	
def trapseq_oncollect(self, x, n):
	if self.detrend != None: x = scipy.signal.detrend(x, axis=1, type=self.detrend)		
	self.ndelivered += 1
	if self.ndelivered > self.discard: self.avg += x   # keep a running average
	self.recent.append(x)
	while len(self.recent) > self.remember: self.recent.pop(0)
	if self.bci != None: self.bci.UpdatePrediction()

class BciTrapSequence(SigTools.TrapSequence):
	onreset = trapseq_onreset
	oncollect = trapseq_oncollect

class BciTriggerlessTrapSequence(SigTools.TriggerlessTrapSequence):
	onreset = trapseq_onreset
	oncollect = trapseq_oncollect

#################################################################
#################################################################

class BciSignalProcessing(BciGenericSignalProcessing):	

	#############################################################

	def Description(self):
		return "binary classification of difference of L-R ERPs triggered by stereo AUX channels"

	#############################################################

	def Construct(self):
		parameters = [
			"PythonSig:Streams int       NumberOfStreams=                          2                      2     2 % // ",
			"PythonSig:Streams matrix    StreamStimuli=                    2 { Standard Target } % % % %  %     % % // ",
			"PythonSig:Streams floatlist PeriodMsec=                         2   500   500              500     0 % // ",
			"PythonSig:Streams floatlist OffsetMsec=                         2     0   250              500     0 % // ",
			"PythonSig:Streams floatlist TargetProbability=                  2     0.2   0.2              0.2   0 1 // ",
			"PythonSig:Streams intlist   InitialStandards=                   2     2       2              3     0 % // how many stimuli at the beginning of each stream are guaranteed to be standards",
			"PythonSig:Streams int       SurroundSoundTrigger=                     0                      0     0 1 // if checked, deliver the trigger signal in sound channels 3 and 4 (boolean)",
			"PythonSig:Streams int       DirectSound=                              1                      0     0 1 // use DirectSound interface or not? (boolean)",
			
			"PythonSig:Epoch   float     EpochDurationMsec=                      600                    600   100 % // ",
			"PythonSig:Epoch   floatlist EpochLowerBoundMsec=                2   100     100            100     0 % // after springing, each ERP trap will not spring again for this many milliseconds",
			"PythonSig:Epoch   list      TriggerChannels=                    2  LAUD    RAUD              %     % % // ",
			"PythonSig:Epoch   floatlist TriggerThreshold=                   2     0.1     0.1            %     0 % // ",
			"PythonSig:Epoch   float     TriggerHPCutoff=                          0.0                    0.0   0 % // ",
			"PythonSig:Epoch   int       TriggerHPOrder=                           4                      4     0 % // ",
			"PythonSig:Epoch   floatlist ERPFilterFreqHz=                    2     0.1     8              %     0 % // lower and upper frequencies of bandpass filter for ERP feature set",
			"PythonSig:Epoch   int       ERPFilterOrder=                           8                      8     0 % // order of bandpass filter for ERP feature set",
			"PythonSig:Epoch   intlist   DiscardEpochs=                      2     2       2              2     0 % // for classification, discard this many epochs at the beginning",
			"PythonSig:Epoch   int       DetrendEpochs=                            2                      2     0 2 // Detrend data? 0: no, 1: mean, 2: linear (enumeration)",
			"PythonSig:Epoch   float     ERPClassifierBias=                        0.0                    0.0   % % // ",
			"PythonSig:Epoch   matrix    ERPClassifierWeights=             0 0                            %     % % // ",
			"PythonSig:Epoch   int       DiffFeatureSets=                          1                      1     0 1 // for 2-stream designs, whether to use the difference between the two feature sets (boolean)",
			"PythonSig:Epoch   int       SaveTrainingData=                         0                      0     0 1 // dump epochs to .pk file (boolean)",
			
			"PythonSig:Control float     EpochAveragingPersistence=                1.0                    1.0   0 % // persistence parameter for the running average of ERPs",
			"PythonSig:Control float     ControlFilterCutoffHz=                    0                      0     0 % // output low-pass cutoff in Hz (0 to disable)",
			"PythonSig:Control int       ControlFilterOrder=                       8                      8     0 % // ",
		]
		states = [
			"StreamingRequired  1 0 0 0",
			"StimulusCode       4 0 0 0",
			"StimulusVariant    2 0 0 0",
			"StimulusType       1 0 0 0",
		]
		return (parameters, states)

	#############################################################

	def Preflight(self, sigprops):
		self.eegfs = self.nominal['SamplesPerSecond']
		self.nstreams = self.params['NumberOfStreams'].val
		self.epoch_samples = SigTools.msec2samples(self.params['EpochDurationMsec'], self.eegfs)
		self.takediff = self.params['DiffFeatureSets'].val
		self.discard = self.params['DiscardEpochs'].val
		if self.takediff and self.nstreams != 2: raise EndUserError, "DiffFeatureSets parameter cannot be set unless NumberOfStreams is 2"
		
		band  = self.params['ERPFilterFreqHz'].val
		order = self.params['ERPFilterOrder'].val
		if len(band) == 0 or order == 0: self.bandpass = None
		else: self.bandpass = SigTools.causalfilter(type='bandpass', order=order, freq_hz=band, samplingfreq_hz=self.eegfs)
		
		trigch = self.params['TriggerChannels'].val
		use_trigger = len(trigch) != 0
		trigHPcutoff = self.params['TriggerHPCutoff'].val
		trigHPorder  = self.params['TriggerHPOrder'].val
		if use_trigger and trigHPorder and trigHPcutoff:
			self.triggerfilter = SigTools.causalfilter(type='highpass', order=trigHPorder, freq_hz=trigHPcutoff, samplingfreq_hz=self.eegfs)
		else:
			self.triggerfilter = None
		
		# check length = number of streams
		perStreamParams = ['EpochLowerBoundMsec', 'DiscardEpochs', 'PeriodMsec', 'OffsetMsec', 'InitialStandards', 'TargetProbability']
		if use_trigger: perStreamParams += ['TriggerChannels', 'TriggerThreshold']
		for paramname in perStreamParams:
			v = self.params[paramname].val
			if len(v) != self.nstreams: raise EndUserError, "number of elements of %s parameter must match NumberOfStreams (=%d)" % (paramname, self.nstreams)

		self.trapgap = [SigTools.msec2samples(x, self.eegfs) for x in self.params['EpochLowerBoundMsec'].val]
		
		chn = self.inchannels()
		if False in [isinstance(x, int) for x in trigch]:
			nf = filter(lambda x: not str(x) in chn, trigch) 
			if len(nf): raise EndUserError, "failed to find %s in module's list of input channel names" % str(nf)
			self.trigchan = [chn.index(str(x)) for x in trigch]
		else:
			nf = [x for x in trigch if x < 1 or x > len(chn) or x != round(x)] 
			if len(nf): raise EndUserError, "illegal channel(s): %s" % str(nf)
			self.trigchan = [x-1 for x in trigch]
				
		if self.trigchan == []:
			raise RuntimeError("triggerlessness not yet implemented")
		
		self.otherchan = [chn.index(x) for x in ['VMRK'] if x in chn]
		self.sigchan = list(set(range(len(chn))).difference(self.trigchan + self.otherchan))
		self.trigthresh = numpy.asarray(self.params['TriggerThreshold'].val)
		
		self.weights = self.params['ERPClassifierWeights'].val
		self.weights2 = numpy.multiply(self.weights, self.weights)
		if len(self.weights) == 0:
			self.weights = self.weights2 = None
		else:
			if self.weights.shape[0] != len(chn): raise EndUserError, "ERPClassifierWeights should have %d rows to match the number of channels" % len(chn)
			if self.weights.shape[1] != self.epoch_samples: raise EndUserError, "ERPClassifierWeights should have %d cols to match the number of samples in an epoch" % self.epoch_samples
		self.bias = float(self.params['ERPClassifierBias'])
				
		if 1:  # turn back off in order to visualize temporally-filtered output
			self.out_signal_props['ChannelLabels']         =  ['prediction']
			self.out_signal_props['ElementLabels']         =  ['1']
			self.out_signal_props['ElementUnit']['Gain']   =  self.nominal['SecondsPerPacket']
			self.out_signal_props['ElementUnit']['RawMin'] =  0.0
			self.out_signal_props['ElementUnit']['RawMax'] =  self.nominal['PacketsPerSecond'] * 10.0 - 1.0
			self.out_signal_props['ValueUnit']['Gain']     =  1.0
			self.out_signal_props['ValueUnit']['RawMin']   = -2.0
			self.out_signal_props['ValueUnit']['RawMax']   = +2.0
		
		self.persistence = float(self.params['EpochAveragingPersistence'])
		ctrllp = float(self.params['ControlFilterCutoffHz'])
		ctrlorder = int(self.params['ControlFilterOrder'])
		if ctrllp and ctrlorder:
			self.controlfilter = SigTools.causalfilter(type='lowpass', order=ctrlorder, freq_hz=ctrllp, samplingfreq_hz=self.eegfs)
		else:
			self.controlfilter = None
		

		self.offsets = []
		self.periods = []
		for istream in range(self.nstreams):
			self.periods.append( self.RoundToPackets('PeriodMsec', istream, minval=1) )
			self.offsets.append( self.RoundToPackets('OffsetMsec', istream, minval=0) )
			p = self.params['TargetProbability'].val[istream]
			if p < 0.0 or p > 1.0: raise EndUserError('TargetProbability values must be in the range [0,1]')
			p = self.params['InitialStandards'].val[istream]
			if p < 0.0 or p != round(p): raise EndUserError('InitialStandards values must be integers >= 0')
			
		if int(self.params['DirectSound']):
			import DirectSoundInterface
		elif 'DirectSoundInterface' in sys.modules and sys.modules['DirectSoundInterface'].loaded:
			raise EndUserError, "once turned on, the DirectSound setting cannot be turned off without restarting BCI2000"
			
		stim = numpy.array(self.params['StreamStimuli'])
		if stim.shape[1] != 2: raise EndUserError("StreamStimuli parameter must have 2 columns (Standard and Target)")
		if stim.shape[0] != self.nstreams: raise EndUserError("StreamStimuli parameter must have one row per streams (NumberOfStreams = %d)" % self.nstreams)
		self.surround = int(self.params['SurroundSoundTrigger'])
		self.standards = [self.prepwav(filename,istream) for istream,filename in enumerate(stim[:,0])]
		self.targets   = [self.prepwav(filename,istream) for istream,filename in enumerate(stim[:,1])]		
		for p in self.standards + self.targets:
			while p.playing: time.sleep(0.001)
			p.vol = 1.0
		
	#############################################################
	
	def RoundToPackets(self, paramname, index=None, minval=0):
		
		msec = self.params[paramname].val
		desc = '%s parameter' % paramname
		if index != None:
			msec = msec[index];
			desc = 'element %d of %s' % (index+1, desc)
		packets = max(minval, SigTools.msec2samples(msec, self.nominal.PacketsPerSecond))
		msec_rounded = SigTools.samples2msec(packets, self.nominal.PacketsPerSecond)
		if abs(msec - msec_rounded) > 0.5: raise EndUserError("at a SampleBlock duration of %g msec, %s gets rounded from %g to %g. To remove this error, specify this as %g to begin with." % (1000.0*self.nominal.SecondsPerPacket, desc, msec, msec_rounded, round(msec_rounded)))
		return packets
		
	#############################################################
	
	def prepwav(self, filename, istream):
		try: w = WavTools.wav(filename)
		except IOError: raise EndUserError("failed to load '%s' as a wav file" % filename)
		
		if w.channels() != 1: raise EndUserError("StreamStimuli wav files must be single-channel: found %d channels in %s" % (w.channels(), w.filename))

		chmask = [i==istream for i in range(self.nstreams)]
		w *= chmask
		if self.surround:
			f = self.eegfs/4.0
			d = 100
			d = SigTools.samples2msec(max(1, SigTools.msec2samples(d, f)), f)
			w &= SigTools.wavegen(freq_hz=f, duration_msec=d, container=w[:,0]*0, waveform=numpy.sin) * chmask
					
		p = WavTools.player(w)
		p.vol = 0.0
		p.play()
		return p
		
	#############################################################

	def Initialize(self, indim, outdim):
		
		self.seq = []
		for istream in xrange(self.nstreams):
			if len(self.trigchan): seqclass = BciTrapSequence
			else: seqclass = BciTriggerlessTrapSequence
			s = seqclass (
					nsamp=self.epoch_samples,
					mingap=self.trapgap[istream],
					trigger_channel=self.trigchan[istream],
					trigger_threshold=self.trigthresh[istream],
					trigger_processing=self.ProcessTrigger,
					discard=self.discard[istream],
					remember=10,
					persistence=self.persistence,
					detrend={0:None, 1:'constant', 2:'linear'}.get(int(self.params['DetrendEpochs'])),
					bci=self,
			)
			self.seq.append(s)

		self.prediction, self.prediction_se = 0.0, 1.0
		
		self.Last10SecondsTrigger = SigTools.Buffering.trap(nsamples=SigTools.msec2samples(10000, self.eegfs), nchannels=2, leaky=True)
			
		self.saving = int(self.params['SaveTrainingData']) != 0       # are we gathering and dumping preprocessed data?
	
	#############################################################
		
	def StartRun(self):
		
		self.x = []
		self.y = []
		self.nbeats = [0] * self.nstreams
		self.TriggerTrouble = [None] * self.nstreams
		
		if self.saving:
			self.dump(channels=list(self.inchannels()), fs=self.nominal['SamplesPerSecond'])
				
	#############################################################
	
	def StopRun(self):
		
		if self.saving:
			self.dump('flush') # the newer, python way
			# and now, the older, matlab way (TODO: remove this)  :
			if len(self.x) and self.x[0] != None:
				if not isinstance(self.data_file, str): raise RuntimeError,'StopRun failed in PythonSig because self.data_file is not valid'
				x = [numpy.matrix(numpy.asarray(xi).T.flatten()).A for xi in self.x]
				xsiz = numpy.matrix((len(x),) + self.x[0].shape, dtype=numpy.float64)
				unexpected = [y-1 not in range(self.nstreams) for y in self.y]
				if any(unexpected): print "WARNING: unexpected labels: self.y = ",self.y
				a = {
					'x':numpy.concatenate(x, axis=0),
					'xsiz':xsiz,
					'y':numpy.matrix(self.y).T,
					'channels':'\n'.join(self.inchannels()),
					'fs':self.eegfs,
				}
				SigTools.savemat(self.data_file.replace('.dat', '_features.mat'), a)
		else:
			print "features not saved (traps not verified)"
		
		p = getattr(self, 'player', None)
		if p: p.stop()
				
	#############################################################

	def Process(self, sig):
		
		if self.bandpass != None:
			sig[self.sigchan,:] = self.bandpass(sig[self.sigchan,:], axis=1)
		if self.triggerfilter != None:
			sig[self.trigchan,:] = self.triggerfilter(sig[self.trigchan,:], axis=1)				
			
		self.Last10SecondsTrigger.process(sig[self.trigchan,:])
				
		starting = self.changed('StreamingRequired', fromVals=0)
		presenting = self.states['StreamingRequired'] != 0
		if starting: self.remember('streaming')
		resetStates = False
		stimDelivered = 0
		
		counts = []
		np = self.since('streaming')['packets']
		for istream in range(self.nstreams):
									
			if starting:
				self.nbeats[istream] = 0
				
			if presenting:
				count = (np - self.offsets[istream]) % self.periods[istream]
				counts.append(count)
				if count == 0:
					if self.nbeats[istream] < self.params['InitialStandards'].val[istream]:
						self.states['StimulusVariant'] = 1
					else:
						self.states['StimulusVariant'] = 1 + int(numpy.random.rand() < self.params['TargetProbability'].val[istream])
					if self.states['StimulusVariant'] == 2:
						stim = self.targets[istream]
					else:
						stim = self.standards[istream]
					self.states['StimulusCode'] = istream + 1
					if self.states.get('TargetStream', 0) == istream + 1: self.states['StimulusType'] = 1
					stim.play()
					self.nbeats[istream] += 1
					stimDelivered += 1
				if count == 1:
					resetStates = True
					
		if (resetStates or not presenting) and not stimDelivered:
			self.states['StimulusCode'] = 0
			self.states['StimulusVariant'] = 0
			self.states['StimulusType'] = 0
			
		if stimDelivered > 1:
			self.debug("stimulus collisions", counts=counts, packetsSinceStartOfStreaming=np, nbeats=list(self.nbeats))


		wasWaiting = False
		stillWaiting = False
		
		for istream, seq in enumerate(self.seq):
			if starting:
				seq.reset()
			else:
				if len(self.trigchan) == 0 and counts[istream] == 0: kwargs = {'event_offset':self.nominal.SamplesPerPacket}
				else: kwargs = {}
				waiting = seq.ndelivered < self.nbeats[istream]
				wasWaiting |= waiting
				if presenting or waiting: seq.process(sig, **kwargs)
				waiting = seq.ndelivered < self.nbeats[istream]
				stillWaiting |= waiting
			
		if not wasWaiting:
			self.prediction, self.prediction_se = 0.0, 1.0
		
		if wasWaiting and not stillWaiting and not presenting:
			for istream, seq in enumerate(self.seq):
				if seq.ndelivered != self.nbeats[istream]:
					self.TriggerTrouble[istream] = "expected %d beats in stream %d, but trapped %d" % (self.nbeats[istream], istream+1, seq.ndelivered)
				elif True in [t.collected() and not t.full() for t in seq.active]:
					self.TriggerTrouble[istream] = "not all traps are full in stream %d" % (istream+1)
				del self.seq[istream].active[:]
			
			trouble = '\n'.join( [''] + [x for x in self.TriggerTrouble if x != None] )
			if int( self.params['SaveTrainingData'] ):
				if len(trouble): raise RuntimeError(trouble)
			else:
				print 'warning: traps not verified, so features will not be saved' + trouble.replace('\n', '\n   ')
			
			xi = self.UpdatePrediction()
			yi = self.states.get('TargetStream', 0)
				
			if xi != None:
				self.dump(x=xi,y=yi) # the new python way
				self.x.append(xi) # the old matlab way
				self.y.append(yi) # the old matlab way
			
		control_signal = self.prediction / self.prediction_se
		if self.controlfilter: control_signal = self.controlfilter.apply([[control_signal]])
				
		if max(self.out_signal_dim) == 1: return control_signal
		else: return sig
								
	#############################################################
	
	def UpdatePrediction(self):

		nmin = min([s.avg.n for s in self.seq])
		if nmin < 1: return None

		#if nmin < 2: return None
		#estvar = [s.avg.v_unbiased / s.avg.n for s in self.seq]
		
		if self.takediff and self.nstreams == 2:
			xi = self.seq[1].avg.m - self.seq[0].avg.m
			#vi = estvar[1] + estvar[0]
		else:
			raise RuntimeError,"multiple feature sets not yet implemented"
		
		if self.weights == None:
			self.prediction = 0.0
			self.prediction_se = 1.0
		else:
			self.prediction    = sum(numpy.multiply(self.weights,  xi).flat) + self.bias
			#self.prediction_se = sum(numpy.multiply(self.weights2, vi).flat) ** 0.5
		
		return xi
	
	#############################################################

	def ProcessTrigger(self, tr):
		return numpy.abs(tr)

	#################################################################
	
	def classify(self, runs=None, xtn='.pk', C=(1e-0,1e-2,1e-4,1e-6), gamma=0.0, rebias=False, save=True, plotopt=False, **kwargs):
		from BCI2000Tools.Classification import ClassifyERPs
		files = self.find_data_files(xtn=xtn, runs=runs)
		u,c = ClassifyERPs(files, C=C, gamma=gamma, rebias=rebias, save=save, **kwargs)
		import SigTools
		u.channels = SigTools.ChannelSet(u.channels)
		
		print u.description
		return u,c
		
	#############################################################

	def foo(self, istream=0):
		"""
		To plot multiple epochs of the first channel (read from the last .dat file and cut
		up according to stream 0 triggers)  try this:
		    z = self.foo(istream=0)
		    z.plot(z.collated[0])  # channel index 0:  probably the stream-0 trigger itself.
		but don't expect the y tick-labels to make sense
		"""###
		from SigTools import sdict
		from BCI2000Tools.FileReader import bcistream
		b = bcistream(self.data_file)
		print b
		sig,states=b.decode('all')
		epochs = self.seq[istream].recent
		collated = []
		for i in xrange(epochs[0].shape[0]):
			collated.append(numpy.concatenate([numpy.asmatrix(x[i,:]) for x in epochs], axis=0))
		return sdict({'b':b, 'sig':sig, 'states':sdict(states), 'epochs':epochs, 'collated':collated, 'plot':b.plotsig})
	
	def plottrap(self, istream=0, itrap=-1):
		"""
		Plot all trigger channels in (by default) the last-trapped epoch, where an epoch is defined
		according to the trigger for stream <istream>.		
		To plot the last-but-one epoch, use itrap=-2,  and so on.  It's most sensible to use negative
		indices to work backwards from the end, since the number of epochs remembered at any one time
		is somewhat arbitrary.
		"""###
		delivered = self.seq[istream].recent
		active = [t.read() for t in self.seq[istream].active if t.collected()] # non-empty, undelivered
		y = (delivered + active)[itrap]
		
		y = y[self.trigchan, :].T
		t = SigTools.samples2msec(numpy.arange(y.shape[0]), self.eegfs)
		SigTools.plot(t, y)
		import pylab
		pylab.grid()
	
	def PlotTrigger(self, msg=None, stream=None):
		"""
		Plot the last 10 seconds' worth of (possibly filtered) uncut trigger signals,
		together with the current trigger thresholds.
		"""###
		import pylab
		
		buffers = [self.Last10SecondsTrigger]
		for fig,buf in enumerate(buffers):
			pylab.figure(fig+1)
			for i,trig in enumerate(buf.read()):
				tt = float(self.params.TriggerThreshold[i])
				msec = SigTools.samples2msec(numpy.arange(trig.shape[-1]), self.eegfs)
				pylab.subplot(self.nstreams, 1, i+1)
				SigTools.plot(msec.T, trig.T)
				if self.TriggerTrouble[i]: pylab.title(self.TriggerTrouble[i])
				pylab.plot(pylab.xlim(), [tt,tt])
				pylab.plot(pylab.xlim(), [-tt,-tt])
				pylab.xlim([0, msec[-1]])
				pylab.grid('on')
			pylab.draw()
					
#################################################################
#################################################################