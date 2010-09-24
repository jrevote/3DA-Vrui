/***********************************************************************
Environment-dependent part of Vrui virtual reality development toolkit.
Copyright (c) 2000-2005 Oliver Kreylos

This file is part of the Virtual Reality User Interface Library (Vrui).

The Virtual Reality User Interface Library is free software; you can
redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

The Virtual Reality User Interface Library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Virtual Reality User Interface Library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <Misc/File.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/Timer.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Threads/Thread.h>
#include <Threads/Barrier.h>
#include <Comm/MulticastPipeMultiplexer.h>
#include <Comm/MulticastPipe.h>
#include <Geometry/Point.h>
#include <Geometry/Plane.h>
#include <Geometry/OrthogonalTransformation.h>
#include <Geometry/AffineTransformation.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/GLValueCoders.h>
#include <GL/GLContextData.h>
#include <GL/GLThingManager.h>
#include <X11/keysym.h>
#include <GLMotif/Event.h>
#include <GLMotif/Popup.h>
#ifdef VRUI_USE_OPENAL
#include <AL/ALContextData.h>
#endif
#include <Vrui/InputDeviceManager.h>
#include <Vrui/InputDeviceAdapterMouse.h>
#include <Vrui/Viewer.h>
#include <Vrui/VRScreen.h>
#include <Vrui/VisletManager.h>
#include <Vrui/ViewSpecification.h>
#include <Vrui/VRWindow.h>

#include <Vrui/Vrui.Internal.h>

namespace Vrui {

namespace {

/***********************************
Workbench-specific global variables:
***********************************/

Misc::ConfigurationFile* vruiConfigFile=0;
char* vruiApplicationName=0;
int vruiNumWindows=0;
VRWindow** vruiWindows=0;
bool vruiWindowsMultithreaded=false;
Threads::Thread* vruiRenderingThreads=0;
Threads::Barrier vruiRenderingBarrier;
int vruiNumSoundContexts=0;
ALContextData** vruiSoundContexts=0;
Comm::MulticastPipeMultiplexer* vruiMultiplexer=0;
Comm::MulticastPipe* vruiPipe=0;
int vruiNumSlaves=0;
pid_t* vruiSlavePids=0;
int vruiSlaveArgc=0;
char** vruiSlaveArgv=0;
volatile bool vruiAsynchronousShutdown=false;

/*****************************************
Workbench-specific private Vrui functions:
*****************************************/

/* Signal handler to shut down Vrui if something goes wrong: */
void vruiTerminate(int)
	{
	/* Request an asynchronous shutdown: */
	vruiAsynchronousShutdown=true;
	}

/* Generic cleanup function called in case of an error: */
void vruiErrorShutdown(bool signalError)
	{
	if(signalError)
		{
		if(vruiMultiplexer!=0)
			{
			/* Signal a fatal error to all nodes and let them die: */
			//vruiMultiplexer->fatalError();
			}
		
		/* Return with an error condition: */
		exit(1);
		}
	
	/* Clean up: */
	GLThingManager::shutdown();
	if(vruiRenderingThreads!=0)
		{
		/* Cancel all rendering threads: */
		for(int i=0;i<vruiNumWindows;++i)
			{
			vruiRenderingThreads[i].cancel();
			vruiRenderingThreads[i].join();
			}
		delete[] vruiRenderingThreads;
		}
	if(vruiWindows!=0)
		{
		/* Delete all windows: */
		for(int i=0;i<vruiNumWindows;++i)
			delete vruiWindows[i];
		delete[] vruiWindows;
		}
	#ifdef VRUI_USE_OPENAL
	if(vruiSoundContexts!=0)
		{
		/* Destroy all sound contexts: */
		for(int i=0;i<vruiNumSoundContexts;++i)
			delete vruiSoundContexts[i];
		delete[] vruiSoundContexts;
		}
	#endif
	
	delete[] vruiApplicationName;
	delete vruiState;
	
	if(vruiMultiplexer!=0)
		{
		bool master=vruiMultiplexer->isMaster();
		
		/* Destroy the multiplexer: */
		delete vruiPipe;
		delete vruiMultiplexer;
		
		if(master&&vruiSlavePids!=0)
			{
			/* Wait for all slaves to terminate: */
			for(int i=0;i<vruiNumSlaves;++i)
				waitpid(vruiSlavePids[i],0,0);
			delete[] vruiSlavePids;
			}
		if(!master&&vruiSlaveArgv!=0)
			{
			/* Delete the slave command line: */
			for(int i=0;i<vruiSlaveArgc;++i)
				delete[] vruiSlaveArgv[i];
			delete[] vruiSlaveArgv;
			}
		}
	
	delete vruiConfigFile;
	}

