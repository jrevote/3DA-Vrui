/***********************************************************************
Environment-independent part of Vrui virtual reality development
toolkit.
Copyright (c) 2000-2007 Oliver Kreylos

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

#define DELAY_NAVIGATIONTRANSFORMATION 1
#define RENDERFRAMETIMES 0
#define SAVESHAREDVRUISTATE 0

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Misc/CreateNumberedFileName.h>
#include <Misc/ValueCoder.h>
#include <Misc/StandardValueCoders.h>
#include <Misc/CompoundValueCoders.h>
#include <Misc/ConfigurationFile.h>
#include <Comm/MulticastPipeMultiplexer.h>
#include <Comm/MulticastPipe.h>
#include <Math/Constants.h>
#include <Geometry/GeometryValueCoders.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLLightModelTemplates.h>
#include <GL/GLContextData.h>
#include <GL/GLFont.h>
#include <GL/GLValueCoders.h>
#include <GL/GLGeometryWrappers.h>
#include <GL/GLTransformationWrappers.h>
#include <GLMotif/Event.h>
#include <GLMotif/Widget.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/Container.h>
#include <GLMotif/Popup.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/Menu.h>
#include <GLMotif/SubMenu.h>
#include <GLMotif/Button.h>
#include <GLMotif/CascadeButton.h>
#include <Vrui/TransparentObject.h>
#include <Vrui/VirtualInputDevice.h>
#include <Vrui/InputGraphManager.h>
#include <Vrui/InputDeviceManager.h>
#include <Vrui/InputDeviceAdapter.h>
#include <Vrui/InputDeviceAdapterMouse.h>
#include <Vrui/InputDeviceAdapterDeviceDaemon.h>
#include <Vrui/MultipipeDispatcher.h>
#include <Vrui/LightsourceManager.h>
#include <Vrui/Viewer.h>
#include <Vrui/VRScreen.h>
#include <Vrui/MutexMenu.h>
#include <Vrui/CoordinateManager.h>
#include <Vrui/ToolManager.h>
#include <Vrui/VisletManager.h>
#include <Vrui/InputDeviceDataSaver.h>

#include <Vrui/Vrui.Internal.h>

namespace Misc {

/***********************************************************************
Helper class to read screen protection values from a configuration file:
***********************************************************************/

template <>
class ValueCoder<Vrui::VruiState::ScreenProtector>
	{
	/* Methods: */
	public:
	static std::string encode(const Vrui::VruiState::ScreenProtector& value)
		{
		std::string result="";
		result+="( ";
		result+=ValueCoder<std::string>::encode(value.inputDevice->getDeviceName());
		result+=", ";
		result+=ValueCoder<Vrui::Point>::encode(value.center);
		result+=", ";
		result+=ValueCoder<Vrui::Scalar>::encode(value.radius);
		result+=" )";
		return result;
		}
	static Vrui::VruiState::ScreenProtector decode(const char* start,const char* end,const char** decodeEnd =0)
		{
		Vrui::VruiState::ScreenProtector result;
		const char* cPtr=start;
		try
			{
			if(*cPtr!='(')
				throw DecodingError("Missing opening parenthesis");
			++cPtr;
			cPtr=skipWhitespace(cPtr,end);
			std::string inputDeviceName=ValueCoder<std::string>::decode(cPtr,end,&cPtr);
			result.inputDevice=Vrui::findInputDevice(inputDeviceName.c_str());
			if(result.inputDevice==0)
				Misc::throwStdErr("Input device \"%s\" not found",inputDeviceName.c_str());
			cPtr=skipSeparator(',',cPtr,end);
			result.center=ValueCoder<Vrui::Point>::decode(cPtr,end,&cPtr);
			cPtr=skipSeparator(',',cPtr,end);
			result.radius=ValueCoder<Vrui::Scalar>::decode(cPtr,end,&cPtr);
			cPtr=skipWhitespace(cPtr,end);
			if(*cPtr!=')')
				throw DecodingError("Missing closing parenthesis");
			++cPtr;
			}
		catch(std::runtime_error err)
			{
			throw DecodingError(std::string("Unable to convert \"")+std::string(start,end)+std::string("\" to ScreenProtector due to ")+err.what());
			}
		
		/* Return result: */
		if(decodeEnd!=0)
			*decodeEnd=cPtr;
		return result;
		}
	};

}

