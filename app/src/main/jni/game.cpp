#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <sstream>



// Do we want to showcase direct JNI/NDK interaction?
// Undefine this to get real cross-platform code.
// Uncomment this to try JNI access; this seems to be broken in latest NDKs
//#define USE_JNI


#if defined(USE_JNI)
// These headers are only needed for direct NDK/JDK interaction
#include <jni.h>
#include <android/native_activity.h>

// Since we want to get the native activity from SFML, we'll have to use an
// extra header here:
#include <SFML/System/NativeActivity.hpp>

// NDK/JNI sub example - call Java code from native code
int vibrate(sf::Time duration)
{
    // First we'll need the native activity handle
    ANativeActivity *activity = sf::getNativeActivity();

    // Retrieve the JVM and JNI environment
    JavaVM* vm = activity->vm;
    JNIEnv* env = activity->env;

    // First, attach this thread to the main thread
    JavaVMAttachArgs attachargs;
    attachargs.version = JNI_VERSION_1_6;
    attachargs.name = "NativeThread";
    attachargs.group = NULL;
    jint res = vm->AttachCurrentThread(&env, &attachargs);

    if (res == JNI_ERR)
        return EXIT_FAILURE;

    // Retrieve class information
    jclass natact = env->FindClass("android/app/NativeActivity");
    jclass context = env->FindClass("android/content/Context");

    // Get the value of a constant
    jfieldID fid = env->GetStaticFieldID(context, "VIBRATOR_SERVICE", "Ljava/lang/String;");
    jobject svcstr = env->GetStaticObjectField(context, fid);

    // Get the method 'getSystemService' and call it
    jmethodID getss = env->GetMethodID(natact, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jobject vib_obj = env->CallObjectMethod(activity->clazz, getss, svcstr);

    // Get the object's class and retrieve the member name
    jclass vib_cls = env->GetObjectClass(vib_obj);
    jmethodID vibrate = env->GetMethodID(vib_cls, "vibrate", "(J)V");

    // Determine the timeframe
    jlong length = duration.asMilliseconds();

    // Bzzz!
    env->CallVoidMethod(vib_obj, vibrate, length);

    // Free references
    env->DeleteLocalRef(vib_obj);
    env->DeleteLocalRef(vib_cls);
    env->DeleteLocalRef(svcstr);
    env->DeleteLocalRef(context);
    env->DeleteLocalRef(natact);

    // Detach thread again
    vm->DetachCurrentThread();
}
#endif

using namespace sf;


// This is the actual Android example. You don't have to write any platform
// specific code, unless you want to use things not directly exposed.
// ('vibrate()' in this example; undefine 'USE_JNI' above to disable it)


int main(int argc, char *argv[])
{
    const int NUM_BRANCHES = 6;
    sf::Sprite branches[NUM_BRANCHES];

// Where is the player/branch?
// Left or Right
    enum class side { LEFT, RIGHT, NONE };
    side branchPositions[NUM_BRANCHES];

//void updateBranches(int seed);

// Function definition
    auto updateBranches = [&](int seed)
    {
        // Move all the branches down one place
        for (int j = NUM_BRANCHES - 1; j > 0; j--) {
            branchPositions[j] = branchPositions[j - 1];
        }

        // Spawn a new branch at position 0
        // LEFT, RIGHT or NONE
        srand((int)time(0) + seed);
        int r = (rand() % 5);

        switch (r) {
            case 0:
                branchPositions[0] = side::LEFT;
                break;

            case 1:
                branchPositions[0] = side::RIGHT;
                break;

            default:
                branchPositions[0] = side::NONE;
                break;
        }
    };

    bool acceptInput = false;

    sf::VideoMode screen(sf::VideoMode(1280, 720));

    sf::RenderWindow window(screen, "", Style::Fullscreen);
    window.setFramerateLimit(30);

    sf::Texture texture;
    if(!texture.loadFromFile("image.png"))
        return EXIT_FAILURE;

    sf::Sprite image(texture);
    image.setPosition(screen.width / 2, screen.height / 2);
    image.setOrigin(texture.getSize().x/2, texture.getSize().y/2);

    sf::View view = window.getDefaultView();
    // Prepare the background
    sf::Texture textureBacground;
    textureBacground.loadFromFile("background.png");

    sf::Sprite spriteBackground;
    spriteBackground.setTexture(textureBacground);

    spriteBackground.setPosition(view.getViewport().width, view.getViewport().height);

    // Make a tree sprite
    Texture textureTree;
    textureTree.loadFromFile("tree.png");
    Sprite spriteTree;
    spriteTree.setTexture(textureTree);
//    spriteTree.setPosition(400, 0);
    spriteTree.setPosition(200, 0);
    // Prepare the player
    Texture texturePlayer;
    texturePlayer.loadFromFile("player.png");
    Sprite spritePlayer;
    spritePlayer.setTexture(texturePlayer);
    spritePlayer.setPosition(10, 720);

    side playerSide = side::LEFT;


    // Prepare the axe
    Texture textureAxe;
    textureAxe.loadFromFile("axe.png");
    Sprite spriteAxe;
    spriteAxe.setTexture(textureAxe);
    spriteAxe.setPosition(120, 790);

    // Line the axe up with the tree
    const float AXE_POSITION_LEFT = 120;
    const float AXE_POSITION_RIGHT = 400;

    // Prepare the left arrows
    Texture textureLeftArrow;
    textureLeftArrow.loadFromFile("left.png");
    Sprite spriteLeftArrow;
    spriteLeftArrow.setTexture(textureLeftArrow);
    spriteLeftArrow.setPosition(50, 1100);

    // Prepare the right arrows
    Texture textureRightArrow;
    textureRightArrow.loadFromFile("right.png");
    Sprite spriteRightArrow;
    spriteRightArrow.setTexture(textureRightArrow);
    spriteRightArrow.setPosition(550, 1100);

    // Prepare the play button
    Texture texturePlayButton;
    texturePlayButton.loadFromFile("play.png");
    Sprite spritePlayButton;
    spritePlayButton.setTexture(texturePlayButton);
    spritePlayButton.setPosition(600, 20);

    // Prepare the pause button
    Texture texturePauseButton;
    texturePauseButton.loadFromFile("pause.png");
    Sprite spritePauseButton;
    spritePauseButton.setTexture(texturePauseButton);
    spritePauseButton.setPosition(2000, 20);

    // make 3 cloud sprites from 1 texture

    sf::Texture textureCloud;

    // Load 1 new texture
    textureCloud.loadFromFile("cloud.png");
    // 3 New sprites with the same texture

    sf::Sprite spriteCloud1;
    sf::Sprite spriteCloud2;
    sf::Sprite spriteCloud3;
    spriteCloud1.setTexture(textureCloud);
    spriteCloud2.setTexture(textureCloud);
    spriteCloud3.setTexture(textureCloud);

    // Position the clouds off screen

    spriteCloud1.setPosition(200, 200);
    spriteCloud2.setPosition(300, 300);
    spriteCloud3.setPosition(100, 100);

    // Are the clouds currently on screen?

    bool cloud1Active = false;
    bool cloud2Active = false;
    bool cloud3Active = false;

    // How fast is each cloud?
    float cloud1Speed = 0.0f;
    float cloud2Speed = 0.0f;
    float cloud3Speed = 0.0f;


    // Prepare the bee
    sf::Texture textureBee;
    textureBee.loadFromFile("bee.png");

    sf::Sprite spriteBee;
    spriteBee.setTexture(textureBee);
    spriteBee.setPosition(0, 400);


    // Is the bee currently moving?
    bool beeActive = false;
    // How fast can the bee fly
    float beeSpeed = 0.f;
    sf::Font font;

    // Prepare the gravestone
    Texture textureRIP;
    textureRIP.loadFromFile("rip.png");
    Sprite spriteRIP;
    spriteRIP.setTexture(textureRIP);
    spriteRIP.setPosition(2000, 800);


    // Prepare the flying log
    Texture textureLog;
    textureLog.loadFromFile("log.png");
    Sprite spriteLog;
    spriteLog.setTexture(textureLog);
    spriteLog.setPosition(200, 800);

    // Some other useful log related variables
    bool logActive = false;
    float logSpeedX = 1000;
    float logSpeedY = -1500;

    // Draw some text
    int score = 100;
    sf::Text messageText("Touch Screen to Start!", font, 64);
    sf::Text scoreText("Score = 0.", font, 64);

    // We need to choose a font
    sf::Font font1;
    font1.loadFromFile("KOMIKAP_.tff");

    // Set the font to our message
    messageText.setFont(font);
    scoreText.setFont(font);

    //Assign the actual message
    messageText.setString("Touch play to Start!");
    scoreText.setString("Score = 0");

    // Make it really big
    messageText.setCharacterSize(75);
    scoreText.setCharacterSize(50);

    // Choose a color
    messageText.setFillColor(sf::Color::White);
    scoreText.setFillColor(sf::Color::White);



    // Position the text
    sf::FloatRect textRect = messageText.getLocalBounds();
    messageText.setOrigin(textRect.left +
                          textRect.width / 2.0f,
                          textRect.top +
                          textRect.height / 2.0f);
    messageText.setPosition(40, 400);

    scoreText.setPosition(40, 80);


    if (!font.loadFromFile("sansation.ttf"))
        return EXIT_FAILURE;

    sf::Text text("Tap anywhere to move the logo.", font, 64);
    text.setFillColor(sf::Color::Black);
    text.setPosition(10, 10);



    // Prepare 5 branches
    sf::Texture textureBranch;
    textureBranch.loadFromFile("branch.png");

    // Set the texture for each branch sprite
        for (int i = 0; i < NUM_BRANCHES; i++) {
        branches[i].setTexture(textureBranch);
        branches[i].setPosition(-2000, -2000);

        // Set the sprite's origin to dead centre
        // We can then spin it round without changing its position
        branches[i].setOrigin(160, 20);
    }



    // Time bar
    RectangleShape timeBar;
    float timeBarStartWidth = 400;
    float timeBarHeight = 80;
    timeBar.setSize(sf::Vector2f(timeBarStartWidth, timeBarHeight));
    timeBar.setFillColor(sf::Color::Red);
    timeBar.setPosition((700 / 2) - timeBarStartWidth / 2, 10);


    Time gameTimeTotal;
    float timeRemaining = 6.0f;
    float timeBarWidthPerSecond = timeBarStartWidth / timeRemaining;

    // Prepare the sound
    SoundBuffer chopBuffer;
    chopBuffer.loadFromFile("chop.wav");
    Sound chop;
    chop.setBuffer(chopBuffer);

    SoundBuffer deathBuffer;
    deathBuffer.loadFromFile("death.wav");
    Sound death;
    death.setBuffer(deathBuffer);

    // Out of time
    SoundBuffer ootBuffer;
    ootBuffer.loadFromFile("out_of_time.wav");
    Sound outOfTime;
    outOfTime.setBuffer(ootBuffer);




    // Loading canary.wav fails for me for now; haven't had time to test why

    /*sf::Music music;
    if(!music.openFromFile("canary.wav"))
        return EXIT_FAILURE;

    music.play();*/


    sf::Color background = sf::Color::White;
    sf::Clock clock;


    // We shouldn't try drawing to the screen while in background
    // so we'll have to track that. You can do minor background
    // work, but keep battery life in mind.
    bool active = true;
    bool paused = true;

    while (window.isOpen())
    {

        sf::Event event;

        while (active ? window.pollEvent(event) : window.waitEvent(event))
        {

            if (event.type == Event::TouchBegan && !paused)
            {
                // Listen for touch presses again
                acceptInput = true;

                // hide the axe
//                spriteAxe.setPosition(2000,
//                                      spriteAxe.getPosition().y);

            }





            switch (event.type)
            {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Escape)
                        window.close();
                    break;
                case sf::Event::Resized:
                    view.setSize(event.size.width, event.size.height);
                    view.setCenter(event.size.width/2, event.size.height/2);
                    window.setView(view);
                    break;
                case sf::Event::LostFocus:
                    background = sf::Color::Black;
                    break;
                case sf::Event::GainedFocus:
                    background = sf::Color::White;
                    break;

                // On Android MouseLeft/MouseEntered are (for now) triggered,
                // whenever the app loses or gains focus.
                case sf::Event::MouseLeft:
                    active = false;
                    break;
                case sf::Event::MouseEntered:
                    active = true;
                    break;
                    /*
****************************************
Handle the players input
****************************************
*/

                case sf::Event::TouchBegan:





                        image.setPosition(event.touch.x, event.touch.y);
#if defined(USE_JNI)
                        vibrate(sf::milliseconds(10));
#endif


                        }
                    break;
            }
        // transform the touch position from window coordinates to world coordinates
        Vector2f touch = window.mapPixelToCoords(Touch::getPosition(event.touch.finger));

        // retrieve the bounding box of the sprite
        FloatRect boundsLeft = spriteLeftArrow.getGlobalBounds();
        FloatRect boundsRight = spriteRightArrow.getGlobalBounds();

        FloatRect boundsPlay = spritePlayButton.getGlobalBounds();
        FloatRect boundsPause = spritePauseButton.getGlobalBounds();

        if (boundsPlay.contains(touch) && event.type == Event::TouchBegan)
        {
            clock.restart();

            paused = false;
//            spritePauseButton.setPosition(600, 20);
            spritePlayButton.setPosition(2000, 20);
            spriteRIP.setPosition(2000, 20);



            // Reset the time and the score
            score = 0;
            timeRemaining = 6;

            // Make all the branches disappear
            for (int i = 1; i < NUM_BRANCHES; i++)
            {
                branchPositions[i] = side::NONE;
            }

//            // Make sure the gravestone is hidden
//            spriteRIP.setPosition(675, 2000);
//
//            // Make sure the axe is hidden
//            spriteAxe.setPosition(675,2000);

            // Move the player into position
            spritePlayer.setPosition(20, 720);

            // Move the Axe into position
            spriteAxe.setPosition(120, 790);

            acceptInput = true;
        }

        if (boundsPause.contains(touch) && event.type == Event::TouchBegan)
        {
            paused = true;
            spritePlayButton.setPosition(600, 20);
            spritePauseButton.setPosition(2000, 20);

        }
        // Wrap the player controls to
        // Make sure we are accepting input
        if (acceptInput)
        {
            // First handle pressing the right key
            if (boundsLeft.contains(touch))
            {
                playerSide = side::LEFT;

                score++;

                // Add to the amount of time remaining
                timeRemaining += (2 / score) + .15;

                spriteAxe.setPosition(AXE_POSITION_LEFT,
                                      spriteAxe.getPosition().y);

                spritePlayer.setPosition(20, 720);

                // update the branches
                updateBranches(score);

                // set the log flying to the Right
                spriteLog.setPosition(200, 730);
                logSpeedX = 5000;
                logActive = true;

                acceptInput = false;

                // Play a chop sound
                chop.play();
            }

            // Handle the Right cursor key
            if (boundsRight.contains(touch))
            {
                // Make sure the player is on the Right
                playerSide = side::RIGHT;

                score++;

                // Add to the amount of time remaining
                timeRemaining += (2 / score) + .15;

                spriteAxe.setPosition(AXE_POSITION_RIGHT,
                                      spriteAxe.getPosition().y);

                spritePlayer.setPosition(520, 720);

                // update the branches
                updateBranches(score);

                // set the log flying
                spriteLog.setPosition(200, 730);
                logSpeedX = -5000;
                logActive = true;

                acceptInput = false;

                // Play a chop sound
                chop.play();
            }
        }

        /*
            ****************************************
            Update the scene
            ****************************************
            */

        if (!paused)
        {
            // Measure time
            Time dt = clock.restart();

            // Substract from the amount of time reamining
            timeRemaining -= dt.asSeconds();
            // size up the time bar
            timeBar.setSize(Vector2f(timeBarWidthPerSecond *
                                     timeRemaining, timeBarHeight));

            if (timeRemaining <= 0.0f) {
                spritePlayButton.setPosition(600, 20);
                // Pause the game
                paused = true;



                // Change the message shown to the player
                messageText.setString("Out of time time!!");

                // Reposition the text based on its new size
                FloatRect textRect = messageText.getLocalBounds();
                messageText.setOrigin(textRect.left +
                                      textRect.width / 2.0f,
                                      textRect.top +
                                      textRect.height / 2.0f);

                messageText.setPosition(400, 400);

                //Play the out of time sound
                outOfTime.play();

                acceptInput = false;
            }

            // Setup the bee
            if (!beeActive)
            {
                // How fast is the bee
                srand((int)time(0) * 10);
                beeSpeed = (rand() % 200) + 200;

                // How high is the bee
                srand((int)time(0) * 10);
                float height = (rand() % 500) + 500;
                spriteBee.setPosition(1000, height);
                beeActive = true;
            } else
                // Move the bee

            {
                spriteBee.setPosition(
                    spriteBee.getPosition().x -
                    (beeSpeed * dt.asSeconds()),
                    spriteBee.getPosition().y);

                // Has the bee reached the right hand edge of the screen?
                if(spriteBee.getPosition().x < - 100)
                {
                    // Set it up ready to be a whole new cloud next frame
                    beeActive = false;
                }
            }

            // Manage the clouds
            // Cloud 1
            if (!cloud1Active)
            {
                // How fast is the cloud
                srand((int)time(0) *10);
                cloud1Speed = (rand() % 200);

                // How high is the cloud
                srand((int)time(0) * 10);
                float height = (rand() % 150);
                spriteCloud1.setPosition(-200, height);
                cloud1Active = true;
            } else{
                spriteCloud1.setPosition(
                    spriteCloud1.getPosition().x +
                    (cloud1Speed * dt.asSeconds()),
                    spriteCloud1.getPosition().y);

                // Has the cloud reached the right hand edge of the screen?
                if(spriteCloud1.getPosition().x > 720)
                {
                    // Set it up ready to be a whole new cloud next frame
                    cloud1Active = false;
                }
            }
            // Cloud 2
            if (!cloud2Active)
            {
                // How fast is the cloud
                srand((int)time(0) * 20);
                float height = (rand() % 300) - 150;
                cloud2Speed = (rand() % 300) - 150;
                spriteCloud2.setPosition(-200, height);
                cloud2Active = true;
            }
            else
            {
                spriteCloud2.setPosition(
                    spriteCloud2.getPosition().x +
                    (cloud2Speed * dt.asSeconds()),
                    spriteCloud2.getPosition().y);

                // Has the cloud reached the right hand edge of the screen?
                if(spriteCloud2.getPosition().x > 720)
                {
                    // Set it up ready to be a whole new cloud next frame
                    cloud2Active = false;
                }
            }

            if (!cloud3Active)
            {
                // How fast is the cloud
                srand((int)time(0) * 30);
                cloud3Speed = (rand() % 200);

                // How high is the cloud
                srand((int)time(0) * 30);
                float height = (rand() % 450) - 150;
                spriteCloud3.setPosition(-200, height);
                cloud3Active = true;
            } else
            {
                spriteCloud3.setPosition(
                    spriteCloud3.getPosition().x +
                    (cloud3Speed * dt.asSeconds()),
                    spriteCloud3.getPosition().y);

                // Has the cloud reached the right edge of the screen?
                if (spriteCloud3.getPosition().x > 1280) {
                    // Set it up ready to be a whole new cloud next frame
                    cloud3Active = false;
                }
            }

            // Update the score text
            std::stringstream ss;
            ss << "Score = " << score;
            scoreText.setString(ss.str());

            // update the branch sprites
            for (int i = 0; i < NUM_BRANCHES; i++)
            {
                float height = i * 150;

                if (branchPositions[i] == side::LEFT)
                {
                    // Move the sprite to the left side
                    branches[i].setPosition(220, height);
                    // Flip the sprite round the other way
                    branches[i].setOrigin(220,40);
                    branches[i].setRotation(180);
                }
                else if (branchPositions[i] == side::RIGHT)
                {
                    // Move the sprite to the right side
                    branches[i].setPosition(450, height);

                    // Set the sprite rotation to normal
                    branches[i].setOrigin(187.5, 200);
                    branches[i].setRotation(0);
                } else
                {
                    // Hide the branch
                    branches[i].setPosition(3000, height);
                }
            }

            // handle a flying log
            if (logActive)
            {
                spriteLog.setPosition(
                    spriteLog.getPosition().x + (logSpeedX * dt.asSeconds()),
                    spriteLog.getPosition().y + (logSpeedY * dt.asSeconds()));

                // Has the insect reached the right hand edge of the screen?
                if (spriteLog.getPosition().x < -100 ||
                    spriteLog.getPosition().x > 2000)
                {
                    // Set it up ready to be a whole new log next frame
                    logActive = false;
                    spriteLog.setPosition(200, 730);
                }
            }

            // has the player been squished by a branch?
            if (branchPositions[5] == playerSide)
            {
                // death
                paused = true;
                acceptInput = false;

                spritePlayButton.setPosition(600, 20);
                // Draw the gravestone
                spriteRIP.setPosition(525, 760);

                // hide the player
                spritePlayer.setPosition(2000, 660);

                // Change the text of the message
                messageText.setString("SQUISHED!!");

                // Center it on the screen
                FloatRect textRect = messageText.getLocalBounds();

                messageText.setOrigin(textRect.left +
                                      textRect.width / 2.0f,
                                      textRect.top + textRect.height / 2.0f);

                messageText.setPosition(400,
                                        400);

                spriteAxe.setPosition(2000, 0);


                spriteLog.setPosition(200, 800);

                // Play the death sound
                death.play();
            }

        }


        if (active)
        {

//            window.clear(background);
//            window.draw(image);
            // Draw game scene here
            window.draw(spriteBackground);
            // Draw the insect
            window.draw(spriteBee);

            // Draw the clouds
            window.draw(spriteCloud1);
            window.draw(spriteCloud2);
            window.draw(spriteCloud3);
            // Draw the player
            window.draw(spritePlayer);

            // Draw the branches
            for (int i = 0; i < NUM_BRANCHES; i++) {
                window.draw(branches[i]);
            }

            // Draw the tree
            window.draw(spriteTree);

            // Draw the axe

            // Draw the timebar
            window.draw(timeBar);

            // Draw the left Arrow
            window.draw(spriteLeftArrow);

            // Draw the right Arrow
            window.draw(spriteRightArrow);

            // Draw the play Button
            window.draw(spritePlayButton);

            // Draw the pause Button
            window.draw(spritePauseButton);

            // Draw score text
            window.draw(scoreText);

            // Draw the flyinglog
            window.draw(spriteLog);

            // Draw the RIP

            if (paused) {
                // Draw the message text
                window.draw(messageText);
            }
            window.draw(spriteAxe);
            window.draw(spriteRIP);
//            window.draw(text);
            window.display();
        }


        else {
            sf::sleep(sf::milliseconds(100));
        }
    }
}