void vruiOpenConfigurationFile(const char* userConfigurationFileName,const char* rootSectionName)
	{
	try
		{
		/* Open the system-wide configuration file: */
		vruiConfigFile=new Misc::ConfigurationFile(SYSVRUICONFIGFILE);
		}
	catch(std::runtime_error error)
		{
		/* Bail out: */
		std::cerr<<"Caught exception "<<error.what()<<" while reading system-wide configuration file"<<std::endl;
		vruiErrorShutdown(true);
		}
	
	try
		{
		/* Merge in the user configuration file: */
		vruiConfigFile->merge(userConfigurationFileName);
		}
	catch(Misc::File::OpenError err)
		{
		/* Ignore the error and continue */
		}
	catch(std::runtime_error error)
		{
		/* Bail out on errors in user configuration file: */
		std::cerr<<"Caught exception "<<error.what()<<" while reading user configuration file"<<std::endl;
		vruiErrorShutdown(true);
		}
	
	try
		{
		/* Fall back to simulator mode if the root section does not exist: */
		bool rootSectionFound=false;
		if(rootSectionName!=0)
			{
			Misc::ConfigurationFile::SectionIterator rootIt=vruiConfigFile->getRootSection().getSection("/Vrui");
			for(Misc::ConfigurationFile::SectionIterator sIt=rootIt.beginSubsections();sIt!=rootIt.endSubsections();++sIt)
				if(sIt.getName()==rootSectionName)
					{
					rootSectionFound=true;
					break;
					}
			}
		if(!rootSectionFound)
			rootSectionName=VRUIDEFAULTROOTSECTIONNAME;
		}
	catch(...)
		{
		/* Bail out if configuration file does not contain the Vrui section: */
		std::cerr<<"Configuration file does not contain /Vrui section"<<std::endl;
		vruiErrorShutdown(true);
		}
	
	/* Go to the given root section: */
	vruiConfigFile->setCurrentSection("/Vrui");
	vruiConfigFile->setCurrentSection(rootSectionName);
	}

struct VruiRenderingThreadArg
	{
	/* Elements: */
	public:
	int windowIndex; // Index of the window in the Vrui window array
	Misc::ConfigurationFileSection windowConfigFileSection; // Configuration file section for the window
	InputDeviceAdapterMouse* mouseAdapter; // Pointer to the mouse input device adapter
	};

void* vruiRenderingThreadFunction(VruiRenderingThreadArg threadArg)
	{
	Threads::Thread::setCancelState(Threads::Thread::CANCEL_ENABLE);
	Threads::Thread::setCancelType(Threads::Thread::CANCEL_ASYNCHRONOUS);
	
	/* Get this thread's parameters: */
	int windowIndex=threadArg.windowIndex;
	
	/* Create this thread's rendering window: */
	try
		{
		char windowName[256];
		if(vruiNumWindows>1)
			snprintf(windowName,sizeof(windowName),"%s%d",vruiApplicationName,windowIndex);
		else
			snprintf(windowName,sizeof(windowName),"%s",vruiApplicationName);
		vruiWindows[windowIndex]=new VRWindow(windowName,threadArg.windowConfigFileSection,vruiState,threadArg.mouseAdapter);
		}
	catch(std::runtime_error error)
		{
		std::cerr<<"Caught exception "<<error.what()<<" while initializing rendering window "<<windowIndex<<std::endl;
		delete vruiWindows[windowIndex];
		vruiWindows[windowIndex]=0;
		}
	VRWindow* window=vruiWindows[windowIndex];
	
	/* Initialize all GLObjects for this window's context data: */
	window->makeCurrent();
	window->getContextData().updateThings();
	
	/* Synchronize with the other rendering threads: */
	vruiRenderingBarrier.synchronize();
	
	/* Terminate early if there was a problem creating the rendering window: */
	if(window==0)
		return 0;
	
	/* Enter the rendering loop and redraw the window until interrupted: */
	while(true)
		{
		/* Wait for the start of the rendering cycle: */
		vruiRenderingBarrier.synchronize();
		
		/* Draw the window's contents: */
		window->draw();
		
		/* Wait until all threads are done rendering: */
		glFinish();
		vruiRenderingBarrier.synchronize();
		
		if(vruiState->multiplexer)
			{
			/* Wait until all other nodes are done rendering: */
			vruiRenderingBarrier.synchronize();
			}
		
		/* Swap buffers: */
		window->swapBuffers();
		}
	
	return 0;
	}

}

/**********************************
Call-in functions for user program:
**********************************/