namespace Vrui {

/************
Global state:
************/

VruiState* vruiState;

#if RENDERFRAMETIMES
const int numFrameTimes=800;
double frameTimes[numFrameTimes];
int frameTimeIndex=-1;
#endif
#if SAVESHAREDVRUISTATE
Misc::File* vruiSharedStateFile=0;
#endif

/**********************
Private Vrui functions:
**********************/

GLMotif::Popup* VruiState::buildViewMenu(void)
	{
	GLMotif::Popup* viewMenuPopup=new GLMotif::Popup("ViewMenuPopup",getWidgetManager());
	
	GLMotif::SubMenu* viewMenu=new GLMotif::SubMenu("View",viewMenuPopup,false);
	
	GLMotif::Button* loadViewButton=new GLMotif::Button("LoadViewButton",viewMenu,"Load View");
	loadViewButton->getSelectCallbacks().add(this,&VruiState::loadViewCallback);
	
	GLMotif::Button* saveViewButton=new GLMotif::Button("SaveViewButton",viewMenu,"Save View");
	saveViewButton->getSelectCallbacks().add(this,&VruiState::saveViewCallback);
	
	GLMotif::Button* restoreViewButton=new GLMotif::Button("RestoreViewButton",viewMenu,"Restore View");
	restoreViewButton->getSelectCallbacks().add(this,&VruiState::restoreViewCallback);
	
	viewMenu->manageChild();
	
	return viewMenuPopup;
	}

void VruiState::buildSystemMenu(GLMotif::Container* parent)
	{
	/* Create the view submenu: */
	GLMotif::CascadeButton* viewMenuCascade=new GLMotif::CascadeButton("ViewMenuCascade",parent,"View");
	viewMenuCascade->setPopup(buildViewMenu());
	
	/* Create buttons to create or destroy virtual input device: */
	GLMotif::Button* createInputDeviceButton=new GLMotif::Button("CreateInputDeviceButton",parent,"Create Input Device");
	createInputDeviceButton->getSelectCallbacks().add(this,&VruiState::createInputDeviceCallback);
	
	GLMotif::Button* destroyInputDeviceButton=new GLMotif::Button("DestroyInputDeviceButton",parent,"Destroy Input Device");
	destroyInputDeviceButton->getSelectCallbacks().add(this,&VruiState::destroyInputDeviceCallback);
	
	/* Create a button to show the scale bar: */
	GLMotif::ToggleButton* showScaleBarToggle=new GLMotif::ToggleButton("ShowScaleBarToggle",parent,"Show Scale Bar");
	showScaleBarToggle->getValueChangedCallbacks().add(this,&VruiState::showScaleBarToggleCallback);
	
	/* Create a button to quit the current application: */
	GLMotif::Button* quitButton=new GLMotif::Button("QuitButton",parent,"Quit Program");
	quitButton->getSelectCallbacks().add(this,&VruiState::quitCallback);
	}

bool VruiState::loadViewpointFile(const char* viewpointFileName)
	{
	try
		{
		/* Open the viewpoint file: */
		Misc::File viewpointFile(viewpointFileName,"rb",Misc::File::LittleEndian);

		/* Check the header: */
		char line[80];
		viewpointFile.gets(line,sizeof(line));
		if(strcmp(line,"Vrui viewpoint file v1.0\n")==0)
			{
			/* Read the environment's center point in navigational coordinates: */
			Point center;
			viewpointFile.read<Scalar>(center.getComponents(),3);
			
			/* Read the environment's size in navigational coordinates: */
			Scalar size=viewpointFile.read<Scalar>();
			
			/* Read the environment's forward direction in navigational coordinates: */
			Vector forward;
			viewpointFile.read<Scalar>(forward.getComponents(),3);
			
			/* Read the environment's up direction in navigational coordinates: */
			Vector up;
			viewpointFile.read<Scalar>(up.getComponents(),3);
			
			/* Construct the navigation transformation: */
			NavTransform nav=NavTransform::identity;
			nav*=NavTransform::translateFromOriginTo(getDisplayCenter());
			nav*=NavTransform::rotate(Rotation::fromBaseVectors(Geometry::cross(getForwardDirection(),getUpDirection()),getForwardDirection()));
			nav*=NavTransform::scale(getDisplaySize()/size);
			nav*=NavTransform::rotate(Geometry::invert(Rotation::fromBaseVectors(Geometry::cross(forward,up),forward)));
			nav*=NavTransform::translateToOriginFrom(center);
			setNavigationTransformation(nav);
			
			return true;
			}
		else
			return false;
		}
	catch(std::runtime_error error)
		{
		/* Ignore the error and return an error code: */
		return false;
		}
	}

VruiState::VruiState(Comm::MulticastPipeMultiplexer* sMultiplexer,Comm::MulticastPipe* sPipe)
	:multiplexer(sMultiplexer),
	 master(multiplexer==0||multiplexer->isMaster()),
	 pipe(sPipe),
	 inchScale(1.0),
	 displayCenter(0.0,0.0,0.0),displaySize(1.0),
	 forwardDirection(0.0,1.0,0.0),
	 upDirection(0.0,0.0,1.0),
	 floorPlane(Vector(0.0,0.0,1.0),0.0),
	 glyphRenderer(0),
	 newInputDevicePosition(0.0,0.0,0.0),
	 virtualInputDevice(0),
	 inputGraphManager(0),
	 inputDeviceManager(0),
	 inputDeviceDataSaver(0),
	 multipipeDispatcher(0),
	 lightsourceManager(0),
	 numViewers(0),viewers(0),mainViewer(0),
	 numScreens(0),screens(0),mainScreen(0),
	 numProtectors(0),protectors(0),
	 frontplaneDist(1.0),
	 backplaneDist(1000.0),
	 backgroundColor(Color(0.0f,0.0f,0.0f,1.0f)),
	 ambientLightColor(Color(0.2f,0.2f,0.2f)),
	 widgetMaterial(GLMaterial::Color(1.0f,1.0f,1.0f),GLMaterial::Color(0.5f,0.5f,0.5f),25.0f),
	 widgetManager(0),
	 popWidgetsOnScreen(false),
	 systemMenuPopup(0),
	 mainMenu(0),
	 navigationTransformationEnabled(false),
	 navigationTransformationChanged(false),
	 navigationTransformation(NavTransform::identity),inverseNavigationTransformation(NavTransform::identity),
	 coordinateManager(0),
	 toolManager(0),
	 visletManager(0),
	 frameFunction(0),frameFunctionData(0),
	 displayFunction(0),displayFunctionData(0),
	 perDisplayInitFunction(0),perDisplayInitFunctionData(0),
	 soundFunction(0),soundFunctionData(0),
	 perSoundInitFunction(0),perSoundInitFunctionData(0),
	 randomSeed(0),
	 numRecentFrameTimes(0),recentFrameTimes(0),nextFrameTimeIndex(0),sortedFrameTimes(0),
	 activeNavigationTool(0),
	 widgetInteraction(false),motionWidget(0),
	 updateContinuously(false),mustRedraw(true),innerLoopBlocked(false)
	{
	#if SAVESHAREDVRUISTATE
	vruiSharedStateFile=new Misc::File("/tmp/VruiSharedState.dat","wb",Misc::File::LittleEndian);
	#endif
	}

VruiState::~VruiState(void)
	{
	#if SAVESHAREDVRUISTATE
	delete vruiSharedStateFile;
	#endif
	
	/* Delete time management: */
	delete[] recentFrameTimes;
	delete[] sortedFrameTimes;
	
	/* Delete vislet management: */
	delete visletManager;
	
	/* Delete tool management: */
	delete toolManager;
	
	/* Delete coordinate manager: */
	delete coordinateManager;
	
	/* Delete widget management: */
	delete systemMenuPopup;
	delete mainMenu;
	delete uiStyleSheet.font;
	delete widgetManager;
	
	/* Delete screen protection management: */
	delete[] protectors;
	
	/* Delete screen management: */
	delete[] screens;
	
	/* Delete viewer management: */
	delete[] viewers;
	
	/* Delete light source management: */
	delete lightsourceManager;
	
	/* Delete input device management: */
	delete multipipeDispatcher;
	delete inputDeviceDataSaver;
	delete inputDeviceManager;
	
	/* Delete input graph management: */
	delete inputGraphManager;
	delete virtualInputDevice;
	
	/* Delete glyph management: */
	delete glyphRenderer;
	}

void VruiState::initialize(const Misc::ConfigurationFileSection& configFileSection)
	{
	typedef std::vector<std::string> StringList;
	
	if(multiplexer!=0)
		{
		/* Set the multiplexer's timeout values: */
		multiplexer->setConnectionWaitTimeout(configFileSection.retrieveValue<double>("./multipipeConnectionWaitTimeout",0.1));
		multiplexer->setPingTimeout(configFileSection.retrieveValue<double>("./multipipePingTimeout",10.0),configFileSection.retrieveValue<int>("./multipipePingRetries",3));
		multiplexer->setReceiveWaitTimeout(configFileSection.retrieveValue<double>("./multipipeReceiveWaitTimeout",0.01));
		multiplexer->setBarrierWaitTimeout(configFileSection.retrieveValue<double>("./multipipeBarrierWaitTimeout",0.01));
		}
	
	/* Initialize environment dimensions: */
	inchScale=configFileSection.retrieveValue<Scalar>("./inchScale",1.0);
	displayCenter=configFileSection.retrieveValue<Point>("./displayCenter");
	displaySize=configFileSection.retrieveValue<Scalar>("./displaySize");
	forwardDirection=configFileSection.retrieveValue<Vector>("./forwardDirection",forwardDirection);
	forwardDirection.normalize();
	upDirection=configFileSection.retrieveValue<Vector>("./upDirection",upDirection);
	upDirection.normalize();
	floorPlane=configFileSection.retrieveValue<Plane>("./floorPlane",floorPlane);
	floorPlane.normalize();
	
	/* Initialize the glyph renderer: */
	glyphRenderer=new GlyphRenderer(configFileSection.retrieveValue<GLfloat>("./glyphSize",GLfloat(inchScale)));
	
	/* Initialize input graph manager: */
	newInputDevicePosition=configFileSection.retrieveValue<Point>("./newInputDevicePosition",displayCenter);
	virtualInputDevice=new VirtualInputDevice(glyphRenderer,configFileSection);
	inputGraphManager=new InputGraphManager(glyphRenderer,virtualInputDevice);
	
	/* Initialize input device manager: */
	inputDeviceManager=new InputDeviceManager(inputGraphManager);
	if(master)
		{
		inputDeviceManager->initialize(configFileSection);
		if(configFileSection.retrieveValue<bool>("./saveInputDeviceData",false))
			{
			std::string inputDeviceDataFileName=configFileSection.retrieveString("./inputDeviceDataFileName");
			char numberedFileName[1024];
			inputDeviceDataSaver=new InputDeviceDataSaver(Misc::createNumberedFileName(inputDeviceDataFileName.c_str(),4,numberedFileName),*inputDeviceManager);
			}
		}
	if(multiplexer!=0)
		multipipeDispatcher=new MultipipeDispatcher(pipe,inputDeviceManager);
	
	/* Initialize the update regime: */
	updateContinuously=configFileSection.retrieveValue<bool>("./updateContinuously",updateContinuously);
	
	/* Initialize the light source manager: */
	lightsourceManager=new LightsourceManager;
	
	/* Initialize the viewers: */
	StringList viewerNames=configFileSection.retrieveValue<StringList>("./viewerNames");
	numViewers=viewerNames.size();
	viewers=new Viewer[numViewers];
	for(int i=0;i<numViewers;++i)
		{
		/* Go to viewers's section: */
		Misc::ConfigurationFileSection viewerSection=configFileSection.getSection(viewerNames[i].c_str());
		
		/* Initialize viewer: */
		viewers[i].initialize(viewerSection);
		}
	mainViewer=&viewers[0];
	
	/* Initialize the screens: */
	StringList screenNames=configFileSection.retrieveValue<StringList>("./screenNames");
	numScreens=screenNames.size();
	screens=new VRScreen[numScreens];
	for(int i=0;i<numScreens;++i)
		{
		/* Go to screen's section: */
		Misc::ConfigurationFileSection screenSection=configFileSection.getSection(screenNames[i].c_str());
		
		/* Initialize screen: */
		screens[i].initialize(screenSection);
		}
	mainScreen=&screens[0];
	
	/* Initialize screen protection: */
	typedef std::vector<ScreenProtector> ScreenProtectorList;
	ScreenProtectorList spl=configFileSection.retrieveValue<ScreenProtectorList>("./screenProtectors",ScreenProtectorList());
	numProtectors=spl.size();
	protectors=new ScreenProtector[numProtectors];
	for(int i=0;i<numProtectors;++i)
		protectors[i]=spl[i];
	
	/* Initialize rendering parameters: */
	frontplaneDist=configFileSection.retrieveValue<Scalar>("./frontplaneDist",frontplaneDist);
	backplaneDist=configFileSection.retrieveValue<Scalar>("./backplaneDist",backplaneDist);
	backgroundColor=configFileSection.retrieveValue<Color>("./backgroundColor",backgroundColor);
	ambientLightColor=configFileSection.retrieveValue<Color>("./ambientLightColor",ambientLightColor);
	
	/* Initialize widget management: */
	widgetMaterial=configFileSection.retrieveValue<GLMaterial>("./widgetMaterial",widgetMaterial);
	
	/* Create Vrui's default widget style sheet: */
	GLFont* font=loadFont(configFileSection.retrieveString("./uiFontName","CenturySchoolbookBoldItalic").c_str());
	font->setTextHeight(configFileSection.retrieveValue<double>("./uiFontTextHeight",1.0*inchScale));
	font->setAntialiasing(configFileSection.retrieveValue<bool>("./uiFontAntialiasing",true));
	uiStyleSheet.setFont(font);
	uiStyleSheet.setSize(configFileSection.retrieveValue<float>("./uiSize",uiStyleSheet.size));
	uiStyleSheet.borderColor=uiStyleSheet.bgColor=configFileSection.retrieveValue<Color>("./uiBgColor",uiStyleSheet.bgColor);
	uiStyleSheet.fgColor=configFileSection.retrieveValue<Color>("./uiFgColor",uiStyleSheet.fgColor);
	uiStyleSheet.textfieldBgColor=configFileSection.retrieveValue<Color>("./uiTextFieldBgColor",uiStyleSheet.textfieldBgColor);
	uiStyleSheet.textfieldFgColor=configFileSection.retrieveValue<Color>("./uiTextFieldFgColor",uiStyleSheet.textfieldFgColor);
	uiStyleSheet.titlebarBgColor=configFileSection.retrieveValue<Color>("./uiTitleBarBgColor",uiStyleSheet.titlebarBgColor);
	uiStyleSheet.titlebarFgColor=configFileSection.retrieveValue<Color>("./uiTitleBarFgColor",uiStyleSheet.titlebarFgColor);
	uiStyleSheet.sliderHandleWidth=configFileSection.retrieveValue<double>("./uiSliderWidth",uiStyleSheet.sliderHandleWidth);
	uiStyleSheet.sliderHandleColor=configFileSection.retrieveValue<Color>("./uiSliderHandleColor",uiStyleSheet.sliderHandleColor);
	uiStyleSheet.sliderShaftColor=configFileSection.retrieveValue<Color>("./uiSliderShaftColor",uiStyleSheet.sliderShaftColor);
	widgetManager=new GLMotif::WidgetManager;
	widgetManager->setStyleSheet(&uiStyleSheet);
	widgetManager->setDrawOverlayWidgets(configFileSection.retrieveValue<bool>("./drawOverlayWidgets",widgetManager->getDrawOverlayWidgets()));
	popWidgetsOnScreen=configFileSection.retrieveValue<bool>("./popWidgetsOnScreen",popWidgetsOnScreen);
	
	/* Create the Vrui system menu and install it as the main menu: */
	systemMenuPopup=new GLMotif::PopupMenu("VruiSystemMenuPopup",widgetManager);
	systemMenuPopup->setTitle("Vrui System");
	GLMotif::Menu* systemMenu=new GLMotif::Menu("VruiSystemMenu",systemMenuPopup,false);
	buildSystemMenu(systemMenu);
	systemMenu->manageChild();
	mainMenu=new MutexMenu(systemMenuPopup);
	
	/* Create the coordinate manager: */
	coordinateManager=new CoordinateManager;
	
	/* Go to tool manager's section: */
	Misc::ConfigurationFileSection toolSection=configFileSection.getSection(configFileSection.retrieveString("./tools").c_str());
	
	/* Initialize tool manager: */
	toolManager=new ToolManager(inputDeviceManager,toolSection);
	
	try
		{
		/* Go to vislet manager's section: */
		Misc::ConfigurationFileSection visletSection=configFileSection.getSection(configFileSection.retrieveString("./vislets").c_str());
		
		/* Initialize vislet manager: */
		visletManager=new VisletManager(visletSection);
		}
	catch(std::runtime_error err)
		{
		/* Ignore error and continue... */
		}
	
	/* Initialize random number management: */
	if(master)
		randomSeed=(unsigned int)time(0);
	if(multiplexer!=0)
		{
		pipe->broadcast<unsigned int>(randomSeed);
		pipe->finishMessage();
		}
	srand(randomSeed);
	
	/* Initialize the application timer: */
	if(master)
		lastFrame=appTime.peekTime();
	if(multiplexer!=0)
		{
		pipe->broadcast<double>(lastFrame);
		pipe->finishMessage();
		}
	numRecentFrameTimes=5;
	recentFrameTimes=new double[numRecentFrameTimes];
	for(int i=0;i<numRecentFrameTimes;++i)
		recentFrameTimes[i]=0.0;
	nextFrameTimeIndex=0;
	sortedFrameTimes=new double[numRecentFrameTimes];
	currentFrameTime=0.0;
	}

void VruiState::initTools(const Misc::ConfigurationFileSection&)
	{
	/* Create default tool assignment: */
	toolManager->loadDefaultTools();
	}

void VruiState::initSound(ALContextData& contextData)
	{
	}

void VruiState::update(void)
	{
	#if RENDERFRAMETIMES
	double lastLastFrame=lastFrame;
	#endif
	if(master)
		{
		/* Update frame time: */
		double frame=appTime.peekTime();
		recentFrameTimes[nextFrameTimeIndex]=frame-lastFrame;
		lastFrame=frame;
		++nextFrameTimeIndex;
		if(nextFrameTimeIndex==numRecentFrameTimes)
			nextFrameTimeIndex=0;
		
		/* Calculate current frame time: */
		for(int i=0;i<numRecentFrameTimes;++i)
			{
			int j;
			for(j=i-1;j>=0&&sortedFrameTimes[j]>recentFrameTimes[i];--j)
				sortedFrameTimes[j+1]=sortedFrameTimes[j];
			sortedFrameTimes[j+1]=recentFrameTimes[i];
			}
		currentFrameTime=sortedFrameTimes[numRecentFrameTimes/2];
		}
	if(multiplexer!=0)
		{
		pipe->broadcast<double>(lastFrame);
		pipe->broadcast<double>(currentFrameTime);
		}
	#if RENDERFRAMETIMES
	++frameTimeIndex;
	if(frameTimeIndex==numFrameTimes)
		frameTimeIndex=0;
	frameTimes[frameTimeIndex]=lastFrame-lastLastFrame;
	#endif
	
	#if DELAY_NAVIGATIONTRANSFORMATION
	if(navigationTransformationEnabled&&navigationTransformationChanged)
		{
		/* Update the navigation transformation from the last frame: */
		navigationTransformation=newNavigationTransformation;
		inverseNavigationTransformation=Geometry::invert(navigationTransformation);
		navigationTransformationChanged=false;
		}
	#endif
	
	/* Update all physical input devices: */
	if(master)
		inputDeviceManager->updateInputDevices();
	if(multiplexer!=0)
		{
		multipipeDispatcher->dispatchState();
		pipe->finishMessage();
		}
	
	#if SAVESHAREDVRUISTATE
	/* Save shared state to a local file for post-mortem analysis purposes: */
	vruiSharedStateFile->write<double>(lastFrame);
	vruiSharedStateFile->write<double>(currentFrameTime);
	int numInputDevices=inputDeviceManager->getNumInputDevices();
	vruiSharedStateFile->write<int>(numInputDevices);
	for(int i=0;i<numInputDevices;++i)
		{
		InputDevice* id=inputDeviceManager->getInputDevice(i);
		vruiSharedStateFile->write<Scalar>(id->getPosition().getComponents(),3);
		vruiSharedStateFile->write<Scalar>(id->getOrientation().getQuaternion(),4);
		}
	#endif
	
	/* Save input device states to data file if requested: */
	if(master&&inputDeviceDataSaver!=0)
		inputDeviceDataSaver->saveCurrentState(lastFrame);
	
	/* Update the input graph: */
	inputGraphManager->update();
	
	/* Update the tool manager: */
	toolManager->update();
	
	/* Update viewer states: */
	for(int i=0;i<numViewers;++i)
		viewers[i].update();
	
	/* Call frame functions of all loaded vislets: */
	if(visletManager!=0)
		visletManager->frame();
	
	/* Call frame function: */
	frameFunction(frameFunctionData);
	
	/* Finish any pending messages on the main pipe, in case an application didn't clean up: */
	if(multiplexer!=0)
		pipe->finishMessage();
	}

void VruiState::display(GLContextData& contextData) const
	{
	/* Initialize standard OpenGL settings: */
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
	glDisable(GL_COLOR_MATERIAL);
	glMatrixMode(GL_MODELVIEW);
	
	/* Clear the display and Z-buffer: */
	glClearColor(backgroundColor);
	glClearDepth(1.0); // Clear depth is "infinity"
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); // Clear color- and Z-buffer
	
