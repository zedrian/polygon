#pragma once


#include <map>
#include <string>

#include "map.h"


using std::map;
using std::string;


class City
{
public:
    City();
    City(string name,
         int tile_size,
         const map<string, Tile>& tile_atlas);

    void load(const string& name,
              const map<string, Tile>& tile_atlas);
    void save(const string& name);

    void update(float dt);
    void bulldoze(const Tile& tile);
    void shuffleTiles();
    void tileChanged();

    double getHomeless() const;
    double getUnemployed() const;


public:
    Map map;

    double population;
    double employable;

    double residential_tax;
    double commercial_tax;
    double industrial_tax;

    double earnings;
    double funds;

    int day;


private:
    double distributePool(double& pool,
                          Tile& tile,
                          double rate = 0.0);


private:
    float _current_time;
    float _time_per_day;

    vector<int> _shuffled_tiles;

    double _population_pool;
    double _employment_pool;

    float _citizens_can_work_proportion;

    double _birth_rate;
    double _death_rate;
};