void init(int& argc,char**& argv,char**&)
	{
	/* Determine whether this node is the master or a slave: */
	if(argc==8&&strcmp(argv[1],"-vruiMultipipeSlave")==0)
		{
		/********************
		This is a slave node:
		********************/
		
		/* Read multipipe settings from the command line: */
		int numSlaves=atoi(argv[2]);
		int nodeIndex=atoi(argv[3]);
		char* master=argv[4];
		int masterPort=atoi(argv[5]);
		char* multicastGroup=argv[6];
		int multicastPort=atoi(argv[7]);
		
		/* Connect back to the master: */
		try
			{
			/* Create the multicast multiplexer: */
			vruiMultiplexer=new Comm::MulticastPipeMultiplexer(numSlaves,nodeIndex,master,masterPort,multicastGroup,multicastPort);
			
			/* Wait until the entire cluster is connected: */
			vruiMultiplexer->waitForConnection();
			
			/* Open a multicast pipe: */
			vruiPipe=vruiMultiplexer->openPipe();
			
			/* Read the user configuration file and root section names: */
			int len1=vruiPipe->read<int>();
			char* userConfigFileName=new char[len1+1];
			vruiPipe->read<char>(userConfigFileName,len1+1);
			int len2=vruiPipe->read<int>();
			char* rootSectionName=new char[len2+1];
			vruiPipe->read<char>(rootSectionName,len2+1);
			
			/* Open the configuration file(s): */
			vruiOpenConfigurationFile(userConfigFileName,rootSectionName);
			delete[] userConfigFileName;
			delete[] rootSectionName;
			
			/* Read the application's command line: */
			vruiSlaveArgc=vruiPipe->read<int>();
			vruiSlaveArgv=new char*[vruiSlaveArgc+1];
			for(int i=0;i<=vruiSlaveArgc;++i)
				vruiSlaveArgv[i]=0;
			for(int i=0;i<vruiSlaveArgc;++i)
				{
				int argLen=vruiPipe->read<int>();
				vruiSlaveArgv[i]=new char[argLen+1];
				vruiPipe->read<char>(vruiSlaveArgv[i],argLen+1);
				}
			
			/* Override the actual command line provided by the caller: */
			argc=vruiSlaveArgc;
			argv=vruiSlaveArgv;
			}
		catch(std::runtime_error error)
			{
			std::cerr<<"Node "<<nodeIndex<<": Caught exception "<<error.what()<<" while initializing cluster communication"<<std::endl;
			vruiErrorShutdown(true);
			}
		}
	else
		{
		/***********************
		This is the master node:
		***********************/
		
		/* Get the user configuration file's name: */
		char* userConfigFileName=getenv("VRUI_CONFIGFILE");
		if(userConfigFileName==0)
			userConfigFileName="./Vrui.cfg";
		
		/* Get the root section name: */
		char* rootSectionName=getenv("VRUI_ROOTSECTION");
		if(rootSectionName==0)
			rootSectionName=getenv("HOSTNAME");
		
		/* Override root section name from command line: */
		for(int i=1;i<argc;++i)
			if(strcasecmp(argv[i],"-rootSection")==0)
				{
				/* Next parameter is name of root section to use: */
				if(i+1<argc)
					{
					/* Save root section name: */
					rootSectionName=argv[i+1];
					
					/* Remove parameters from argument list: */
					argc-=2;
					for(int j=i;j<argc;++j)
						argv[j]=argv[j+2];
					break;
					}
				else
					{
					/* Ignore the rootSection parameter: */
					std::cerr<<"No root section name given after -rootSection option"<<std::endl;
					--argc;
					}
				}
		
		/* Open configuration file: */
		vruiOpenConfigurationFile(userConfigFileName,rootSectionName);
		
		/* Check if this is a multipipe environment: */
		if(vruiConfigFile->retrieveValue<bool>("./enableMultipipe",false))
			{
			typedef std::vector<std::string> StringList;
			
			try
				{
				/* Read multipipe settings from configuration file: */
				std::string master=vruiConfigFile->retrieveString("./multipipeMaster");
				int masterPort=vruiConfigFile->retrieveValue<int>("./multipipeMasterPort",0);
				StringList slaves=vruiConfigFile->retrieveValue<StringList>("./multipipeSlaves");
				vruiNumSlaves=slaves.size();
				std::string multicastGroup=vruiConfigFile->retrieveString("./multipipeMulticastGroup");
				int multicastPort=vruiConfigFile->retrieveValue<int>("./multipipeMulticastPort");
				
				/* Create the multicast multiplexer: */
				vruiMultiplexer=new Comm::MulticastPipeMultiplexer(vruiNumSlaves,0,master.c_str(),masterPort,multicastGroup.c_str(),multicastPort);
				
				/* Start the multipipe slaves on all slave nodes: */
				std::string multipipeRemoteCommand=vruiConfigFile->retrieveString("./multipipeRemoteCommand","ssh");
				masterPort=vruiMultiplexer->getLocalPortNumber();
				vruiSlavePids=new pid_t[vruiNumSlaves];
				char cwd[512];
				getcwd(cwd,sizeof(cwd));
				char rc[2048];
				for(int i=0;i<vruiNumSlaves;++i)
					{
					pid_t childPid=fork();
					if(childPid==0)
						{
						/* Create a command line to run the program from the current working directory: */
						int ai=0;
						ai+=snprintf(rc+ai,sizeof(rc)-ai,"cd %s ;",cwd);
						ai+=snprintf(rc+ai,sizeof(rc)-ai," %s",argv[0]);
						ai+=snprintf(rc+ai,sizeof(rc)-ai," -vruiMultipipeSlave");
						ai+=snprintf(rc+ai,sizeof(rc)-ai," %d %d",vruiNumSlaves,i+1);
						ai+=snprintf(rc+ai,sizeof(rc)-ai," %s %d",master.c_str(),masterPort);
						ai+=snprintf(rc+ai,sizeof(rc)-ai," %s %d",multicastGroup.c_str(),multicastPort);
						
						/* Create command line for the ssh (or other remote login) program: */
						char* sshArgv[20];
						int sshArgc=0;
						sshArgv[sshArgc++]=const_cast<char*>(multipipeRemoteCommand.c_str());
						sshArgv[sshArgc++]=const_cast<char*>(slaves[i].c_str());
						sshArgv[sshArgc++]=rc;
						sshArgv[sshArgc]=0;
						
						/* Run the remote login program: */
						execvp(sshArgv[0],sshArgv);
						}
					else
						{
						/* Store PID of ssh process for later: */
						vruiSlavePids[i]=childPid;
						}
					}
				
				/* Wait until the entire cluster is connected: */
				vruiMultiplexer->waitForConnection();
				
				/* Open a multicast pipe: */
				vruiPipe=vruiMultiplexer->openPipe();
				
				/* Write the user configuration file and root section names: */
				int len1=strlen(userConfigFileName);
				vruiPipe->write<int>(len1);
				vruiPipe->write<char>(userConfigFileName,len1+1);
				int len2=strlen(rootSectionName);
				vruiPipe->write<int>(len2);
				vruiPipe->write<char>(rootSectionName,len2+1);
				
				/* Write the application's command line: */
				vruiPipe->write<int>(argc);
				for(int i=0;i<argc;++i)
					{
					int argLen=strlen(argv[i]);
					vruiPipe->write<int>(argLen);
					vruiPipe->write<char>(argv[i],argLen+1);
					}
				
				/* Flush the pipe: */
				vruiPipe->finishMessage();
				}
			catch(std::runtime_error error)
				{
				std::cerr<<"Master node: Caught exception "<<error.what()<<" while initializing cluster communication"<<std::endl;
				vruiErrorShutdown(true);
				}
			}
		}
	
	/* Initialize Vrui state object: */
	try
		{
		vruiState=new VruiState(vruiMultiplexer,vruiPipe);
		vruiState->initialize(vruiConfigFile->getCurrentSection());
		}
	catch(std::runtime_error error)
		{
		std::cerr<<"Caught exception "<<error.what()<<" while initializing Vrui state object"<<std::endl;
		vruiErrorShutdown(true);
		}
	
	/* Check if user wants to load a viewpoint file via the command line: */
	for(int i=1;i<argc;++i)
		if(strcasecmp(argv[i],"-loadView")==0)
			{
			/* Next parameter is name of viewpoint file to load: */
			if(i+1<argc)
				{
				/* Save viewpoint file name: */
				vruiState->viewpointFileName=argv[i+1];
				
				/* Remove parameters from argument list: */
				argc-=2;
				for(int j=i;j<argc;++j)
					argv[j]=argv[j+2];
				break;
				}
			else
				{
				/* Ignore the loadView parameter: */
				std::cerr<<"No viewpoint file name given after -loadView option"<<std::endl;
				--argc;
				}
			}
	
	/* Load all vislets listed on the command line: */
	for(int i=1;i<argc;++i)
		if(strcasecmp(argv[i],"-vislet")==0)
			{
			if(i+1<argc)
				{
				/* First parameter is name of vislet class: */
				const char* className=argv[i+1];
				
				/* Find semicolon terminating vislet parameter list: */
				int argEnd;
				for(argEnd=i+2;argEnd<argc&&(argv[argEnd][0]!=';'||argv[argEnd][1]!='\0');++argEnd)
					;
				
				if(vruiState->visletManager!=0)
					{
					try
						{
						/* Initialize the vislet: */
						VisletFactory* factory=vruiState->visletManager->loadClass(className);
						vruiState->visletManager->createVislet(factory,argEnd-(i+2),argv+(i+2));
						}
					catch(std::runtime_error err)
						{
						std::cerr<<"Vrui::init: Ignoring vislet of type "<<className<<" due to exception "<<err.what()<<std::endl;
						}
					}
				
				/* Remove all vislet parameters from the command line: */
				if(argEnd<argc)
					++argEnd;
				int numArgs=argEnd-i;
				argc-=numArgs;
				for(int j=i;j<argc;++j)
					argv[j]=argv[j+numArgs];
				}
			else
				{
				/* Ignore the vislet parameter: */
				std::cerr<<"No vislet class name given after -vislet option"<<std::endl;
				argc=i;
				}
			}
		
	/* Extract the application name: */
	const char* appNameStart=argv[0];
	const char* cPtr;
	for(cPtr=appNameStart;*cPtr!='\0';++cPtr)
		if(*cPtr=='/')
			appNameStart=cPtr+1;
	vruiApplicationName=new char[cPtr-appNameStart+1];
	memcpy(vruiApplicationName,appNameStart,cPtr-appNameStart);
	vruiApplicationName[cPtr-appNameStart]='\0';
	}

