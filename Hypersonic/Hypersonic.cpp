#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

template<class T>
auto operator>>(std::istream & in, T& t) -> decltype(t.Read(in), in)
{
    t.Read(in);
    return in;
}

template<class T>
auto operator<<(std::ostream& os, const T& t) -> decltype(t.Print(os), os)
{
    t.Print(os);
    return os;
}

struct Rules
{
    int width;
    int height;
    int myId;

    void Read(std::istream & in)
    {
        in >> width >> height >> myId; in.ignore();
    }
};

template <typename T>
void clip(T &value, T min, T max)
{
    if (value < min)
    {
        value = min;
    }
    if (value > max)
    {
        value = max;
    }
}

struct Cell
{
    int x;
    int y;
};

const char CELL_FLOOR = '.';
const char CELL_WALL = 'X';

class Grid
{
public:
    Grid(const int width, const int height)
        : m_width(width)
        , m_height(height)
    {
    }
    void ReadRow(const string &row)
    {
        m_grid.push_back(row);
    }
    bool IsBox(int x, int y) const
    {
        return CELL_FLOOR != m_grid[y][x] && CELL_WALL != m_grid[y][x];
    }
    bool IsBoxSafeCheck(int x, int y) const
    {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height)
            return false;
        else
            return IsBox(x, y);
    }

    bool IsBoxAround(int x, int y) const
    {
        return IsBoxSafeCheck(x - 1, y) || IsBoxSafeCheck(x + 1, y)
            || IsBoxSafeCheck(x, y - 1) || IsBoxSafeCheck(x, y + 1);
    }

    std::vector<string> m_grid;
    const size_t m_width;
    const size_t m_height;
protected:
};



class Map
{
public:
    Map(const Rules &rules)
        : m_rules(rules)
    {}
protected:
    const Rules &m_rules;

};

class GameState
{

};

const int ENTITY_TYPE_PLAYER = 0;
const int ENTITY_TYPE_BOMB = 1;
const int ENTITY_TYPE_ITEM = 2;

struct Entity
{
    int entityType;
    int owner;
    int x;
    int y;
    int param1;
    int param2;
    void Print(std::ostream& os) const
    {
        os << "entityType=" << entityType
           << " owner=" << owner
           << " x=" << x
           << " y=" << y
           << " param1=" << param1
           << " param2=" << param2;
    }
};

class Entities
{
public:
    Entities(int myId)
        : m_myId(myId)
        , m_myEntity(-1)
        , m_bombsByRoundsToExplode(8 + 1)
    {}
    void AddEntity(const Entity &e)
    {
        size_t entityOrder = m_entitiesList.size();
        if (ENTITY_TYPE_PLAYER == e.entityType && m_myId == e.owner)
        {
            m_myEntity = entityOrder;
        }
        m_entitiesList.push_back(e);

        if (ENTITY_TYPE_BOMB == e.entityType)
        {
            bool myBomb = (m_myId == e.owner);
            try
            {
                if (myBomb)
                {
                    m_bombsByRoundsToExplode[e.param1].push_front(entityOrder);
                }
                else
                {
                    m_bombsByRoundsToExplode[e.param1].push_back(entityOrder);
                }
            }
            catch (...)
            {
                cerr << "e.param1 = " << e.param1 << endl;
                cerr << "m_bombsByRoundsToExplode[e.param1].size() = " << m_bombsByRoundsToExplode[e.param1].size() << endl;
                throw;
            }

        }
    }

    const Entity & Me() const
    {
        return m_entitiesList[m_myEntity];
    }
    void Print(std::ostream& os) const
    {
        os << "----Entities (" << m_entitiesList.size() << ") :-------------------" << endl;
        os << "Bombs to explode in rounds:" << endl;
        for (size_t i = 1; i < 8 + 1; i++)
        {
            os << i << " (" << m_bombsByRoundsToExplode[i].size() << ") :";
            for (auto b = m_bombsByRoundsToExplode[i].begin(); b != m_bombsByRoundsToExplode[i].end(); b++)
            {
                const Entity &bombEntity = m_entitiesList[*b];
                if (bombEntity.x > 12 || bombEntity.y > 10)
                {
                    cerr << "----ERROR: *b=" << *b << " " << bombEntity << endl;
                }
                os << " @ " << bombEntity.x << " " << bombEntity.y;
            }
            os << endl;
        }
    }


    //protected:
    int m_myId;
    int m_myEntity;
    std::vector<Entity> m_entitiesList;

