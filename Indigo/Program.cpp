#include "stdafx.h"

#include "SystemManager.h"
#include "WindowManager.h"
#include "AssetManager.h"

void init();
void run();

/* Managers */
SystemManager* g_system;
WindowManager* g_window;
AssetManager*  g_assetManager;
//GameManager*   g_gameManager;

DX::StepTimer g_timer;

int main()
{
}

void init()
{
    g_timer = DX::StepTimer();
    g_timer.SetFixedTimeStep(true);
    g_timer.SetTargetElapsedSeconds(1.0f / 60.0f);

    g_system = new SystemManager();
    g_system->init();

    g_window = new WindowManager();
    g_window->init();

    g_assetManager = new AssetManager();
    g_assetManager->init();

    /*g_gameManager = new GameManager();
    g_gameManager->init();*/
}

void run()
{
    while (WND->isRunning())
    {
        g_timer.Tick([&]()
            {
                WND->update();
                //GAME->update();
            });

        WND->clear();

        //GAME->draw();

        WND->swap();
    }
}