void startDisplay(PerDisplayInitFunctionType perDisplayInitFunction,void* userData)
	{
	/* Wait for all nodes in the multicast group to reach this point: */
	if(vruiState->multiplexer!=0)
		vruiState->pipe->barrier();
	
	vruiState->perDisplayInitFunction=perDisplayInitFunction;
	vruiState->perDisplayInitFunctionData=userData;
	
	/* Initialize the Vrui tools: */
	vruiState->initTools(vruiConfigFile->getCurrentSection());
	
	/* Find the mouse adapter listed in the input device manager (if there is one): */
	InputDeviceAdapterMouse* mouseAdapter=0;
	for(int i=0;i<vruiState->inputDeviceManager->getNumInputDeviceAdapters()&&mouseAdapter==0;++i)
		mouseAdapter=dynamic_cast<InputDeviceAdapterMouse*>(vruiState->inputDeviceManager->getInputDeviceAdapter(i));
	
	try
		{
		/* Retrieve the list of VR windows and determine whether to use one rendering thread per window: */
		typedef std::vector<std::string> StringList;
		
		StringList windowNames;
		if(vruiState->multiplexer!=0)
			{
			char windowNamesTag[40];
			snprintf(windowNamesTag,sizeof(windowNamesTag),"./node%dWindowNames",vruiState->multiplexer->getNodeIndex());
			windowNames=vruiConfigFile->retrieveValue<StringList>(windowNamesTag);
			
			char windowsMultithreadedTag[40];
			snprintf(windowsMultithreadedTag,sizeof(windowsMultithreadedTag),"./node%dWindowsMultithreaded",vruiState->multiplexer->getNodeIndex());
			vruiWindowsMultithreaded=vruiConfigFile->retrieveValue<bool>(windowsMultithreadedTag,false);
			}
		else
			{
			windowNames=vruiConfigFile->retrieveValue<StringList>("./windowNames");
			vruiWindowsMultithreaded=vruiConfigFile->retrieveValue<bool>("./windowsMultithreaded",false);
			}
		vruiNumWindows=windowNames.size();
		
		/* Ready the GLObject manager to initialize its objects per-window: */
		GLContextData::resetThingManager();
		
		/* Create all rendering windows: */
		vruiWindows=new VRWindow*[vruiNumWindows];
		for(int i=0;i<vruiNumWindows;++i)
			vruiWindows[i]=0;
		
		if(vruiWindowsMultithreaded)
			{
			/* Initialize the rendering barrier: */
			vruiRenderingBarrier.setNumSynchronizingThreads(vruiNumWindows+1);
			
			/* Create one rendering thread for each window (which will in turn create the windows themselves): */
			vruiRenderingThreads=new Threads::Thread[vruiNumWindows];
			for(int i=0;i<vruiNumWindows;++i)
				{
				VruiRenderingThreadArg ta;
				ta.windowIndex=i;
				ta.windowConfigFileSection=vruiConfigFile->getSection(windowNames[i].c_str());
				ta.mouseAdapter=mouseAdapter;
				vruiRenderingThreads[i].start(vruiRenderingThreadFunction,ta);
				}
			
			/* Wait until all threads have created their windows: */
			vruiRenderingBarrier.synchronize();
			
			/* Check if all windows have been properly created: */
			bool windowsOk=true;
			for(int i=0;i<vruiNumWindows;++i)
				if(vruiWindows[i]==0)
					windowsOk=false;
			if(!windowsOk)
				Misc::throwStdErr("Vrui::startDisplay: Could not create all rendering windows");
			}
		else
			{
			for(int i=0;i<vruiNumWindows;++i)
				{
				char windowName[256];
				if(vruiNumWindows>1)
					snprintf(windowName,sizeof(windowName),"%s%d",vruiApplicationName,i);
				else
					snprintf(windowName,sizeof(windowName),"%s",vruiApplicationName);
				vruiWindows[i]=new VRWindow(windowName,vruiConfigFile->getSection(windowNames[i].c_str()),vruiState,mouseAdapter);
				
				/* Initialize all GLObjects for this window's context data: */
				vruiWindows[i]->makeCurrent();
				vruiWindows[i]->getContextData().updateThings();
				}
			}
		}
	catch(std::runtime_error error)
		{
		std::cerr<<"Caught exception "<<error.what()<<" while initializing rendering windows"<<std::endl;
		vruiErrorShutdown(true);
		}
	
	/* Check if the user gave a viewpoint file on the command line: */
	if(!vruiState->viewpointFileName.empty())
		{
		/* Override the navigation transformation: */
		if(!vruiState->loadViewpointFile(vruiState->viewpointFileName.c_str()))
			{
			/* Print a warning message and continue: */
			std::cerr<<"Unable to load viewpoint file "<<vruiState->viewpointFileName<<std::endl;
			}
		}
	}