	/* Enable ambient light source: */
	glLightModelAmbient(ambientLightColor);
	
	/* Set light sources: */
	if(navigationTransformationEnabled)
		lightsourceManager->setLightsources(navigationTransformation,contextData);
	else
		lightsourceManager->setLightsources(contextData);
	
	/* Render input graph state: */
	inputGraphManager->glRenderAction(contextData);
	
	/* Render tool manager's state: */
	toolManager->glRenderAction(contextData);
	
	/* Display any realized widgets: */
	glMaterial(GLMaterialEnums::FRONT,widgetMaterial);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
	widgetManager->draw(contextData);
	glDisable(GL_COLOR_MATERIAL);
	
	/* Display all loaded vislets: */
	if(visletManager!=0)
		visletManager->display(contextData);
	
	/* Call the user display function: */
	if(displayFunction!=0)
		{
		glPushMatrix();
		if(navigationTransformationEnabled)
			glMultMatrix(navigationTransformation);
		displayFunction(contextData,displayFunctionData);
		glPopMatrix();
		}
	
	/* Execute the transparency rendering pass: */
	if(TransparentObject::needRenderPass())
		{
		/* Set up OpenGL state for transparency: */
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(GL_FALSE);
		
		/* Execute transparent rendering pass: */
		TransparentObject::transparencyPass(contextData);
		}
	}