    typedef std::list<size_t> EntitiesPositions;
    std::vector<EntitiesPositions> m_bombsByRoundsToExplode;
};

class CellSituation
{
public:
    CellSituation()
        : m_roundsToExplode(0xFFFF)
        , m_waysToExit(0)
        , m_selfBomb(false)
        , m_boxCost(0)
        , m_bombPlacementValue(0)
    {
    }

    //threats / explosion:
    size_t m_roundsToExplode;       //when this cell be exploded by any bomb (param1)
    size_t m_waysToExit;            //free cells to move

    //explosion:
    bool   m_selfBomb;              //player's bomb is one of the earliest, so the player will score
    size_t m_boxCost;               //does this cell contain a box? With any item?

    //bomb placement
    size_t m_bombPlacementValue;    //aggregated score, including explosion of unique boxes, another bombs
};

typedef std::vector<CellSituation> RowSituation;
typedef std::vector<RowSituation> GridSituation;


class GridCostEstimator
{
public:
    GridCostEstimator(const Grid &grid, const Entities &entitiesList)
        : m_grid(grid)
        , m_width(m_grid.m_width)
        , m_height(m_grid.m_height)
        , m_zeroRow(m_width, 0)
        , m_gridCost(m_height, m_zeroRow)
        , m_entitiesList(entitiesList)
        , max(0)
        , zeroRowSituation(m_width)
        , m_gridSituation(m_height, zeroRowSituation)
    {
        size_t bombsLeft = entitiesList.Me().param1;
        size_t range = entitiesList.Me().param2;

        CalcExplosions();

        //for (int y = 0; y < m_height; y++) {
        //    for (int x = 0; x < m_width; x++) {
        //        if (m_grid.IsBox(x,y))
        //        {
        //            AddBox(x,y,3);
        //            
        //        }
        //    }
        //}
    }

    //void AddEntity(Entity &e)
    //{
    //    if (ENTITY_TYPE_BOMB == e.entityType)
    //}

    void AddBox(size_t boxX, size_t boxY, size_t bombRange)
    {
        int rangeX[2] = { boxX - bombRange + 1, boxX + bombRange - 1 };
        clip<int>(rangeX[0], 0, m_width);
        clip<int>(rangeX[1], 0, m_width);

        int rangeY[2] = { boxY - bombRange + 1, boxY + bombRange - 1 };
        clip<int>(rangeY[0], 0, m_height);
        clip<int>(rangeY[1], 0, m_height);

        for (int x = rangeX[0]; x < rangeX[1]; x++) {
            int y = boxY;
            int &cost = m_gridCost[y][x];
            if (cost >= 0 && boxX != x && !m_grid.IsBox(x, y))
            {
                cost += 2;
                if (cost > max)
                {
                    max = cost;
                    maxCell.x = x;
                    maxCell.y = y;
                }
            }
        }

        for (int y = rangeY[0]; y < rangeY[1]; y++) {
            int x = boxX;
            int &cost = m_gridCost[y][x];
            if (cost >= 0 && boxY != y && !m_grid.IsBox(x, y))
            {
                cost += 2;
                if (cost > max)
                {
                    max = cost;
                    maxCell.x = x;
                    maxCell.y = y;
                }
            }
        }
    }

    void Print(std::ostream& os) const
    {
        for (int y = 0; y < m_height; y++) {
            for (int x = 0; x < m_width; x++) {
                os << m_gridCost[y][x] << " ";
            }
            os << endl;
        }
    }
    Cell maxCell;

protected:
    const Grid &m_grid;
    const size_t m_width;
    const size_t m_height;

    typedef std::vector<int> CellRowCost;
    typedef std::vector<CellRowCost> GridCost;

    RowSituation zeroRowSituation;
    GridSituation m_gridSituation;

    const CellRowCost m_zeroRow;
    GridCost m_gridCost;
    const Entities &m_entitiesList;
    //std::vector<Entity> m_entitiesList;


    int max;

