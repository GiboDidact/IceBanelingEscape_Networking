#include "pchserver.h"
#include "ServerApp.h"
#include "ServerGameApp.h"

int main()
{
	config::LoadConfigValues();
	if(config::STANDALONE_GAME)
	{
		ServerGameApp app(config::STANDALONE_PLAYER_COUNT);
		app.Run();
	}
	else
	{
		ServerApp app;
		app.Run();
	}
	
	return 0;
}