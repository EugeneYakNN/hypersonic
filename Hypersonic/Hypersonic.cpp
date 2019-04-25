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

template<typename T>
auto operator>>(std::istream & in, T& t) -> decltype(t.Read(in), in)
{
    t.Read(in);
    return in;
}

template<typename T>
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

enum EntityType
{
    Player = 0,
    Bomb = 1,
    Item = 2
};

struct Entity
{
    union
    {
        int entityType;
        EntityType type;
    };
    int owner; //id of the player | bomb's owner
    int x;
    int y;
    union
    {
        int param1;                 
        int bombsLeft;              //For players: number of bombs the player can still place.
        int roundsToExplode;   //For bombs: number of rounds left until the bomb explodes.
        int itemType;               //For items: the integer representing the item.
    };
    union
    {
        int param2;                 //For players : current explosion range of the player's bombs.
        int explosionRange;         //For bombs: current explosion range of the bomb.
    };

    void Print(std::ostream& os) const
    {
        os << "entityType=" << entityType
           << " owner=" << owner
           << " x=" << x
           << " y=" << y
           << " param1=" << param1
           << " param2=" << param2;
    }
    int entityOrder;
};

class Entities
{
public:
    Entities(int myId)
        : m_myId(myId)
        , m_myEntity(-1)
        , m_bombsByRoundsToExplode(8 + 1)
    {}
    void AddEntity(Entity &e)
    {
        e.entityOrder = m_entitiesList.size();
        if (Player == e.type && m_myId == e.owner)
        {
            m_myEntity = e.entityOrder;
        }
        m_entitiesList.push_back(e);

        if (Bomb == e.type)
        {
            //bool myBomb = (m_myId == e.owner);
            try
            {
                //sorry, cannot guarantee order of the bombs in the list (we move bombs in the middle of checking them).=
                //if (myBomb)
                //{
                //    m_bombsByRoundsToExplode[e.roundsToExplode].push_front(e.entityOrder);
                //}
                //else
                //{
                    m_bombsByRoundsToExplode[e.roundsToExplode].push_back(e.entityOrder);
                //}
            }
            catch (...)
            {
                cerr << "e.roundsToExplode = " << e.roundsToExplode << endl;
                cerr << "m_bombsByRoundsToExplode[e.roundsToExplode].size() = " << m_bombsByRoundsToExplode[e.roundsToExplode].size() << endl;
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

    void Reset()
    {
        m_myEntity = -1;
        for (size_t i = 1; i < 8 + 1; i++)
        {
            m_bombsByRoundsToExplode[i].clear();
        }
        m_entitiesList.clear();
    }

    //protected:
    int m_myId;
    int m_myEntity;
    std::vector<Entity> m_entitiesList;

    typedef std::vector<size_t> EntitiesPositions;
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
    GridCostEstimator(const Grid &grid, Entities &entitiesList)
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
        size_t bombsLeft = entitiesList.Me().bombsLeft;
        size_t range = entitiesList.Me().explosionRange;

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
    
    void Reset()
    {
        CellSituation defaultSituation;
        for (GridSituation::iterator row = m_gridSituation.begin(); row != m_gridSituation.end(); row++)

        {
            //cerr << "clear row: ";
            for (RowSituation::iterator cell = row->begin(); cell != row->end(); cell++)
                //for (auto cell : row)
            {
                *cell = defaultSituation;
                //cerr << (cell.m_roundsToExplode > 9 ) ? '.' : cell.m_roundsToExplode;
            }
            //cerr << endl;
        }
    }

    void Analyze()
    {
        CalcExplosions();
    }

    //void AddEntity(Entity &e)
    //{
    //    if (Bomb == e.type)
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
    Entities &m_entitiesList; //will modify (less rounds to explode bombs)
    //std::vector<Entity> m_entitiesList;


    int max;

    template <typename T>
    void EraseFromVector(std::vector<T> &vec, const T &value)
    {
        vec.erase(std::remove(vec.begin(), vec.end(), value), vec.end());
    }

    void BombExplodesBomb(const Entity &bomb1, Entity &bomb2)
    {
        cerr << "BombExplodesBomb, rounds: " << bomb2.roundsToExplode << "->" << bomb1.roundsToExplode << endl;
        if (bomb1.roundsToExplode >= bomb2.roundsToExplode)
            throw std::exception();
        EraseFromVector<size_t>(m_entitiesList.m_bombsByRoundsToExplode[bomb2.roundsToExplode], bomb2.entityOrder);
        //m_entitiesList.m_bombsByRoundsToExplode[bomb2.roundsToExplode].remove(bomb2.entityOrder);
        bomb2.roundsToExplode = bomb1.roundsToExplode;
        m_entitiesList.m_bombsByRoundsToExplode[bomb2.roundsToExplode].push_back(bomb2.entityOrder);
    }

    void CalcExplosionDirection(int dx, int dy, const Entity &bombEntity)
    {
        bool myBomb = (m_entitiesList.m_myId == bombEntity.owner);

        //DEBUG cerr << "bomb @ " << bombEntity.x << " " << bombEntity.y << " dx,dy=" << dx << "," << dy;
        for (int r = 1; r < bombEntity.explosionRange; r++)
        {
            //DEBUG cerr << " r";
            int explodedX = bombEntity.x + dx * r;
            int explodedY = bombEntity.y + dy * r;
            if (explodedX < 0 || explodedX >= m_width || explodedY < 0 || explodedY >= m_height)
            {
                break;
            }
            CellSituation &situation = m_gridSituation[explodedY][explodedX];

            //DEBUG cerr << "(" << m_grid.m_grid[explodedY][explodedX] << ")";

            if (CELL_WALL == m_grid.m_grid[explodedY][explodedX])
            {
                break;
            }
            else //box or floor
            {
                if (bombEntity.roundsToExplode <= situation.m_roundsToExplode)
                {
                    situation.m_roundsToExplode = bombEntity.roundsToExplode;
                    if (myBomb)
                    {
                        situation.m_selfBomb = true;
                    }
                }
                if (CELL_FLOOR != m_grid.m_grid[explodedY][explodedX]) //box
                {
                    break;
                }
                else
                {
                    for (auto entity : m_entitiesList.m_entitiesList)
                    {
                        if (entity.x == explodedX && entity.y == explodedY) //bomb explodes otther entity
                        {
                            if (Item == entity.type)
                            {
                                break;
                            }
                            else if (Bomb == entity.type
                                && bombEntity.roundsToExplode < entity.roundsToExplode)
                            {
                                BombExplodesBomb(bombEntity, entity); // will move it to the end of list of bombs exploding earlier
                            }
                        }
                    }
                    //TODO: check for other items and bombs
                }
            }
            if (0 == dx && 0 == dy)
            {
                break; //check bomb position only once
            }
        }
        //DEBUG cerr << endl;
    }
    
    void CalcExplosions()
    {
        const std::vector<std::vector<int>> directions
            = { {0, 0},  // bomb position
                {-1, 0}, // dy=-1 dx= 0 up
                {0, -1}, // dy= 0 dx=-1 left
                {0, 1},  // dy= 0 dx=+1 right
                {1, 0} };// dy=+1 dx= 0 down

        for (size_t rounds = 1; rounds < 8 + 1; rounds++)
        {
            //bombs to explode in N rounds (we don't count 0 - they already exloded)
            for (auto b = m_entitiesList.m_bombsByRoundsToExplode[rounds].begin();
                b != m_entitiesList.m_bombsByRoundsToExplode[rounds].end(); b++) //it is expected that the list may be extended in the middle
            {
                const Entity &bombEntity = m_entitiesList.m_entitiesList[*b];

                for (auto direction = directions.begin(); direction != directions.end(); direction++)
                {
                    CalcExplosionDirection((*direction)[0], (*direction)[1], bombEntity);
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

        static Entities entitiesList(myId);
        entitiesList.Reset();

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

        //cerr << entitiesList;


        high_resolution_clock::time_point timeHaveInput = high_resolution_clock::now();
        static GridCostEstimator gridCost(grid, entitiesList);
        gridCost.Reset();
        gridCost.Analyze();
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