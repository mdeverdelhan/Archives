#include <string>
#include <vector>
#include "AnimApp.h"

using namespace std;
using namespace Ogre;

template<>AnimApp * Ogre::Singleton<AnimApp>::ms_Singleton = 0;

/**
 * Constructeur.
 */
AnimApp::AnimApp() : FrameListener() {

	root = new Root("conf/plugins_d.cfg", "conf/ogre.cfg");
	root->restoreConfig();
	window = root->initialise(true, "Anim Player");
	scMgr = root->createSceneManager(ST_GENERIC);
	camera = scMgr->createCamera("Cam");
	viewport = window->addViewport(camera);
	rootNode = scMgr->getRootSceneNode();

	window->reposition(25, 25);
	camera->setNearClipDistance(2);
	root->addFrameListener(this);

    ResourceGroupManager::getSingleton().addResourceLocation("./data", "FileSystem");
    ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	buildScene();

	root->startRendering();
}

/**
 * Construit la scène animée.
 */
void AnimApp::buildScene() {
	Entity * ent = scMgr->createEntity("AnimatedMesh", "Sinbad.mesh");
	scMgr->getRootSceneNode()->attachObject(ent);

	camera->setPosition(0, 2, 12);
	AnimationStateSet * set = ent->getAllAnimationStates();

	AnimationStateIterator iter = set->getAnimationStateIterator();
	vector<string> animSetNames;
	while (iter.hasMoreElements()) {
		animSetNames.push_back(iter.getNext()->getAnimationName());
	}
	animState = ent->getAnimationState(animSetNames.at(0));
	animState->setEnabled(true);
	animState->setLoop(true);
}

/**
 * Destructeur.
 */
AnimApp::~AnimApp() {
	if (root) {
		delete root;
	}
}

/**
 * Modifie le curseur de temps (pour la durée de l'animation).
 * @return true
 */
bool AnimApp::frameStarted(const FrameEvent & evt) {
	animState->addTime(evt.timeSinceLastFrame);
	return true;
}

/**
 * Instantiation du singleton.
 */
AnimApp * AnimApp::instantiate() {
	new AnimApp;
	return ms_Singleton;
}