void VruiState::sound(ALContextData& contextData) const
	{
	/* Call the user sound function: */
	if(soundFunction!=0)
		soundFunction(contextData,soundFunctionData);
	}

void VruiState::loadViewCallback(Misc::CallbackData* cbData)
	{
	/* Only load if no navigation tools are active: */
	if(activeNavigationTool==0)
		loadViewpointFile("Viewpoint.dat");
	}

void VruiState::saveViewCallback(Misc::CallbackData* cbData)
	{
	/* Push the current navigation transformation onto the stack of navigation transformations: */
	storedNavigationTransformations.push_back(getNavigationTransformation());
	
	if(isMaster())
		{
		try
			{
			/* Create a uniquely named viewpoint file: */
			char numberedFileName[40];
			Misc::File viewpointFile(Misc::createNumberedFileName("SavedViewpoint.dat",4,numberedFileName),"wb",Misc::File::LittleEndian);
			
			/* Write a header identifying this as an environment-independent viewpoint file: */
			fprintf(viewpointFile.getFilePtr(),"Vrui viewpoint file v1.0\n");
			
			/* Write the environment's center point in navigational coordinates: */
			Point center=getInverseNavigationTransformation().transform(getDisplayCenter());
			viewpointFile.write<Scalar>(center.getComponents(),3);
			
			/* Write the environment's size in navigational coordinates: */
			Scalar size=getDisplaySize()*getInverseNavigationTransformation().getScaling();
			viewpointFile.write<Scalar>(size);
			
			/* Write the environment's forward direction in navigational coordinates: */
			Vector forward=getInverseNavigationTransformation().transform(getForwardDirection());
			viewpointFile.write<Scalar>(forward.getComponents(),3);
			
			/* Write the environment's up direction in navigational coordinates: */
			Vector up=getInverseNavigationTransformation().transform(getUpDirection());
			viewpointFile.write<Scalar>(up.getComponents(),3);
			}
		catch(Misc::File::OpenError err)
			{
			/* Ignore errors if viewpoint file could not be created */
			}
		}
	}

