#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <chrono>

#ifdef _WIN32
#include <windows.h>

std::ifstream inputFile;
#define VERBOSE_INPUT
inline std::istream &input()
{
    return inputFile;
}

inline std::ostream &debug()
{
    return std::cout;
}

inline std::ostream &command()
{
    return std::cout;
}
#else
#define VERBOSE_INPUT
inline std::istream &input()
{
    return std::cin;
}

inline std::ostream &debug()
{
    return std::cerr;
}

inline std::ostream &command()
{
    return std::cout;
}
#endif

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

struct Position
{
    int x;
    int y;
    std::ostream& Print(std::ostream& os) const
    {
        os << x << " " << y << std::endl;
        return os;
    }
};

bool operator==(const Position& lhs, const Position& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

struct Rules
{
    Position size;
    int myId;

    auto Read(std::istream & in)
    {
        in >> size.x >> size.y >> myId; in.ignore();
#ifdef VERBOSE_INPUT
        debug() << size.x << " " << size.y << " " << myId <<std::endl;
#endif //#ifdef VERBOSE_INPUT
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

enum CellType
{
    Floor = '.',
    Wall = 'X'
    //numbers are boxes
};

class Grid
{
public:
    Grid(Position size)
        : m_size(size)
    {
    }
    void ReadRow(const std::string &row)
    {
        m_grid.push_back(row);
    }

    std::istream & Read(std::istream & in)
    {
        for (int i = 0; i < m_size.y; i++) {
            std::string row;
            in >> row; in.ignore();
#ifdef VERBOSE_INPUT
            debug() << row << std::endl;
#endif //#ifdef VERBOSE_INPUT
            ReadRow(row);
        }
        for (int x = 0; x < m_size.x; x++)
        {
            debug() << "--";
        };
        debug() << std::endl;
        return in;
    }

    bool IsBox(int x, int y) const
    {
        return Floor != m_grid[y][x] && Wall!= m_grid[y][x];
    }
    bool IsBoxSafeCheck(int x, int y) const
    {
        if (x < 0 || x >= m_size.x || y < 0 || y >= m_size.y)
            return false;
        else
            return IsBox(x, y);
    }

    bool IsBoxAround(int x, int y) const
    {
        return IsBoxSafeCheck(x - 1, y) || IsBoxSafeCheck(x + 1, y)
            || IsBoxSafeCheck(x, y - 1) || IsBoxSafeCheck(x, y + 1);
    }

    std::vector<std::string> m_grid;
    Position m_size;
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
    Position pos;
    union
    {
        int param1;                 
        int bombsLeft;          //For players: number of bombs the player can still place.
        int roundsToExplode;    //For bombs: number of rounds left until the bomb explodes.
        int itemType;           //For items: the integer representing the item.
    };
    union
    {
        int param2;             //For players : current explosion range of the player's bombs.
        int explosionRange;     //For bombs: current explosion range of the bomb.
    };

    std::ostream& Print(std::ostream& os) const
    {
        os << "entityType=" << entityType
           << " owner=" << owner
           << " x=" << pos.x
           << " y=" << pos.y
           << " param1=" << param1
           << " param2=" << param2;
        return os;
    }

    std::istream & Read(std::istream & in)
    {
        in >> entityType
           >> owner
           >> pos.x
           >> pos.y
           >> param1
           >> param2;
#ifdef VERBOSE_INPUT
        debug() << entityType
            << " " << owner
            << " " << pos.x
            << " " << pos.y
            << " " << param1
            << " " << param2
            << std::endl;
#endif //#ifdef VERBOSE_INPUT
        return in;
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
                debug() << "e.roundsToExplode = " << e.roundsToExplode << std::endl;
                debug() << "m_bombsByRoundsToExplode[e.roundsToExplode].size() = " << m_bombsByRoundsToExplode[e.roundsToExplode].size() << std::endl;
                throw;
            }

        }
    }

    const Entity & Me() const
    {
        return m_entitiesList[m_myEntity];
    }
    std::ostream& Print(std::ostream& os) const
    {
        os << "----Entities (" << m_entitiesList.size() << ") :-------------------" << std::endl;
        os << "Bombs to explode in rounds:" << std::endl;
        for (size_t rounds = 1; rounds < 8 + 1; rounds++)
        {
            const EntitiesPositions &bombsToExplode = m_bombsByRoundsToExplode[rounds];
            os << rounds << " (" << bombsToExplode.size() << ") :";
            for (auto b : bombsToExplode)
            {
                const Entity &bombEntity = m_entitiesList[bombsToExplode[b]];
                if (bombEntity.pos.x > 12 || bombEntity.pos.y > 10)
                {
                    debug() << "----ERROR: b=" << b << " " << bombEntity << std::endl;
                }
                os << " @ " << bombEntity.pos;
            }
            os << std::endl;
        }
        return os;
    }

    std::istream & Read(std::istream & in)
    {
        int entities;
        in >> entities; in.ignore();
#ifdef VERBOSE_INPUT
        debug() << entities << std::endl;
#endif //#ifdef VERBOSE_INPUT
        for (int i = 0; i < entities; i++) {
            Entity e;
            in >> e;
            AddEntity(e);
            //debug() << " " << i << ": " << e << " (entities: " << entitiesList.m_entitiesList.size() << ")" << std::endl;
            in.ignore();
        }
        return in;
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

class GridSituationPos : public GridSituation //TODO: replace inheritance with aggregation
{
public:
    CellSituation Pos(Position pos) const { return at(pos.y).at(pos.x); }
    CellSituation & Pos(Position pos) { return at(pos.y).at(pos.x); }

    //unsigned long operator[](Position pos) const { return Pos(pos); }
    //unsigned long & operator[](Position pos) { return Pos(pos); }

    GridSituationPos(Position size)
        : GridSituation(size.y, RowSituation(size.x))
    {
    }

    void Reset()
    {
        CellSituation defaultSituation;
        for (auto &row : *this)
        {
            std::fill(row.begin(), row.end(), CellSituation());
        }
    }
};

class GridCostEstimator
{
public:
    GridCostEstimator(const Grid &grid, Entities &entitiesList)
        : m_grid(grid)
        , m_zeroRow(m_grid.m_size.x, 0)
        , m_gridCost(m_grid.m_size.y, m_zeroRow)
        , m_entitiesList(entitiesList)
        , maxScore(0)
        , m_gridSituation(m_grid.m_size)
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
        m_gridSituation.Reset();
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
        clip<int>(rangeX[0], 0, m_grid.m_size.x);
        clip<int>(rangeX[1], 0, m_grid.m_size.x);

        int rangeY[2] = { boxY - bombRange + 1, boxY + bombRange - 1 };
        clip<int>(rangeY[0], 0, m_grid.m_size.y);
        clip<int>(rangeY[1], 0, m_grid.m_size.y);

        for (int x = rangeX[0]; x < rangeX[1]; x++) {
            int y = boxY;
            int &cost = m_gridCost[y][x];
            if (cost >= 0 && boxX != x && !m_grid.IsBox(x, y))
            {
                cost += 2;
                if (cost > maxScore)
                {
                    maxScore = cost;
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
                if (cost > maxScore)
                {
                    maxScore = cost;
                    maxCell.x = x;
                    maxCell.y = y;
                }
            }
        }
    }

    std::ostream& Print(std::ostream& os) const
    {
        for (int y = 0; y < m_grid.m_size.y; y++)
        {
            for (int x = 0; x < m_grid.m_size.x; x++)
            {
                if (m_gridSituation[y][x].m_roundsToExplode > 9)
                {
                    os << ". ";
                }
                else
                {
                    os << m_gridSituation[y][x].m_roundsToExplode << " ";
                }
            }

            os << "|";

            for (int x = 0; x < m_grid.m_size.x; x++) {
                os << m_gridCost[y][x] << " ";
            }
            os << std::endl;
        }
        return os;
    }

    Position maxCell;

protected:
    const Grid &m_grid;

    typedef std::vector<int> CellRowCost;
    typedef std::vector<CellRowCost> GridCost;

    GridSituationPos m_gridSituation;

    const CellRowCost m_zeroRow;
    GridCost m_gridCost;
    Entities &m_entitiesList; //will modify (less rounds to explode bombs)
    //std::vector<Entity> m_entitiesList;


    int maxScore;

    template <typename T>
    void EraseFromVector(std::vector<T> &vec, const T &value)
    {
        vec.erase(std::remove(vec.begin(), vec.end(), value), vec.end());
    }

    void ChainReaction(const Entity &bomb1, Entity &bomb2) // Bomb explodes another bomb
    {
        debug() << "ChainReaction, rounds: " << bomb2.roundsToExplode << "->" << bomb1.roundsToExplode << std::endl;
        if (bomb1.roundsToExplode >= bomb2.roundsToExplode)
            throw std::exception();
        EraseFromVector<size_t>(m_entitiesList.m_bombsByRoundsToExplode[bomb2.roundsToExplode], bomb2.entityOrder);
        //m_entitiesList.m_bombsByRoundsToExplode[bomb2.roundsToExplode].remove(bomb2.entityOrder);
        bomb2.roundsToExplode = bomb1.roundsToExplode;
        m_entitiesList.m_bombsByRoundsToExplode[bomb2.roundsToExplode].push_back(bomb2.entityOrder);
    }

    bool ExplosionMeetsObstacle(Position exploded, const Entity &bombEntity) //true means we need to break
    {
        CellSituation &situation = m_gridSituation.Pos(exploded);

        //DEBUG debug() << "(" << m_grid.m_grid[explodedY][explodedX] << ")";

        const char cell = m_grid.m_grid[exploded.y][exploded.x];
        if (Wall == cell)
        {
            return true;
        }
        else //box or floor
        {
            if (bombEntity.roundsToExplode <= situation.m_roundsToExplode)
            {
                situation.m_roundsToExplode = bombEntity.roundsToExplode;
                if (m_entitiesList.m_myId == bombEntity.owner) //my bomb
                {
                    situation.m_selfBomb = true;
                }
            }
            if (Floor != cell) //box
            {
                return true;
            }
            else
            {
                for (auto &entity : m_entitiesList.m_entitiesList)
                {
                    if (entity.pos == exploded) //bomb explodes otther entity
                    {
                        if (Item == entity.type)
                        {
                            return true;
                        }
                        else if (Bomb == entity.type
                            && bombEntity.roundsToExplode < entity.roundsToExplode)
                        {
                            ChainReaction(bombEntity, entity); // will move it to the end of list of bombs exploding earlier
                        }
                    }
                }
                //TODO: check for other items and bombs
            }
        }
        return false;
    }

    void CalcExplosionDirection(Position direction, const Entity &bombEntity)
    {
        //DEBUG debug() << "bomb @ " << bombEntity.x << " " << bombEntity.y << " dx,dy=" << dx << "," << dy;
        for (int r = 1; r < bombEntity.explosionRange; r++)
        {
            //DEBUG debug() << " r";
            Position exploded = { bombEntity.pos.x + direction.x * r, bombEntity.pos.y + direction.y * r };
            if (exploded.x < 0 || exploded.x >= m_grid.m_size.x || exploded.y < 0 || exploded.y >= m_grid.m_size.y)
            {
                break;
            }
            if (ExplosionMeetsObstacle(exploded, bombEntity))
            {
                break;
            }
            const Position Zero = { 0,0 };
            if (Zero == direction)
            {
                break; //check bomb position only once
            }
        }
        //DEBUG debug() << std::endl;
    }
    
    void CalcExplosions()
    {
        
        const std::vector<Position> directions
            = { {0, 0},  // bomb position
                {0, -1}, // dx= 0 dy=-1 up
                {-1, 0}, // dx=-1 dy= 0 left
                {1, 0},  // dx=+1 dy= 0 right
                {0, 1} };// dx= 0 dy=+1 down

        for (size_t rounds = 1; rounds < 8 + 1; rounds++)
        {
            Entities::EntitiesPositions &bombsToExplode = m_entitiesList.m_bombsByRoundsToExplode[rounds];
            //bombs to explode in N rounds (we don't count 0 - they already exloded)
            for (size_t b = 0; b < bombsToExplode.size(); b++) //not using iterators here as want to add elements to the end of vector in the middle
            {
                const Entity &bombEntity = m_entitiesList.m_entitiesList[bombsToExplode[b]];

                for (auto direction : directions)
                {
                    CalcExplosionDirection(direction, bombEntity);
                }
            }
        }
    }
};

int main()
{
#ifdef _WIN32
    char NPath[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, NPath);
    debug() << NPath << std::endl;
    inputFile.open("input.txt");
    if (!inputFile.is_open())
    {
        return -1;
    }
#endif //#ifdef _WIN32

    Rules rules;
    input() >> rules;


    // game loop
    while (1) {
#ifdef _WIN32
        if (inputFile.eof())
        {
            break;
        }
#endif //#ifdef _WIN32

        std::chrono::high_resolution_clock::time_point timeRoundBegin = std::chrono::high_resolution_clock::now();
        Grid grid(rules.size);
        input() >> grid;

        static Entities entitiesList(rules.myId);
        entitiesList.Reset();
        input() >> entitiesList;


        std::chrono::high_resolution_clock::time_point timeHaveInput = std::chrono::high_resolution_clock::now();
        static GridCostEstimator gridCost(grid, entitiesList);
        gridCost.Reset();
        gridCost.Analyze();
        debug() << gridCost;

        // Write an action using cout. DON'T FORGET THE "<< std::endl"
        // To debug: debug() << "Debug messages..." << std::endl;


        //if (grid.IsBoxAround(entitiesList.Me().x, entitiesList.Me().y))
        //{
        //    command() << "BOMB " << gridCost.maxCell.x << " " <<  gridCost.maxCell.y << std::endl;
        //}
        //else
        {
            command() << "MOVE " << gridCost.maxCell.x << " " << gridCost.maxCell.y << std::endl;
        }
        std::chrono::high_resolution_clock::time_point timeHaveOutput = std::chrono::high_resolution_clock::now();
        long long readDuration = std::chrono::duration_cast<std::chrono::microseconds>(timeHaveInput - timeRoundBegin).count();
        long long thinkDuration = std::chrono::duration_cast<std::chrono::microseconds>(timeHaveOutput - timeHaveInput).count();
        debug() << "read: " << readDuration / 1000 << "." << readDuration % 1000
            << " ms; think: " << thinkDuration / 1000 << "." << thinkDuration % 1000 << " ms" << std::endl;
    }
}