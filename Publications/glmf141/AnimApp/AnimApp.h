#include "Ogre.h"

class AnimApp : public Ogre::Singleton<AnimApp>, public Ogre::FrameListener {

private:

	Ogre::Root * root; // Objet racine d'Ogre3D
	Ogre::RenderWindow * window;
	Ogre::SceneManager * scMgr;
	Ogre::Camera * camera;
	Ogre::Viewport * viewport;
	Ogre::SceneNode * rootNode;
	Ogre::AnimationState * animState;

    /**
     * Constructeur.
     */
	AnimApp();

	/**
	 * Construit la scène animée.
	 */
    void buildScene();

	/**
	 * Destructeur.
	 */
    ~AnimApp();

public:

    /**
     * Modifie le curseur de temps (pour la durée de l'animation).
     * @return true
     */
    bool frameStarted(const Ogre::FrameEvent & evt);

    /**
     * Instantiation du singleton.
     * Point d'entrée du rendu.
     */
	static AnimApp * instantiate();

};