void VruiState::restoreViewCallback(Misc::CallbackData* cbData)
	{
	/* Only restore if no navigation tools are active and the stack is not empty: */
	if(activeNavigationTool==0&&!storedNavigationTransformations.empty())
		{
		/* Pop the most recently stored navigation transformation off the stack: */
		setNavigationTransformation(storedNavigationTransformations.back());
		storedNavigationTransformations.pop_back();
		}
	}

void VruiState::createInputDeviceCallback(Misc::CallbackData* cbData)
	{
	/* Create a new one-button virtual input device: */
	createdVirtualInputDevices.push_back(addVirtualInputDevice("VirtualInputDevice",1,0));
	}

void VruiState::destroyInputDeviceCallback(Misc::CallbackData* cbData)
	{
	/* Destroy the oldest virtual input device: */
	if(!createdVirtualInputDevices.empty())
		{
		getInputDeviceManager()->destroyInputDevice(createdVirtualInputDevices.front());
		createdVirtualInputDevices.pop_front();
		}
	}

void VruiState::showScaleBarToggleCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	}

void VruiState::quitCallback(Misc::CallbackData* cbData)
	{
	/* Request Vrui to shut down cleanly: */
	shutdown();
	}

/********************************
Global Vrui kernel API functions:
********************************/

void vruiDelay(double interval)
	{
	#ifdef __SGI_IRIX__
	long intervalCount=(long)(interval*(double)CLK_TCK+0.5);
	while(intervalCount>0)
		intervalCount=sginap(intervalCount);
	#else
	int seconds=int(floor(interval));
	interval-=double(seconds);
	int microseconds=int(floor(interval*1000000.0+0.5));
	struct timeval tv;
	tv.tv_sec=seconds;
	tv.tv_usec=microseconds;
	select(0,0,0,0,&tv);
	#endif
	}

