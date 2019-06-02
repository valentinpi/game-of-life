#include <ctime>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <SDL2/SDL.h>

int32_t window_width = 1280, 
    window_height = 720, 
    entity_size = 5;

struct Entity
{
    bool alive = std::rand() % 2;
    int32_t neighbors = 0;
};

std::vector<std::vector<Entity>> entities;

inline bool is_alive(int32_t x, int32_t y)
{
    if (x < 0 || y < 0 || 
        x > window_width / entity_size - 1 || 
        y > window_height / entity_size - 1)
        return false;

    return entities[y][x].alive;
}

int main(int argc, char *argv[])
{
    unsigned int fps_cap = 60;
    bool random_revive = false;

    if (argc <= 1)
    {
        std::cout 
            << "Usage: gol <entity-size> <width> <height> <fps-cap>\n"
            << "<entity-size>: Size of each entity (default: 5, 0 to disable)\n"
            << "<width>: Window width in pixels\n"
            << "<height>: Window height in pixels\n"
            << "(default: 1280x720)\n"
            << "<fps-cap>: Maximum FPS (default: 60)\n"
            << "<random-spawn>: Revive a random entity every frame "
            << "(default: false)" << std::endl;

        return EXIT_SUCCESS;
    }

    if (argc >= 2)
    {
        try
        {
            entity_size = std::stoi(argv[1]);
            
            if (entity_size <= 0)
            {
                std::cout 
                    << "Size of each entity must be larger than zero" 
                    << std::endl;

                return EXIT_SUCCESS;
            }
        }
        catch (...)
        {
            entity_size = 5;
        }
    }

    if (argc >= 3)
    {
        try
        {
            window_width = std::stoi(argv[2]);
            
            if (window_width <= 0)
            {
                std::cout 
                    << "Window width must be larger than zero" << std::endl;

                return EXIT_SUCCESS;
            }
        }
        catch (...)
        {
            window_width = 1280;
        }
    }

    if (argc >= 4)
    {
        try
        {
            window_height = std::stoi(argv[3]);
            
            if (window_height <= 0)
            {
                std::cout 
                    << "Window height must be larger than zero" << std::endl;

                return EXIT_SUCCESS;
            }
        }
        catch (...)
        {
            window_height = 720;
        }
    }

    if (argc >= 5)
    {
        try
        {
            fps_cap = std::stoi(argv[4]);

            if (fps_cap < 0)
            {
                std::cout 
                    << "FPS cap must be larger or equal zero" << std::endl;

                return EXIT_SUCCESS;
            }
        }
        catch (...)
        {
            fps_cap = 60;
        }
    }

    if (argc >= 6)
        if (std::string(argv[5]) == "true")
            random_revive = true;

    int32_t columns = window_width / entity_size,
        rows = window_height / entity_size;

    std::cout 
        << "Entity size: " << entity_size << "\n"
        << "Resolution: " << window_width << "x" << window_height << "\n";

    if (fps_cap == 0)
        std::cout << "FPS cap: Disabled\n";
    else
        std::cout << "FPS cap: " << fps_cap << "\n";

    std::cout 
        << "Random revive: " << std::boolalpha << random_revive << std::endl;

    std::cout
        << columns << " columns, "
        << rows << " rows" << "\n"
        << columns * rows * sizeof(Entity) 
        << " bytes to be allocated" << std::endl;

    std::srand((unsigned int) std::time(nullptr));

    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER | SDL_INIT_VIDEO) != 0)
    {
        std::cout 
            << "SDL: Initialization failed! SDL-Error: " << SDL_GetError() 
            << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Window *window = SDL_CreateWindow(
            "GOL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
            window_width, window_height, 0);

    if (window == nullptr)
    {
        std::cout 
            << "SDL_Window: Creation failed! SDL-Error: " << SDL_GetError() 
            << std::endl;

        SDL_Quit();

        return EXIT_FAILURE;
    }

    SDL_Renderer *renderer = 
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == nullptr)
    {
        std::cout 
            << "SDL_Renderer: Creation failed! SDL-Error: " << SDL_GetError() 
            << std::endl;
        
        SDL_DestroyWindow(window);
        SDL_Quit();

        return EXIT_FAILURE;
    }

    entities.reserve(rows);
    for (int32_t y = 0; y < rows; y++)
    {
        std::vector<Entity> row;
        row.reserve(columns);

        for (int32_t x = 0; x < columns; x++)
        {
            Entity entity;
            row.push_back(entity);
        }

        entities.push_back(row);
    }

    SDL_Event event;
    bool running = true;
    unsigned int default_delay = 0;

    if (fps_cap > 0)
         default_delay = 1000 / fps_cap;

    unsigned int fps = 0;
    unsigned int t1 = SDL_GetTicks();

    while(running)
    {
        unsigned int t2_1 = SDL_GetTicks();

        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    running = false;
                    break;
                default:
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        
        for (int32_t y = 0; y < rows; y++)
            for (int32_t x = 0; x < columns; x++)
            {
                int32_t neighbor_count = 0;

                neighbor_count = 
                    is_alive(x - 1, y - 1) + 
                    is_alive(x    , y - 1) + 
                    is_alive(x + 1, y - 1) + 
                    is_alive(x - 1, y    ) + 
                    is_alive(x + 1, y    ) + 
                    is_alive(x - 1, y + 1) + 
                    is_alive(x    , y + 1) + 
                    is_alive(x + 1, y + 1);

                entities[y][x].neighbors = neighbor_count;
            }

        // A real revive mechanic would need to check for dead entities first
        if (random_revive)
            entities[std::rand() % rows][std::rand() % columns].alive = true;

        SDL_Rect rect;
        rect.w = entity_size;
        rect.h = entity_size;
        
        for (int y = 0; y < rows; y++)
        {
            for (int x = 0; x < columns; x++)
            {
                int neighbor_count = entities[y][x].neighbors;

                if (neighbor_count == 3)
                    entities[y][x].alive = true;
                else if (neighbor_count < 2 || neighbor_count > 3)
                {
                    entities[y][x].alive = false;
                    continue;
                }

                rect.x = x * entity_size;
                rect.y = y * entity_size;

                SDL_RenderFillRect(renderer, &rect);
            }
        }

        SDL_RenderPresent(renderer);
        fps++;

        unsigned int t2_2 = SDL_GetTicks();

        if ((t2_2 - t1) > 1000)
        {
            std::stringstream title;
            title << "GOL - " << fps << " FPS";
            SDL_SetWindowTitle(window, title.str().c_str());
            fps = 0;
            t1 = SDL_GetTicks();
        }
        else if ((t2_2 - t2_1) < default_delay)
            SDL_Delay(default_delay - (t2_2 - t2_1));
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
