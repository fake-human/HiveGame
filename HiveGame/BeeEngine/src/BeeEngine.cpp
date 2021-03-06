/*  This file is part of HiveGame.
    Copyright(C) 2011 Anonymous <fake0mail0@gmail.com>

    HiveGame is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    HiveGame is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with HiveGame.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <clocale>

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>

#include <OgreErrorDialog.h>

#include "BeeEngine.hpp"

#include "config.h"

using namespace std;
using namespace boost::filesystem;
using namespace Ogre;


typedef std::vector<path> pathlist;

// Icon ID
#define IDR_MAINFRAME 101

String ToLocalCodepage(const String str)
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	// Ш1ИДОШ5 CANNOT INTO UNICODE
	String res(str.length(), 0x20);
	wchar_t* buff = new wchar_t[str.length()];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buff, str.length());
	WideCharToMultiByte(1251, 0, buff, str.length(), 
		const_cast<char*>(res.c_str()), str.length(), NULL, NULL);
	delete[] buff;
	return res;
#else
	return String(str);
#endif  // OGRE_PLATFORM == OGRE_PLATFORM_WIN32
}

BeeEngine::BeeEngine()
 : mRoot(NULL),
 mSceneMgr(NULL),
 mWindow(NULL),
 mTrayMgr(NULL),
 mCharacterMan(NULL),
 mDetailsPanel(NULL),
 mCursorWasVisible(false),
 mShutDown(false),
 mInputManager(NULL),
 mMouse(NULL),
 mKeyboard(NULL)
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	setlocale(LC_ALL, "");
#endif
}

BeeEngine::~BeeEngine()
{
	if(mTrayMgr)
		delete mTrayMgr;
	if(mCharacterMan)
		delete mCharacterMan;

	boxes.clear();

	delete World::getSingletonPtr();

	// Удаляемся из Window listener
	WindowEventUtilities::removeWindowEventListener(mWindow, this);
	windowClosed(mWindow);
	delete mRoot;
}

bool BeeEngine::configure()
{
	// Показать диалог конфигурации
	// TODO : root.restoreConfig()
	if(mRoot->showConfigDialog())
	{
		// Щёлкнули Ok, создаём окно
		mWindow = mRoot->initialise(true, "BeeEngine Render Window");

		// Выставляем правильную иконку у окна
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		HWND hwnd;
		mWindow->getCustomAttribute("WINDOW", (void*)&hwnd);
		LONG iconID =
			(LONG)LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDR_MAINFRAME));
		SetClassLong(hwnd, GCL_HICON, iconID);
#endif
		return true;
	}
	else
	{
		return false;
	}
}

void BeeEngine::chooseSceneManager()
{
	mSceneMgr = mRoot->createSceneManager(ST_GENERIC);
}

void BeeEngine::setupInput()
{
	LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
	OIS::ParamList pl;
	size_t windowHnd = 0;
	ostringstream windowHndStr;

	mWindow->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;
	pl.insert(make_pair(string("WINDOW"), windowHndStr.str()));

	mInputManager = OIS::InputManager::createInputSystem( pl );

	mKeyboard = static_cast<OIS::Keyboard*>(
		mInputManager->createInputObject(OIS::OISKeyboard, true));
	mMouse = static_cast<OIS::Mouse*>(
		mInputManager->createInputObject(OIS::OISMouse, true));

	mMouse->setEventCallback(this);
	mKeyboard->setEventCallback(this);
}

void BeeEngine::setupCharacter()
{
	mCharacterMan = new CharacterManager(mCamera);
}

void BeeEngine::createCamera()
{
	mCamera = mSceneMgr->createCamera("PlayerCamera");
	mCamera->setNearClipDistance(0.1f);
	mCamera->setFarClipDistance(9999*6);
}

void BeeEngine::createFrameListener()
{
	// Установить начальную область для мыши
	windowResized(mWindow);

	// Зарегистрироваться в качестве слушателя окна
	WindowEventUtilities::addWindowEventListener(mWindow, this);

	mTrayMgr = new OgreBites::SdkTrayManager(
		"InterfaceName", mWindow, mMouse, this);
	mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
	mTrayMgr->hideCursor();

	// Сделать отладочную панельку
	StringVector items;
	items.push_back("cam.pX");
	items.push_back("cam.pY");
	items.push_back("cam.pZ");
	items.push_back("");
	items.push_back("cam.oW");
	items.push_back("cam.oX");
	items.push_back("cam.oY");
	items.push_back("cam.oZ");
	items.push_back("");
	items.push_back("Filtering");
	items.push_back("Poly Mode");
	items.push_back("");
	items.push_back("Press B");
	items.push_back("");
	items.push_back("Time");

	mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE,
		"DetailsPanel", 220, items);
	mDetailsPanel->setParamValue(9, "Bilinear");
	mDetailsPanel->setParamValue(10, "Solid");
//	mDetailsPanel->hide();

	Vector3 gravity(.0f, -9.81f, .0f);
	AxisAlignedBox bounds(Vector3(-10000, -10000, -10000),  Vector3 (10000,  10000,  10000));
	mRoot->addFrameListener(new World(mSceneMgr, mViewport, mResourcesDir, gravity, bounds));

	mRoot->addFrameListener(this);
}

void BeeEngine::destroyScene()
{
}

void BeeEngine::createViewports()
{
	// Создать один вьюпорт на всё окно
	mViewport = mWindow->addViewport(mCamera);

	// Подогнать соотношение сторон камеры под вьюпорт
	mCamera->setAspectRatio(
		Real(mViewport->getActualWidth()) / mViewport->getActualHeight());
}

void BeeEngine::setupResources()
{
	// BUG : В windows сразу после запуска current_path(), скорее всего, будет
	//  совпадать с каталогом, где лежит бинарник. В Linux'е часто не так: для
	//  запуска хайвоигры придётся переходить в её каталог.
	//  Хотя, если под Linux сделать .desktop-файл, то будет ок.
	path res_path = current_path().parent_path();
	res_path /= "data";
	if(!exists(res_path) || !is_directory(res_path))
		OGRE_EXCEPT(Ogre::Exception::ERR_FILE_NOT_FOUND, "Не обнаружена директория с \
данными игры.\nСкачайте архив data.zip с https://github.com/fake-human/HiveGame\
 и распакуйте его в директорию \"data\" в директории с игрой.\n", "BeeEngine");

	// Добавляем в менеджер ресурсов директорию $(PWD)/../data
	mResourcesDir = res_path.string();
	ResourceGroupManager::getSingleton().addResourceLocation(
		mResourcesDir,
		"FileSystem",
		ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	// Смотрим, есть ли в ней что-нибудь
	pathlist files;
	copy(directory_iterator(res_path),
		directory_iterator(), back_inserter(files));
	if(files.empty())
		OGRE_EXCEPT(Ogre::Exception::ERR_FILE_NOT_FOUND, "Данные игры не найдены.\n\
Скачайте архив data.zip с https://github.com/fake-human/HiveGame и распакуйте \
его в директорию \"data\" в директории с игрой.\n", "BeeEngine");

	static const char* std_dirs[] =
		{ "maps", "models", "materials", "shaders", "sounds", "ui" };
	const char** std_dirs_end = std_dirs + sizeof(std_dirs) / sizeof(char*);
	for(pathlist::const_iterator it(files.begin()); it != files.end(); ++it)
		try
		{
			// Добавляем в менеджер ресурсов субдиректории data первого уровня
			if( is_directory(*it) && find(std_dirs, std_dirs_end,
				(*it).filename()) != std_dirs_end )
					ResourceGroupManager::getSingleton()
					.addResourceLocation(
						(*it).string(),
						"FileSystem",
						ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			// И zip-архивы data/*.zip
			else if((*it).extension() == ".zip")
				ResourceGroupManager::getSingleton().addResourceLocation(
					(*it).string(),
					"Zip",
					ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
					true );
		} catch(Ogre::Exception& e) {
			LogManager::getSingleton().logMessage(LML_NORMAL,
				"Unable to load zip pack \"" + (*it).string() +
				"\", try re-downloading/repacking. " + e.getFullDescription());
		}
}

void BeeEngine::createResourceListener()
{
}

void BeeEngine::loadResources()
{
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void BeeEngine::createGUI()
{
	mGUIPlatform = new MyGUI::OgrePlatform();
	mGUIPlatform->initialise(mWindow, mSceneMgr);
	mGUI = new MyGUI::Gui();
	mGUI->initialise();
}

void BeeEngine::run()
{
	if(!setup())
		return;

	mRoot->startRendering();

	destroyScene();
}

int BeeEngine::exec()
{
	try
	{
		run();
	} catch(Exception& e)
	{
		ErrorDialog dialog;
		dialog.display(ToLocalCodepage(e.getFullDescription()));
		return e.getNumber();
	}
	return 0;
}

bool BeeEngine::setup()
{
	// TODO : ogre.cfg хранить в своём конфиге
	mRoot = new Root(StringUtil::BLANK, "ogre.cfg", "BeeEngine.log");

	loadPlugins(PLUGINS_DIR);

	LogManager::getSingleton().setLogDetail(LL_BOREME);

	setupResources();

	if(!configure())
		return false;

	chooseSceneManager();
	setupInput();
	createCamera();
	createViewports();

	// Установить умолчальный уровень mipmap
	TextureManager::getSingleton().setDefaultNumMipmaps(5);

	// Создать слушателей ресурсов (для экрана загрузки)
	createResourceListener();

	loadResources();
	createGUI();
	createFrameListener();
	setupCharacter();
	createScene();

	return true;
};

bool BeeEngine::loadPlugins(const String& plugins_dir)
{
	String platform_debug_suffix, debug_suffix;
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
# ifdef _DEBUG
	platform_debug_suffix = "_d";
# endif
#endif
#ifdef _DEBUG
	debug_suffix = "_d";
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	mRoot->loadPlugin(plugins_dir + "RenderSystem_Direct3D9" + platform_debug_suffix);
	// TODO : Direct3D 10 и 11
#endif
	mRoot->loadPlugin(plugins_dir + "RenderSystem_GL" + platform_debug_suffix);
	mRoot->loadPlugin(plugins_dir + "Plugin_CgProgramManager" + platform_debug_suffix);
	mRoot->loadPlugin("./OgreOggSound" + debug_suffix);

	return true; // TODO : обработку исключений с просьбой переустановить
}

bool BeeEngine::frameRenderingQueued(const FrameEvent& evt)
{
	if(mWindow->isClosed())
		return false;

	if(mShutDown)
		return false;

	// Захватываем все девайсы
	mKeyboard->capture();
	mMouse->capture();

	mTrayMgr->frameRenderingQueued(evt);

	if(!mTrayMgr->isDialogVisible())
	{
		// Если отладочная панелька видна, обновить её
		if(mDetailsPanel->isVisible())
		{
			mDetailsPanel->setParamValue(0,
				StringConverter::toString(mCamera->getDerivedPosition().x));
			mDetailsPanel->setParamValue(1,
				StringConverter::toString(mCamera->getDerivedPosition().y));
			mDetailsPanel->setParamValue(2,
				StringConverter::toString(mCamera->getDerivedPosition().z));
			mDetailsPanel->setParamValue(4,
				StringConverter::toString(mCamera->getDerivedOrientation().w));
			mDetailsPanel->setParamValue(5,
				StringConverter::toString(mCamera->getDerivedOrientation().x));
			mDetailsPanel->setParamValue(6,
				StringConverter::toString(mCamera->getDerivedOrientation().y));
			mDetailsPanel->setParamValue(7,
				StringConverter::toString(mCamera->getDerivedOrientation().z));
			mDetailsPanel->setParamValue(12, "get COPROCUBE!");

			int hour, minute, second;
			World::getSingleton().getTime(hour, minute, second);
			mDetailsPanel->setParamValue(14, StringConverter::toString(hour)+":"+StringConverter::toString(minute));
		}
	}

	mCharacterMan->update(evt.timeSinceLastFrame);
	return true;
}

bool BeeEngine::keyPressed(const OIS::KeyEvent& arg )
{
	MyGUI::InputManager::getInstance().injectKeyPress(
		MyGUI::KeyCode::Enum(arg.key), arg.text);

	if(mTrayMgr->isDialogVisible())
		return true;

	// По клавише F показываем продвинутую статистику по фреймам
	if(arg.key == OIS::KC_F)
	{
		mTrayMgr->toggleAdvancedFrameStats();
	}
	else if(arg.key == OIS::KC_G) // а по клавише G - совсем продвинутую
	{
		if(mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
		{
			mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
			mDetailsPanel->show();
		}
		else
		{
			mTrayMgr->removeWidgetFromTray(mDetailsPanel);
			mDetailsPanel->hide();
		}
	}
	else if(arg.key == OIS::KC_T) // по T меняем режим рендеринга
	{
		String newVal;
		TextureFilterOptions tfo;
		unsigned int aniso;

		switch(mDetailsPanel->getParamValue(9).asUTF8()[0])
		{
		case 'B':
			newVal = "Trilinear";
			tfo = TFO_TRILINEAR;
			aniso = 1;
			break;
		case 'T':
			newVal = "Anisotropic";
			tfo = TFO_ANISOTROPIC;
			aniso = 8;
			break;
		case 'A':
			newVal = "None";
			tfo = TFO_NONE;
			aniso = 1;
			break;
		default:
			newVal = "Bilinear";
			tfo = TFO_BILINEAR;
			aniso = 1;
		}

		MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
		MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
		mDetailsPanel->setParamValue(9, newVal);
	}
	else if(arg.key == OIS::KC_R) // по клавише R меняем вид рендеринга
	{
		String newVal;
		PolygonMode pm;

		switch(mCamera->getPolygonMode())
		{
		case PM_SOLID:
			newVal = "Wireframe";
			pm = PM_WIREFRAME;
			break;
		case PM_WIREFRAME:
			newVal = "Points";
			pm = PM_POINTS;
			break;
		default:
			newVal = "Solid";
			pm = PM_SOLID;
		}
		mCamera->setPolygonMode(pm);
		mDetailsPanel->setParamValue(10, newVal);
	}
	else if(arg.key == OIS::KC_F5) // по F5 обновляем все текстуры
	{
		TextureManager::getSingleton().reloadAll();
	}
	else if(arg.key == OIS::KC_SYSRQ) // по PrintScreen - скриншот
	{
		mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
	}
	else if(arg.key == OIS::KC_B)  // по B - копрокубик для проверки физики
	{
		static int cntr = 0;
		cntr++;
		Vector3 position = mCamera->getDerivedPosition() +
			mCamera->getDerivedDirection().normalisedCopy() * 10;
		SharedPtr<GameObject> box = World::getSingleton().addBox(
			"test" + StringConverter::toString(cntr), 0.04f);
		box->setMaterial("Cube");
		box->setPosition(position);
		box->setRestitution(0.9f);
		box->setFriction(0.1f);
		boxes.push_back(box);
	}
	else if(arg.key == OIS::KC_ESCAPE)
	{
		mShutDown = true;
	}


	mCharacterMan->injectKeyDown(arg);
	return true;
}

bool BeeEngine::keyReleased(const OIS::KeyEvent& arg)
{
	MyGUI::InputManager::getInstance().injectKeyRelease(
		MyGUI::KeyCode::Enum(arg.key));
	mCharacterMan->injectKeyUp(arg);
	return true;
}

bool BeeEngine::mouseMoved(const OIS::MouseEvent& arg)
{
	MyGUI::InputManager::getInstance().injectMouseMove(
		arg.state.X.abs, arg.state.Y.abs, arg.state.Z.abs);
	if(mTrayMgr->injectMouseMove(arg))
		return true;
	mCharacterMan->injectMouseMove(arg);
	return true;
}

bool BeeEngine::mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id)
{
	MyGUI::InputManager::getInstance().injectMousePress(
		arg.state.X.abs, arg.state.Y.abs, MyGUI::MouseButton::Enum(id));
	if(mTrayMgr->injectMouseDown(arg, id))
		return true;
	mCharacterMan->injectMouseDown(arg, id);
	return true;
}

bool BeeEngine::mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID id)
{
	MyGUI::InputManager::getInstance().injectMouseRelease(
		arg.state.X.abs, arg.state.Y.abs, MyGUI::MouseButton::Enum(id));
	if(mTrayMgr->injectMouseUp(arg, id))
		return true;
	return true;
}

// Подогнать область мыши под окно
void BeeEngine::windowResized(RenderWindow* rw)
{
	unsigned int width, height, depth;
	int left, top;
	rw->getMetrics(width, height, depth, left, top);

	const OIS::MouseState& ms = mMouse->getMouseState();
	ms.width = width;
	ms.height = height;
}

// Отсоединить OIS перед убиранием окна (важно под Linux)
void BeeEngine::windowClosed(RenderWindow* rw)
{
	if(rw == mWindow)
	{
		if( mInputManager )
		{
			mInputManager->destroyInputObject(mMouse);
			mInputManager->destroyInputObject(mKeyboard);

			OIS::InputManager::destroyInputSystem(mInputManager);
			mInputManager = 0;
		}
	}
}

