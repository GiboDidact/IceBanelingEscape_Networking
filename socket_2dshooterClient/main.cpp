#include "pchclient.h"
#include "ClientLobbyApp.h"
#include "ClientGameAppold.h"
#include "ClientGameApp.h"
#include "SharedSrc/Util/Camera.h"

void runLobby();
void runGame();

int port_id = -1;
int main()
{
	config::LoadConfigValues();
	if (config::STANDALONE_GAME)
	{
		ClientGameApp* app = new ClientGameApp(config::ServerPort);
		app->Run();
		delete app;
	}
	else
	{
		runLobby();
	}

	return 0;
}

void runLobby()
{
	int result;
	{
		ClientLobbyApp app;
		result = app.Run(port_id);
	}
	if (result)
		runGame();
}

void runGame()
{
	int result;
	{
		ClientGameApp* app = new ClientGameApp(port_id);
		result = app->Run();
		delete app;
	}
	if (result)
		runLobby();
}