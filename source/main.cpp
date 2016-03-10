#include "game.h"
#include "game-state-start.h"


int main(int argc,
         char* argv[])
{
    Game game;

    game.pushState(new GameStateStart(&game));
    game.main(argc, argv);

    return 0;
}