void startSound(PerSoundInitFunctionType perSoundInitFunction,void* userData)
	{
	/* Wait for all nodes in the multicast group to reach this point: */
	if(vruiState->multiplexer!=0)
		vruiState->pipe->barrier();
	
	#ifdef VRUI_USE_OPENAL
	vruiState->perSoundInitFunction=perSoundInitFunction;
	vruiState->perSoundInitFunctionData=userData;
	
	/* Retrieve the name of the sound device: */
	std::string soundDeviceName;
	if(vruiState->multiplexer!=0)
		{
		char soundDeviceNameTag[40];
		snprintf(soundDeviceNameTag,sizeof(soundDeviceNameTag),"./node%dSoundDeviceName",vruiState->multiplexer->getNodeIndex());
		soundDeviceName=vruiConfigFile->retrieveValue<std::string>(soundDeviceNameTag,"");
		}
	else
		soundDeviceName=vruiConfigFile->retrieveValue<std::string>("./soundDeviceName","");
	
	if(soundDeviceName!="")
		{
		/* Create the sound context: */
		vruiNumSoundContexts=1;
		vruiSoundContexts=new ALContextData*[1];
		for(int i=0;i<vruiNumSoundContexts;++i)
			vruiSoundContexts[i]=0;
		for(int i=0;i<vruiNumSoundContexts;++i)
			{
			/* Create a new sound context: */
			vruiSoundContexts[i]=new ALContextData(101);
			
			/* Initialize Vrui sound state for this sound context: */
			vruiState->initSound(*vruiSoundContexts[i]);
			
			/* Initialize application sound state: */
			if(vruiState->perSoundInitFunction!=0)
				vruiState->perSoundInitFunction(*vruiSoundContexts[i],vruiState->perSoundInitFunctionData);
			}
		}
	#endif
	}

