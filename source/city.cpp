#include <fstream>
#include <map>
#include <sstream>

#include "city.h"


using std::endl;
using std::ifstream;
using std::ios;
using std::istringstream;
using std::map;
using std::ofstream;
using std::runtime_error;


City::City() :
        _birth_rate(0.00055),
        _death_rate(0.00023),
        _citizens_can_work_proportion(0.5),
        _population_pool(0),
        population(_population_pool),
        _employment_pool(0),
        employable(_employment_pool),
        residential_tax(0.05),
        commercial_tax(0.05),
        industrial_tax(0.05),
        earnings(0),
        funds(0),
        _current_time(0.0),
        _time_per_day(1.0),
        day(0)
{ }

City::City(string name,
           int tile_size,
           const std::map<string, Tile>& tile_atlas) :
        _birth_rate(0.00055),
        _death_rate(0.00023),
        _citizens_can_work_proportion(0.5),
        _population_pool(0),
        population(_population_pool),
        _employment_pool(0),
        employable(_employment_pool),
        residential_tax(0.05),
        commercial_tax(0.05),
        industrial_tax(0.05),
        earnings(0),
        funds(0),
        _current_time(0.0),
        _time_per_day(1.0),
        day(0)
{
    map.tile_size = tile_size;
    load(name, tile_atlas);
}


void City::load(const string& name,
                const std::map<string, Tile>& tile_atlas)
{
    int width = 0;
    int height = 0;

    ifstream file(name + "-configuration.dat", ios::in);
    string line;

    while (getline(file, line))
    {
        istringstream stream(line);
        string key;
        if (getline(stream, key, '='))
        {
            string value;
            if (getline(stream, value))
            {
                if (key == "width") width = stoi(value);
                else if (key == "height") height = stoi(value);
                else if (key == "day") day = stoi(value);
                else if (key == "population pool") _population_pool = stod(value);
                else if (key == "employment pool") _employment_pool = stod(value);
                else if (key == "population") population = stod(value);
                else if (key == "employable") employable = stod(value);
                else if (key == "birth rate") _birth_rate = stod(value);
                else if (key == "death rate") _death_rate = stod(value);
                else if (key == "residential tax") residential_tax = stod(value);
                else if (key == "commercial tax") commercial_tax = stod(value);
                else if (key == "industrial tax") industrial_tax = stod(value);
                else if (key == "funds") funds = stod(value);
                else if (key == "earnings") earnings = stod(value);
            }
            else
                throw runtime_error("No value for key '" + key + "'.");
        }
    }

    file.close();

    map.load(name + "-map.dat", width, height, tile_atlas);
    tileChanged();
}


void City::save(const string& name)
{
    ofstream file(name + "-configuration.dat", ios::out);

    file << "width=" << map.width << endl;
    file << "height=" << map.height << endl;
    file << "day=" << day << endl;
    file << "population pool=" << _population_pool << endl;
    file << "employment pool=" << _employment_pool << endl;
    file << "population=" << population << endl;
    file << "employable=" << employable << endl;
    file << "birth rate=" << _birth_rate << endl;
    file << "death rate=" << _death_rate << endl;
    file << "residential tax=" << residential_tax << endl;
    file << "commercial tax=" << commercial_tax << endl;
    file << "industrial tax=" << industrial_tax << endl;
    file << "funds=" << funds << endl;
    file << "earnings=" << earnings << endl;

    file.close();

    map.save(name + "-map.dat");
}