/**********************************
Call-in functions for user program:
**********************************/

void setFrameFunction(FrameFunctionType frameFunction,void* userData)
	{
	vruiState->frameFunction=frameFunction;
	vruiState->frameFunctionData=userData;
	}

void setDisplayFunction(DisplayFunctionType displayFunction,void* userData)
	{
	vruiState->displayFunction=displayFunction;
	vruiState->displayFunctionData=userData;
	}

void setSoundFunction(SoundFunctionType soundFunction,void* userData)
	{
	vruiState->soundFunction=soundFunction;
	vruiState->soundFunctionData=userData;
	}

bool isMaster(void)
	{
	return vruiState->master;
	}

int getNodeIndex(void)
	{
	if(vruiState->multiplexer!=0)
		return vruiState->multiplexer->getNodeIndex();
	else
		return 0;
	}

int getNumNodes(void)
	{
	if(vruiState->multiplexer!=0)
		return vruiState->multiplexer->getNumNodes();
	else
		return 1;
	}

Comm::MulticastPipe* getMainPipe(void)
	{
	return vruiState->pipe;
	}

Comm::MulticastPipe* openPipe(void)
	{
	if(vruiState->multiplexer!=0)
		return vruiState->multiplexer->openPipe();
	else
		return 0;
	}

GlyphRenderer* getGlyphRenderer(void)
	{
	return vruiState->glyphRenderer;
	}

void renderGlyph(const Glyph& glyph,const OGTransform& transformation,GLContextData& contextData)
	{
	vruiState->glyphRenderer->renderGlyph(glyph,transformation,vruiState->glyphRenderer->getContextDataItem(contextData));
	}

VirtualInputDevice* getVirtualInputDevice(void)
	{
	return vruiState->virtualInputDevice;
	}

InputGraphManager* getInputGraphManager(void)
	{
	return vruiState->inputGraphManager;
	}

InputDeviceManager* getInputDeviceManager(void)
	{
	return vruiState->inputDeviceManager;
	}

int getNumInputDevices(void)
	{
	return vruiState->inputDeviceManager->getNumInputDevices();
	}

InputDevice* getInputDevice(int index)
	{
	return vruiState->inputDeviceManager->getInputDevice(index);
	}

InputDevice* findInputDevice(const char* name)
	{
	return vruiState->inputDeviceManager->findInputDevice(name);
	}

InputDevice* addVirtualInputDevice(const char* name,int numButtons,int numValuators)
	{
	InputDevice* newDevice=vruiState->inputDeviceManager->createInputDevice(name,InputDevice::TRACK_POS|InputDevice::TRACK_DIR|InputDevice::TRACK_ORIENT,numButtons,numValuators);
	newDevice->setTransformation(TrackerState::translateFromOriginTo(vruiState->newInputDevicePosition));
	newDevice->setDeviceRayDirection(Vector(0.0,1.0,0.0));
	vruiState->inputGraphManager->getInputDeviceGlyph(newDevice).enable(Glyph::BOX,vruiState->widgetMaterial);
	return newDevice;
	}

LightsourceManager* getLightsourceManager(void)
	{
	return vruiState->lightsourceManager;
	}

Viewer* getMainViewer(void)
	{
	return vruiState->mainViewer;
	}

int getNumViewers(void)
	{
	return vruiState->numViewers;
	}

Viewer* getViewer(int index)
	{
	return &vruiState->viewers[index];
	}

Viewer* findViewer(const char* name)
	{
	Viewer* result=0;
	for(int i=0;i<vruiState->numViewers;++i)
		if(strcmp(name,vruiState->viewers[i].getName())==0)
			{
			result=&vruiState->viewers[i];
			break;
			}
	return result;
	}

VRScreen* getMainScreen(void)
	{
	return vruiState->mainScreen;
	}

int getNumScreens(void)
	{
	return vruiState->numScreens;
	}

VRScreen* getScreen(int index)
	{
	return vruiState->screens+index;
	}

VRScreen* findScreen(const char* name)
	{
	VRScreen* result=0;
	for(int i=0;i<vruiState->numScreens;++i)
		if(strcmp(name,vruiState->screens[i].getName())==0)
			{
			result=&vruiState->screens[i];
			break;
			}
	return result;
	}

std::pair<VRScreen*,Scalar> findScreen(const Ray& ray)
	{
	/* Find the closest intersection with any screen: */
	VRScreen* closestScreen=0;
	Scalar closestLambda=Math::Constants<Scalar>::max;
	for(int screenIndex=0;screenIndex<vruiState->numScreens;++screenIndex)
		{
		VRScreen* screen=&vruiState->screens[screenIndex];
		
		/* Calculate screen plane: */
		ONTransform t=screen->getScreenTransformation();
		Vector screenNormal=t.getDirection(2);
		Scalar screenOffset=screenNormal*t.getOrigin();
		
		/* Intersect selection ray with screen plane: */
		Scalar divisor=screenNormal*ray.getDirection();
		if(divisor!=Scalar(0))
			{
			Scalar lambda=(screenOffset-screenNormal*ray.getOrigin())/divisor;
			if(lambda>=Scalar(0)&&lambda<closestLambda)
				{
				/* Check if the ray intersects the screen: */
				Point screenPos=t.inverseTransform(ray.getOrigin()+ray.getDirection()*lambda);
				if(screenPos[0]>=Scalar(0)&&screenPos[0]<=screen->getWidth()&&screenPos[1]>=Scalar(0)&&screenPos[1]<=screen->getHeight())
					{
					/* Save the intersection: */
					closestScreen=screen;
					closestLambda=lambda;
					}
				}
			}
		}
	
	return std::pair<VRScreen*,Scalar>(closestScreen,closestLambda);
	}