void vruiInnerLoopMultiWindow(void)
	{
	if(vruiNumWindows==0&&vruiState->master)
		{
		/* Disable line buffering on stdin to detect key presses in the inner loop: */
		termios term;
		tcgetattr(fileno(stdin),&term);
		term.c_lflag&=~ICANON;
		tcsetattr(fileno(stdin),TCSANOW,&term);
		setbuf(stdin,0);
		
		printf("Press Esc to exit...\n");
		}
	
	bool processEvents=true;
	while(processEvents)
		{
		/* Process all pending events: */
		for(int i=0;i<vruiNumWindows;++i)
			while(XPending(vruiWindows[i]->getDisplay()))
				{
				XEvent event;
				XNextEvent(vruiWindows[i]->getDisplay(),&event);
				for(int j=0;j<vruiNumWindows;++j)
					if(vruiWindows[j]->isEventForWindow(event))
						{
						if(vruiWindows[j]->processEvent(event))
							processEvents=false;
						break;
						}
				}
		
		if(vruiNumWindows==0&&vruiState->master)
			{
			/* Check for Escape key on the console for window-less Vrui processes: */
			fd_set readFdSet;
			FD_ZERO(&readFdSet);
			FD_SET(fileno(stdin),&readFdSet);
			struct timeval timeout;
			timeout.tv_sec=0;
			timeout.tv_usec=0;
			if(select(fileno(stdin)+1,&readFdSet,0,0,&timeout)>=0&&FD_ISSET(fileno(stdin),&readFdSet))
				processEvents=false;
			}
		
		/* Check for asynchronous shutdown: */
		if(vruiAsynchronousShutdown)
			processEvents=false;
		
		/* Update Vrui state: */
		if(vruiState->multiplexer!=0)
			vruiState->pipe->broadcast(processEvents);
		if(!processEvents)
			{
			if(vruiState->multiplexer!=0&&vruiState->master)
				vruiState->pipe->finishMessage();
			break;
			}
		vruiState->update();
		
		/* Reset the GL thing manager: */
		GLContextData::resetThingManager();
		
		if(vruiWindowsMultithreaded)
			{
			/* Start the rendering cycle by synchronizing with the render threads: */
			vruiRenderingBarrier.synchronize();
			
			/* Wait until all threads are done rendering: */
			vruiRenderingBarrier.synchronize();
			
			if(vruiState->multiplexer!=0)
				{
				/* Synchronize with other nodes: */
				vruiState->pipe->barrier();
				
				/* Notify the render threads to swap buffers: */
				vruiRenderingBarrier.synchronize();
				}
			}
		else
			{
			/* Update rendering: */
			for(int i=0;i<vruiNumWindows;++i)
				vruiWindows[i]->draw();
			
			if(vruiState->multiplexer!=0)
				{
				/* Synchronize with other nodes: */
				for(int i=0;i<vruiNumWindows;++i)
					{
					vruiWindows[i]->makeCurrent();
					glFinish();
					}
				vruiState->pipe->barrier();
				}
			
			/* Swap all buffers at once: */
			for(int i=0;i<vruiNumWindows;++i)
				{
				vruiWindows[i]->makeCurrent();
				vruiWindows[i]->swapBuffers();
				}
			}
		
		/* Print current frame rate on head node's console for window-less Vrui processes: */
		if(vruiNumWindows==0&&vruiState->master)
			{
			printf("\rCurrent frame rate: %8.3f fps",1.0/vruiState->currentFrameTime);
			fflush(stdout);
			}
		}
	
	if(vruiNumWindows==0&&vruiState->master)
		printf("\n");
	}