void City::update(float dt)
{
    double total_population = 0.0;
    double commercial_revenue = 0.0;
    double industrial_revenue = 0.0;

    _current_time += dt;
    if (_current_time < _time_per_day)
        return;

    ++day;
    _current_time = 0.0;

    if (day % 30 == 0)
    {
        funds += earnings;
        earnings = 0.0;
    }

    for (int i = 0; i < map.tiles.size(); ++i)
    {
        auto& tile = map.tiles[_shuffled_tiles[i]];

        switch (tile.type)
        {
            case TileType::Residential:
                distributePool(_population_pool, tile, _birth_rate - _death_rate);
                total_population += tile.population;
                break;

            case TileType::Commercial:
                if (rand() % 100 < 15 * (1.0 - commercial_tax))
                    distributePool(_employment_pool, tile, 0.0);
                break;

            case TileType::Industrial:
                if (map.resources[i] > 0 && rand() % 100 < population)
                {
                    ++tile.production;
                    --map.resources[i];
                }
                if (rand() % 100 < 15 * (1.0 - industrial_tax))
                    distributePool(_employment_pool, tile, 0.0);
                break;

            default:
                break;
        }

        tile.update();
    }

    for (int i = 0; i < map.tiles.size(); ++i)
    {
        auto& tile = map.tiles[_shuffled_tiles[i]];

        if (tile.type == TileType::Industrial)
        {
            int received_resources = 0;
            for (auto& tile2 : map.tiles)
                if (tile2.regions[0] == tile.regions[0] && tile2.type == TileType::Industrial)
                {
                    if (tile2.production > 0)
                    {
                        ++received_resources;
                        --tile2.production;
                    }
                    if (received_resources >= tile.tile_variant + 1)
                        break;
                }
            tile.stored_goods += (received_resources + tile.production) * (tile.tile_variant + 1);
        }
    }

    for (int i = 0; i < map.tiles.size(); ++i)
    {
        auto& tile = map.tiles[_shuffled_tiles[i]];

        if (tile.type == TileType::Commercial)
        {
            int received_goods = 0;
            double max_customers = 0.0;

            for (auto& tile2 : map.tiles)
            {
                if (tile2.regions[0] == tile.regions[0] && tile2.type == TileType::Industrial && tile2.stored_goods > 0)
                    while (tile2.stored_goods > 0 && received_goods != tile.tile_variant + 1)
                    {
                        --tile2.stored_goods;
                        ++received_goods;
                        industrial_revenue += 100 * (1.0 - industrial_tax);
                    }
                else if (tile2.regions[0] == tile.regions[0] && tile2.type == TileType::Residential)
                    max_customers += tile2.population;

                if (received_goods == tile.tile_variant + 1)
                    break;
            }

            tile.production = (received_goods * 100.0 + rand() % 20) * (1.0 - commercial_tax);

            auto revenue = tile.production * max_customers * tile.population / 100.0;
            commercial_revenue += revenue;
        }
    }

    _population_pool += _population_pool * (_birth_rate - _death_rate);
    total_population += _population_pool;

    auto new_workers = (total_population - population) * _citizens_can_work_proportion;
    if (new_workers < 0.0f)
        new_workers = -new_workers;
    _employment_pool += new_workers;
    employable += new_workers;
    if (_employment_pool < 0)
        _employment_pool = 0;
    if (employable < 0)
        employable = 0;

    population = total_population;

    earnings = (population - _population_pool) * 15 * residential_tax;
    earnings += commercial_revenue * commercial_tax;
    earnings += industrial_revenue * industrial_tax;
}

void City::bulldoze(const Tile& tile)
{
    for (int position = 0; position < map.width * map.height; ++position)
        if (map.selected_tiles[position] == 1)
        {
            switch (map.tiles[position].type)
            {
                case TileType::Residential:
                    _population_pool += map.tiles[position].population;
                    break;

                case TileType::Commercial:
                case TileType::Industrial:
                    _employment_pool += map.tiles[position].population;
                    break;

                default:
                    break;
            }

            map.tiles[position] = tile;
        }
}

void City::shuffleTiles()
{
    while (_shuffled_tiles.size() < map.tiles.size())
        _shuffled_tiles.push_back(0);

    iota(_shuffled_tiles.begin(), _shuffled_tiles.end(), 1);
    random_shuffle(_shuffled_tiles.begin(), _shuffled_tiles.end());
}

void City::tileChanged()
{
    map.updateDirection(TileType::Road);
    map.findConnectedRegions(
            {
                    TileType::Road,
                    TileType::Residential,
                    TileType::Commercial,
                    TileType::Industrial
            },
            0);
}

double City::getHomeless() const
{
    return _population_pool;
}

double City::getUnemployed() const
{
    return _employment_pool;
}

double City::distributePool(double& pool,
                            Tile& tile,
                            double rate)
{
    const static int move_rate = 4;

    unsigned int max_population = tile.max_population_per_level * (tile.tile_variant + 1);

    if (pool > 0)
    {
        auto moving = max_population - tile.population;
        if (moving > move_rate)
            moving = move_rate;
        if (moving > pool)
            moving = pool;
        pool -= moving;
        tile.population += moving;
    }

    tile.population += tile.population * rate;

    if (tile.population > max_population)
    {
        pool += tile.population - max_population;
        tile.population = max_population;
    }

    return tile.population;
}