    void CalcExplosions()
    {
        for (size_t rounds = 1; rounds < 8 + 1; rounds++)
        {
            //bombs to explode in N rounds (we don't count 0 - they already exloded)
            for (auto b = m_entitiesList.m_bombsByRoundsToExplode[rounds].begin();
                b != m_entitiesList.m_bombsByRoundsToExplode[rounds].end(); b++)
            {
                const Entity &bombEntity = m_entitiesList.m_entitiesList[*b];
                bool myBomb = (m_entitiesList.m_myId == bombEntity.owner);

                int explosionRange = bombEntity.param2;

                int dx = -1, dy = 0;
                const std::vector<std::vector<int>> directions
                    = { {0, 0},  // bomb position
                        {-1, 0}, // dy=-1 dx= 0
                        {0, -1}, // dy= 0 dx=-1
                        {0, 1},  // dy= 0 dx=+1
                        {1, 0} };// dy=+1 dx= 0

                for (auto direction = directions.begin(); direction != directions.end(); direction++)
                {
                    dy = (*direction)[0];
                    dx = (*direction)[1];
                    //DEBUG cerr << "bomb @ " << bombEntity.x << " " << bombEntity.y << " dx,dy=" << dx << "," << dy;
                    for (int r = 1; r < explosionRange; r++)
                    {
                        //DEBUG cerr << " r";
                        int explodedX = bombEntity.x + dx*r;
                        if (explodedX < 0 || explodedX >= m_width) break;
                        int explodedY = bombEntity.y + dy*r;
                        if (explodedY < 0 || explodedY >= m_height) break;
                        CellSituation &situation = m_gridSituation[explodedY][explodedX];

                        //DEBUG cerr << "(" << m_grid.m_grid[explodedY][explodedX] << ")";

                        if (CELL_FLOOR == m_grid.m_grid[explodedY][explodedX])
                        {
                            if (rounds <= situation.m_roundsToExplode)
                            {
                                situation.m_roundsToExplode = rounds;
                                if (myBomb)
                                {
                                    situation.m_selfBomb = true;
                                }
                            }
                        }
                        else if (CELL_WALL == m_grid.m_grid[explodedY][explodedX])
                        {
                            break;
                        }
                        else //box?
                        {
                            //avoid copy-past from CELL_FLOOR
                            if (rounds <= situation.m_roundsToExplode)
                            {
                                situation.m_roundsToExplode = rounds;
                                if (myBomb)
                                {
                                    situation.m_selfBomb = true;
                                }
                            }
                            break;
                        }
                        if (direction == directions.begin())
                        {
                            break; //check bomb position only once
                        }
                    }
                    //DEBUG cerr << endl;
                }
            }
        }

        for (int y = 0; y < m_height; y++) {
            for (int x = 0; x < m_width; x++) {
                if (m_gridSituation[y][x].m_roundsToExplode > 9)
                {
                    cerr << ". ";
                }
                else
                {
                    cerr << m_gridSituation[y][x].m_roundsToExplode << " ";
                }
            }
            cerr << endl;
        }
    }

};

int main()
{
    int width;
    int height;
    int myId;
    cin >> width >> height >> myId; cin.ignore();


    // game loop
    while (1) {
        high_resolution_clock::time_point timeRoundBegin = high_resolution_clock::now();

        Grid grid(width, height);
        for (int i = 0; i < height; i++) {
            string row;
            cin >> row; cin.ignore();
            //cerr << row << endl;
            grid.ReadRow(row);
        }

        for (int x = 0; x < width; x++)
        {
            cerr << "--";
        };
        cerr << endl;


        int entities;
        cin >> entities; cin.ignore();

        Entities entitiesList(myId);

        for (int i = 0; i < entities; i++) {
            Entity e;
            cin >> e.entityType
                >> e.owner
                >> e.x
                >> e.y
                >> e.param1
                >> e.param2;
            entitiesList.AddEntity(e);
            //cerr << "" << i << ": " << e << " (entities: " << entitiesList.m_entitiesList.size() << ")" << endl;
            cin.ignore();
        }

        cerr << entitiesList;


        high_resolution_clock::time_point timeHaveInput = high_resolution_clock::now();
        GridCostEstimator gridCost(grid, entitiesList);
        cerr << gridCost;

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
        //vec.back().c

        //if (grid.IsBoxAround(entitiesList.Me().x, entitiesList.Me().y))
        //{
        //    cout << "BOMB " << gridCost.maxCell.x << " " <<  gridCost.maxCell.y << endl;
        //}
        //else
        {
            cout << "MOVE " << gridCost.maxCell.x << " " << gridCost.maxCell.y << endl;
        }
        high_resolution_clock::time_point timeHaveOutput = high_resolution_clock::now();
        long long readDuration = duration_cast<microseconds>(timeHaveInput - timeRoundBegin).count();
        long long thinkDuration = duration_cast<microseconds>(timeHaveOutput - timeHaveInput).count();
        cerr << "read: " << readDuration / 1000 << "." << readDuration % 1000
            << " ms; think: " << thinkDuration / 1000 << "." << thinkDuration % 1000 << " ms" << endl;
    }
}