Scalar getInchFactor(void)
	{
	return vruiState->inchScale;
	}

Scalar getDisplaySize(void)
	{
	return vruiState->displaySize;
	}

const Point& getDisplayCenter(void)
	{
	return vruiState->displayCenter;
	}

const Vector& getForwardDirection(void)
	{
	return vruiState->forwardDirection;
	}

const Vector& getUpDirection(void)
	{
	return vruiState->upDirection;
	}

const Plane& getFloorPlane(void)
	{
	return vruiState->floorPlane;
	}

void setFrontplaneDist(Scalar newFrontplaneDist)
	{
	vruiState->frontplaneDist=newFrontplaneDist;
	}

Scalar getFrontplaneDist(void)
	{
	return vruiState->frontplaneDist;
	}

void setBackplaneDist(Scalar newBackplaneDist)
	{
	vruiState->backplaneDist=newBackplaneDist;
	}

Scalar getBackplaneDist(void)
	{
	return vruiState->backplaneDist;
	}

void setBackgroundColor(const Color& newBackgroundColor)
	{
	vruiState->backgroundColor=newBackgroundColor;
	}

const Color& getBackgroundColor(void)
	{
	return vruiState->backgroundColor;
	}

GLFont* loadFont(const char* fontName)
	{
	return new GLFont(fontName);
	}

const GLMotif::StyleSheet* getUiStyleSheet(void)
	{
	return &vruiState->uiStyleSheet;
	}

float getUiSize(void)
	{
	return vruiState->uiStyleSheet.size;
	}

const Color& getUiBgColor(void)
	{
	return vruiState->uiStyleSheet.bgColor;
	}

const Color& getUiFgColor(void)
	{
	return vruiState->uiStyleSheet.fgColor;
	}

const Color& getUiTextFieldBgColor(void)
	{
	return vruiState->uiStyleSheet.textfieldBgColor;
	}

const Color& getUiTextFieldFgColor(void)
	{
	return vruiState->uiStyleSheet.textfieldFgColor;
	}

GLFont* getUiFont(void)
	{
	return vruiState->uiStyleSheet.font;
	}

void setWidgetMaterial(const GLMaterial& newWidgetMaterial)
	{
	vruiState->widgetMaterial=newWidgetMaterial;
	}

const GLMaterial& getWidgetMaterial(void)
	{
	return vruiState->widgetMaterial;
	}

void setMainMenu(GLMotif::PopupMenu* newMainMenu)
	{
	/* Delete old main menu shell and system menu popup: */
	delete vruiState->mainMenu;
	delete vruiState->systemMenuPopup;
	vruiState->systemMenuPopup=0;
	
	/* Add the Vrui system menu to the end of the given main menu: */
	GLMotif::Menu* menuChild=dynamic_cast<GLMotif::Menu*>(newMainMenu->getChild());
	if(menuChild!=0)
		{
		/* Create the Vrui system menu (not saved, because it's deleted automatically by the cascade button): */
		GLMotif::Popup* systemMenuPopup=new GLMotif::Popup("VruiSystemMenuPopup",vruiState->widgetManager);
		GLMotif::SubMenu* systemMenu=new GLMotif::SubMenu("VruiSystemMenu",systemMenuPopup,false);
		vruiState->buildSystemMenu(systemMenu);
		systemMenu->manageChild();
		
		/* Create a cascade button at the end of the main menu: */
		GLMotif::CascadeButton* systemMenuCascade=new GLMotif::CascadeButton("VruiSystemMenuCascade",menuChild,"Vrui System");
		systemMenuCascade->setPopup(systemMenuPopup);
		}
	
	/* Create new main menu shell: */
	vruiState->mainMenu=new MutexMenu(newMainMenu);
	}

MutexMenu* getMainMenu(void)
	{
	return vruiState->mainMenu;
	}

GLMotif::WidgetManager* getWidgetManager(void)
	{
	return vruiState->widgetManager;
	}

void popupPrimaryWidget(GLMotif::Widget* topLevel,const Point& hotSpot)
	{
	typedef GLMotif::WidgetManager::Transformation WTransform;
	typedef WTransform::Point WPoint;
	typedef WTransform::Vector WVector;
	
	WPoint globalHotSpot;
	if(vruiState->navigationTransformationEnabled)
		globalHotSpot=vruiState->inverseNavigationTransformation.transform(hotSpot);
	else
		globalHotSpot=hotSpot;
	
	WTransform widgetTransformation;
	if(vruiState->popWidgetsOnScreen)
		{
		/* Project the global hot spot into the screen plane: */
		const ONTransform screenT=vruiState->mainScreen->getScreenTransformation();
		Point screenHotSpot=screenT.inverseTransform(Point(globalHotSpot));
		// screenHotSpot[1]=Scalar(0);
		
		/* Align the widget with the main screen's plane: */
		widgetTransformation=WTransform(screenT);
		widgetTransformation*=WTransform::translateFromOriginTo(screenHotSpot);
		}
	else
		{
		/* Align the widget with the viewing direction: */
		WVector viewDirection=globalHotSpot-vruiState->mainViewer->getHeadPosition();
		WVector x=Geometry::cross(viewDirection,vruiState->upDirection);
		WVector y=Geometry::cross(x,viewDirection);
		widgetTransformation=WTransform::translateFromOriginTo(globalHotSpot);
		WTransform::Rotation rot=WTransform::Rotation::fromBaseVectors(x,y);
		widgetTransformation*=WTransform::rotate(rot);
		}
	
	/* Center the widget on the given hot spot: */
	WVector widgetOffset;
	for(int i=0;i<3;++i)
		widgetOffset[i]=topLevel->getExterior().origin[i]+0.5*topLevel->getExterior().size[i];
	widgetTransformation*=WTransform::translate(-widgetOffset);
	
	/* Pop up the widget: */
	vruiState->widgetManager->popupPrimaryWidget(topLevel,widgetTransformation);
	}