void vruiInnerLoopSingleWindow(void)
	{
	VRWindow* win=vruiWindows[0];
	
	bool processEvents=true;
	while(processEvents)
		{
	  /* Process all pending events: */
		while(XPending(win->getDisplay()))
			{
			XEvent event;
			XNextEvent(win->getDisplay(),&event);
			processEvents=processEvents&&!win->processEvent(event);
			}
		
		/* Check for asynchronous shutdown: */
		if(vruiAsynchronousShutdown)
			processEvents=false;
		
		/* Update Vrui state: */
		if(vruiState->multiplexer!=0)
			vruiState->pipe->broadcast(processEvents);
		if(!processEvents)
			{
			if(vruiState->multiplexer!=0&&vruiState->master)
				vruiState->pipe->finishMessage();
			break;
			}
		vruiState->update();
		
		/* Reset the GL thing manager: */
		GLContextData::resetThingManager();
		
		/* Update rendering: */
		win->draw();
		
		if(vruiState->multiplexer!=0)
			{
			/* Synchronize with other nodes: */
			glFinish();
			vruiState->pipe->barrier();
			}
		
		/* Swap buffer: */
		win->swapBuffers();
		}
	}

void vruiInnerLoopSingleWindowBlocking(void)
	{
	VRWindow* win=vruiWindows[0];
	
	bool processEvents=true;
	while(processEvents)
		{
		/* Handle X events: */
		vruiState->innerLoopBlocked=true;
		if(vruiState->mustRedraw||!vruiState->master)
			{
			vruiState->innerLoopBlocked=false;
			
	  	/* Process all pending events: */
			while(XPending(win->getDisplay()))
				{
				XEvent event;
				XNextEvent(win->getDisplay(),&event);
				processEvents=processEvents&&!win->processEvent(event);
				}
			}
		else
			{
			/* Wait for the next event: */
			do
				{
				XEvent event;
				XNextEvent(win->getDisplay(),&event);
				vruiState->innerLoopBlocked=false;
				if(event.type==Expose)
					vruiState->mustRedraw=true;
				processEvents=processEvents&&!win->processEvent(event);
				}
			while(XPending(win->getDisplay()));
			}
		
		/* Check for asynchronous shutdown: */
		if(vruiAsynchronousShutdown)
			processEvents=false;
		
		/* Redraw all VR windows: */
		if(vruiState->mustRedraw)
			{
			vruiState->mustRedraw=false;
			
			/* Update Vrui state: */
			if(vruiState->multiplexer!=0)
				vruiState->pipe->broadcast(processEvents);
			if(!processEvents)
				{
				if(vruiState->multiplexer!=0&&vruiState->master)
					vruiState->pipe->finishMessage();
				break;
				}
			vruiState->update();
			
			/* Reset the GL thing manager: */
			GLContextData::resetThingManager();
			
			/* Update rendering: */
			win->draw();
			
			if(vruiState->multiplexer!=0)
				{
				/* Synchronize with other nodes: */
				glFinish();
				vruiState->pipe->barrier();
				}
			
			/* Swap buffer: */
			win->swapBuffers();
			}
		}
	}