void popupPrimaryScreenWidget(GLMotif::Widget* topLevel,Scalar x,Scalar y)
	{
	typedef GLMotif::WidgetManager::Transformation WTransform;
	typedef WTransform::Vector WVector;
	
	/* Calculate a transformation moving the widget to its given position on the screen: */
	Scalar screenX=x*(vruiState->mainScreen->getWidth()-topLevel->getExterior().size[0]);
	Scalar screenY=y*(vruiState->mainScreen->getHeight()-topLevel->getExterior().size[1]);
	WTransform widgetTransformation=vruiState->mainScreen->getTransform();
	widgetTransformation*=WTransform::translate(WVector(screenX,screenY,vruiState->inchScale));
	
	/* Pop up the widget: */
	vruiState->widgetManager->popupPrimaryWidget(topLevel,widgetTransformation);
	}

void popdownPrimaryWidget(GLMotif::Widget* topLevel)
	{
	/* Pop down the widget: */
	vruiState->widgetManager->popdownWidget(topLevel);
	}

void setNavigationTransformation(const NavTransform& newNavigationTransformation)
	{
	vruiState->navigationTransformationEnabled=true;
	#if DELAY_NAVIGATIONTRANSFORMATION
	vruiState->newNavigationTransformation=newNavigationTransformation;
	vruiState->newNavigationTransformation.renormalize();
	vruiState->navigationTransformationChanged=true;
	requestUpdate();
	#else
	vruiState->navigationTransformation=newNavigationTransformation;
	#endif
	}

void setNavigationTransformation(const Point& center,Scalar radius)
	{
	NavTransform t=NavTransform::translateFromOriginTo(vruiState->displayCenter);
	t*=NavTransform::scale(vruiState->displaySize/radius);
	t*=NavTransform::translateToOriginFrom(center);
	vruiState->navigationTransformationEnabled=true;
	#if DELAY_NAVIGATIONTRANSFORMATION
	vruiState->newNavigationTransformation=t;
	vruiState->navigationTransformationChanged=true;
	requestUpdate();
	#else
	vruiState->navigationTransformation=t;
	#endif
	}

void setNavigationTransformation(const Point& center,Scalar radius,const Vector& up)
	{
	NavTransform t=NavTransform::translateFromOriginTo(vruiState->displayCenter);
	t*=NavTransform::scale(vruiState->displaySize/radius);
	t*=NavTransform::rotate(NavTransform::Rotation::rotateFromTo(up,vruiState->upDirection));
	t*=NavTransform::translateToOriginFrom(center);
	vruiState->navigationTransformationEnabled=true;
	#if DELAY_NAVIGATIONTRANSFORMATION
	vruiState->newNavigationTransformation=t;
	vruiState->navigationTransformationChanged=true;
	requestUpdate();
	#else
	vruiState->navigationTransformation=t;
	#endif
	}

void concatenateNavigationTransformation(const NavTransform& t)
	{
	#if DELAY_NAVIGATIONTRANSFORMATION
	vruiState->newNavigationTransformation*=t;
	vruiState->newNavigationTransformation.renormalize();
	vruiState->navigationTransformationChanged=true;
	requestUpdate();
	#else
	vruiState->navigationTransformation*=t;
	#endif
	}

void concatenateNavigationTransformationLeft(const NavTransform& t)
	{
	#if DELAY_NAVIGATIONTRANSFORMATION
	vruiState->newNavigationTransformation.leftMultiply(t);
	vruiState->newNavigationTransformation.renormalize();
	vruiState->navigationTransformationChanged=true;
	requestUpdate();
	#else
	vruiState->navigationTransformation*=t;
	#endif
	}

const NavTransform& getNavigationTransformation(void)
	{
	if(vruiState->navigationTransformationEnabled)
		return vruiState->navigationTransformation;
	else
		return NavTransform::identity;
	}

const NavTransform& getInverseNavigationTransformation(void)
	{
	if(vruiState->navigationTransformationEnabled)
		return vruiState->inverseNavigationTransformation;
	else
		return NavTransform::identity;
	}

void disableNavigationTransformation(void)
	{
	vruiState->navigationTransformationEnabled=false;
	}

Point getHeadPosition(void)
	{
	if(vruiState->navigationTransformationEnabled)
		return vruiState->inverseNavigationTransformation.transform(vruiState->mainViewer->getHeadPosition());
	else
		return vruiState->mainViewer->getHeadPosition();
	}

Vector getViewDirection(void)
	{
	if(vruiState->navigationTransformationEnabled)
		return vruiState->inverseNavigationTransformation.transform(vruiState->mainViewer->getViewDirection());
	else
		return vruiState->mainViewer->getViewDirection();
	}

Point getDevicePosition(InputDevice* device)
	{
	if(vruiState->navigationTransformationEnabled)
		return vruiState->inverseNavigationTransformation.transform(device->getPosition());
	else
		return device->getPosition();
	}

NavTrackerState getDeviceTransformation(InputDevice* device)
	{
	if(vruiState->navigationTransformationEnabled)
		return vruiState->inverseNavigationTransformation*NavTransform(device->getTransformation());
	else
		return device->getTransformation();
	}

CoordinateManager* getCoordinateManager(void)
	{
	return vruiState->coordinateManager;
	}

ToolManager* getToolManager(void)
	{
	return vruiState->toolManager;
	}

bool activateNavigationTool(const Tool* tool)
	{
	/* Can not activate the given tool if navigation is disabled: */
	if(!vruiState->navigationTransformationEnabled)
		return false;
	
	/* Can not activate the given tool if another navigation tool is already active: */
	if(vruiState->activeNavigationTool!=0&&vruiState->activeNavigationTool!=tool)
		return false;
	
	/* Activate the given tool: */
	vruiState->activeNavigationTool=tool;
	return true;
	}

void deactivateNavigationTool(const Tool* tool)
	{
	/* If the given tool is currently active, deactivate it: */
	if(vruiState->activeNavigationTool==tool)
		vruiState->activeNavigationTool=0;
	}

VisletManager* getVisletManager(void)
	{
	return vruiState->visletManager;
	}

double getApplicationTime(void)
	{
	return vruiState->lastFrame;
	}

double getCurrentFrameTime(void)
	{
	return vruiState->currentFrameTime;
	}

void updateContinuously(void)
	{
	vruiState->updateContinuously=true;
	}

}