void mainLoop(void)
	{
	/* Wait for all nodes in the multicast group to reach this point: */
	if(vruiState->multiplexer!=0)
		vruiState->pipe->barrier();
	
	#if 0
	/* Turn off the screen saver: */
	int screenSaverTimeout,screenSaverInterval;
	int screenSaverPreferBlanking,screenSaverAllowExposures;
	XGetScreenSaver(vruiWindow->getDisplay(),&screenSaverTimeout,&screenSaverInterval,&screenSaverPreferBlanking,&screenSaverAllowExposures);
	XSetScreenSaver(vruiWindow->getDisplay(),0,0,DefaultBlanking,DefaultExposures);
	XResetScreenSaver(vruiWindow->getDisplay());
	#endif
	
	/* Perform the main loop until the ESC key is hit: */
	if(vruiNumWindows!=1)
		vruiInnerLoopMultiWindow();
	else if(vruiState->updateContinuously)
		vruiInnerLoopSingleWindow();
	else
		vruiInnerLoopSingleWindowBlocking();
	
	/* Shut down the rendering system: */
	GLThingManager::shutdown();
	if(vruiRenderingThreads!=0)
		{
		/* Cancel all rendering threads: */
		for(int i=0;i<vruiNumWindows;++i)
			{
			vruiRenderingThreads[i].cancel();
			vruiRenderingThreads[i].join();
			}
		delete[] vruiRenderingThreads;
		}
	if(vruiWindows!=0)
		{
		/* Delete all windows: */
		for(int i=0;i<vruiNumWindows;++i)
			delete vruiWindows[i];
		delete[] vruiWindows;
		}
	#ifdef VRUI_USE_OPENAL
	if(vruiSoundContexts!=0)
		{
		/* Destroy all sound contexts: */
		for(int i=0;i<vruiNumSoundContexts;++i)
			delete vruiSoundContexts[i];
		delete[] vruiSoundContexts;
		}
	#endif
	
	#if 0
	/* Turn the screen saver back on: */
	XSetScreenSaver(vruiWindow->getDisplay(),screenSaverTimeout,screenSaverInterval,screenSaverPreferBlanking,screenSaverAllowExposures);
	#endif
	}

void deinit(void)
	{
	/* Clean up: */
	delete[] vruiApplicationName;
	delete vruiState;
	
	if(vruiMultiplexer!=0)
		{
		bool master=vruiMultiplexer->isMaster();
		
		/* Destroy the multiplexer: */
		delete vruiPipe;
		delete vruiMultiplexer;
		
		if(master&&vruiSlavePids!=0)
			{
			/* Wait for all slaves to terminate: */
			for(int i=0;i<vruiNumSlaves;++i)
				waitpid(vruiSlavePids[i],0,0);
			delete[] vruiSlavePids;
			}
		if(!master&&vruiSlaveArgv!=0)
			{
			/* Delete the slave command line: */
			for(int i=0;i<vruiSlaveArgc;++i)
				delete[] vruiSlaveArgv[i];
			delete[] vruiSlaveArgv;
			}
		}
	
	delete vruiConfigFile;
	}

void shutdown(void)
	{
	/* Signal asynchronous shutdown if this node is the master node: */
	if(vruiState->master)
		{
		vruiAsynchronousShutdown=true;
		Vrui::requestUpdate();
		}
	}

int getNumWindows(void)
	{
	return vruiNumWindows;
	}

VRWindow* getWindow(int index)
	{
	return vruiWindows[index];
	}

ViewSpecification calcViewSpec(int windowIndex,int eyeIndex)
	{
	/* Get the view specification in physical coordinates: */
	ViewSpecification viewSpec=vruiWindows[windowIndex]->calcViewSpec(eyeIndex);
	
	if(vruiState->navigationTransformationEnabled)
		{
		/* Transform the view specification to navigation coordinates: */
		ATransform invNav=vruiState->inverseNavigationTransformation;
		Scalar invNavScale=vruiState->inverseNavigationTransformation.getScaling();
		Plane newScreenPlane=viewSpec.getScreenPlane();
		newScreenPlane.transform(invNav);
		newScreenPlane.normalize();
		viewSpec.setScreenPlane(newScreenPlane);
		Scalar newScreenSize[2];
		for(int i=0;i<2;++i)
			{
			newScreenSize[i]=viewSpec.getScreenSize(i);
			newScreenSize[i]*=invNavScale;
			}
		viewSpec.setScreenSize(newScreenSize);
		viewSpec.setEye(invNav.transform(viewSpec.getEye()));
		viewSpec.setEyeScreenDistance(viewSpec.getEyeScreenDistance()*invNavScale);
		for(int i=0;i<8;++i)
			viewSpec.setFrustumVertex(i,invNav.transform(viewSpec.getFrustumVertex(i)));
		for(int i=0;i<6;++i)
			{
			Plane newFrustumPlane=viewSpec.getFrustumPlane(i);
			newFrustumPlane.transform(invNav);
			newFrustumPlane.normalize();
			viewSpec.setFrustumPlane(i,newFrustumPlane);
			}
		}
	
	return viewSpec;
	}

void requestUpdate(void)
	{
	if(vruiState->master&&!vruiState->mustRedraw)
		{
		Threads::Mutex::Lock requestUpdateLock(vruiState->requestUpdateMutex);
		
		/* Flag a redraw of all VR windows: */
		vruiState->mustRedraw=true;
		
		if(vruiState->innerLoopBlocked)
			{
			/* Wake up the inner loop by sending a bogus X event: */
			vruiWindows[0]->redraw();
			}
		}
	}

}
