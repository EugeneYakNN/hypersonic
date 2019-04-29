#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <chrono>

#define VALIDATE_INPUT
#define EXIT_ON_ERRORS

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

bool operator<=(const Position& lhs, const Position& rhs)
{
    return lhs.x <= rhs.x && lhs.y <= rhs.y;
}

bool operator<(const Position& lhs, const Position& rhs)
{
    return lhs.x < rhs.x && lhs.y < rhs.y;
}

bool operator>=(const Position& lhs, const Position& rhs)
{
    return lhs.x >= rhs.x && lhs.y >= rhs.y;
}

bool operator>(const Position& lhs, const Position& rhs)
{
    return lhs.x > rhs.x && lhs.y > rhs.y;
}

Position operator+(const Position& lhs, const Position& rhs)
{
    Position result = { lhs.x + rhs.x, lhs.y + rhs.y };
    return result;
}

Position operator*(const Position& lhs, const int scalar)
{
    Position result = { lhs.x * scalar, lhs.y * scalar };
    return result;
}

inline Position operator*(const int scalar, const Position& rhs) { return rhs * scalar; }

const Position zero  = {  0,  0 }; // same position or left top corner
const Position up    = {  0, -1 }; // dx= 0 dy=-1
const Position left  = { -1,  0 }; // dx=-1 dy= 0
const Position right = { +1,  0 }; // dx=+1 dy= 0
const Position down  = {  0, +1 }; // dx= 0 dy=+1

const std::vector<Position> directions = { zero, up, left, right, down };

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
bool checkRange(T &value, T min, T max)
{
    return (value >= min && value <= max);
}

#define CHECK_RANGE_THROW(VAL, MIN, MAX) if ((VAL) < (MIN) || (VAL) > (MAX)) { debug() << #VAL << " = " << (VAL) <<" is out of range: (" << (MIN) << "," << (MAX) << ")" << std::endl; throw std::exception(); }

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
    //Grid (const Grid &grid) - basically same as copy constructor
    //{
    //    m_size = grid.m_size;
    //    m_grid = grid.m_grid;
    //}
    Position size() const { return m_size; }
    void ReadRow(const std::string &row)
    {
        m_grid.push_back(row);
    }

    std::istream & Read(std::istream & in)
    {
        m_grid.clear();
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

    std::ostream& Print(std::ostream& os) const
    {
        for (auto row: m_grid) {
            os << row << std::endl;
        }
        return os;
    }

    char Pos(Position pos) const { return m_grid[pos.y][pos.x]; }
    char & Pos(Position pos) { return m_grid[pos.y][pos.x]; }

    bool IsBox(Position pos) const
    {
        return Floor != Pos(pos) && Wall!= Pos(pos);
    }
    bool IsBoxSafeCheck(Position pos) const
    {
        int x = pos.x;
        int y = pos.y;
        if (pos >= zero && pos < m_size )
            return IsBox(pos);
        else
            return false;
    }

    bool IsBoxAround(Position pos) const
    {
        return IsBoxSafeCheck(pos + directions[1]) || IsBoxSafeCheck(pos + directions[2])
            || IsBoxSafeCheck(pos + directions[3]) || IsBoxSafeCheck(pos + directions[4]);
    }

    bool Equals(const Grid &rhs) const
    {
        return m_grid == rhs.m_grid;
    }
protected:
    Position m_size;
    std::vector<std::string> m_grid;
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

    bool Equals(const Entity &rhs) const
    {
        return entityType == rhs.entityType
            && owner == rhs.owner
            && pos == rhs.pos
            && param1 == rhs.param1
            && param2 == rhs.param2;
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

#ifdef VALIDATE_INPUT
        try
        {
            CHECK_RANGE_THROW(e.type, Player, Item);
            CHECK_RANGE_THROW(e.owner, 0, 3);
            switch (e.type)
            {
            case Player:
                CHECK_RANGE_THROW(e.bombsLeft, 0, 99);
                CHECK_RANGE_THROW(e.explosionRange, 3, 99);
                break;
            case Bomb:
                CHECK_RANGE_THROW(e.roundsToExplode, 1, 8);
                CHECK_RANGE_THROW(e.explosionRange, 3, 99);
                break;
            case Item:
                CHECK_RANGE_THROW(e.itemType, 1, 2);
                //CHECK_RANGE_THROW(e.param2, 0, 0); bug in documentation: The param2 will be: For items : ignored number(= 0). In practice game provides param2 = 2: e.g. 2 0 12 8 2 2
                break;
            };
            CHECK_RANGE_THROW(e.owner, 0, 9);
        }
        catch (...)
        {
            debug() << "Invalid entity read" << std::endl;
            throw;
        }
#endif //#ifdef VALIDATE_INPUT
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
        Reset();
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

    bool CannotPredictEntity(const Entity &entityActual) //entity is not in prediction, is it ok?
    {
        if (Bomb == entityActual.entityType)
        {
            if (m_myId != entityActual.owner) //we can't predict when other players place bombs - ignore
            {
                return true;
            }
            else
            {
                debug() << "***ERROR*** No prediction for own bomb:" << std::endl;
                debug() << entityActual << std::endl;
#ifdef EXIT_ON_ERRORS
                throw std::exception();
#endif //#ifdef EXIT_ON_ERRORS
                return false;
            }
        }
        else if (Item == entityActual.entityType)
        {
            //TODO: predict Items
            return true; //temporal limitation
        }
        else
        {
            debug() << "***ERROR*** No prediction for entity:" << std::endl;
            debug() << entityActual << std::endl;
#ifdef EXIT_ON_ERRORS
            throw std::exception();
#endif //#ifdef EXIT_ON_ERRORS
            return false;
        }
    }
    
    bool CoincidesPrediction(const Entities &prediction)
    {
        std::vector<Entity>::const_iterator entityActual = m_entitiesList.begin();
        std::vector<Entity>::const_iterator entityPredicted = prediction.m_entitiesList.begin();
        do
        {
            while (entityPredicted->entityType > entityActual->entityType) //we predicted less items
            {
                if (CannotPredictEntity(*entityActual))
                {
                    return false;
                }
                else
                {
                    if (++entityActual == m_entitiesList.end())
                    {
                        break;
                    }
                }
            }
//            if (Player == entityActual->entityType)
//            {
//                if (m_myId == entityActual->owner)
//                {
//                    if (!entityActual->Equals(*entityPredicted))
//                    {
//                        debug() << "***ERROR*** Incorrect prediction own player:" << *entityPredicted << std::endl;
//                        debug() << "Actual (game input):" << *entityActual << std::endl;
//#ifdef EXIT_ON_ERRORS
//                        throw std::exception();
//#endif //#ifdef EXIT_ON_ERRORS
//                        return false;
//                    }
//                }
//            }
//            else if (Bomb == entityActual->entityType)
//            {
//            }
            entityActual++;
            entityPredicted++;
        } while (entityActual != m_entitiesList.end() && entityPredicted != prediction.m_entitiesList.end());
        return true;
    }

    //protected:
    int m_myId;
    int m_myEntity;
    std::vector<Entity> m_entitiesList;

    typedef std::vector<size_t> EntitiesPositions;
    std::vector<EntitiesPositions> m_bombsByRoundsToExplode;
};

class GameState
{
public:
    GameState(const Rules &rules)
        : m_grid(rules.size)
        , m_entitiesList(rules.myId)
    {}
    std::istream & Read(std::istream & in)
    {
        in >> m_grid;
        in >> m_entitiesList;
        return in;
    }

    bool CoincidesPrediction(const GameState &prediction)
    {
        if ( ! m_grid.Equals(prediction.m_grid) )
        {
            debug() << "***ERROR*** Incorrect prediction of grid:" << std::endl;
            debug() << prediction.m_grid << std::endl << "-------------" << std::endl;
            debug() << "Actual (game input):" << std::endl;
            debug() << m_grid << std::endl;
#ifdef EXIT_ON_ERRORS
            throw std::exception();
#endif //#ifdef EXIT_ON_ERRORS
            return false;
        }
        
        return m_entitiesList.CoincidesPrediction(prediction.m_entitiesList);
    }

    Grid m_grid;
    Entities m_entitiesList; //will modify (less rounds to explode bombs)
};

class CellSituation
{
public:
    CellSituation()
        : analyzed(false)
        , m_roundsToExplode(0xFFFF)
        , m_waysToExit(0)
        , m_selfBomb(false)
        , m_boxCost(0)
        , m_bombPlacementValue(0)
    {
    }

    bool analyzed;

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

    //CellSituation operator[](Position pos) const { return Pos(pos); }
    //CellSituation & operator[](Position pos) { return Pos(pos); }

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
    GridCostEstimator(GameState &state)
        : m_state(state)
        , m_gridCost(m_state.m_grid.size().y, CellRowCost(m_state.m_grid.size().x, 0))

        , maxScore(0)
        , m_gridSituation(m_state.m_grid.size())
        , m_prediction(nullptr)
    {
        //size_t bombsLeft = entitiesList.Me().bombsLeft;
        //size_t range = entitiesList.Me().explosionRange;

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
    virtual ~GridCostEstimator()
    {
        delete m_prediction;
    }
    
    void NextTurn()
    {
        m_gridSituation.Reset();
        if (nullptr == m_prediction)
        {
            m_prediction = new GameState(m_state);
        };

    }

    bool CheckPrediction()
    {
        if (nullptr == m_prediction)
            return true;
        return m_state.CoincidesPrediction(*m_prediction);
    }

    void Analyze()
    {
        CalcExplosions();
    }

    //void AddEntity(Entity &e)
    //{
    //    if (Bomb == e.type)
    //}

    void AddBox(Position box, size_t bombRange)
    {
        int rangeX[2] = { box.x - bombRange + 1, box.x + bombRange - 1 };
        clip<int>(rangeX[0], 0, m_state.m_grid.size().x);
        clip<int>(rangeX[1], 0, m_state.m_grid.size().x);

        int rangeY[2] = { box.y - bombRange + 1, box.y + bombRange - 1 };
        clip<int>(rangeY[0], 0, m_state.m_grid.size().y);
        clip<int>(rangeY[1], 0, m_state.m_grid.size().y);

        Position pos;
        for (pos.x = rangeX[0]; pos.x < rangeX[1]; pos.x++) {
            int &cost = m_gridCost[pos.y][pos.x];
            if (cost >= 0 && box.x != pos.x && !m_state.m_grid.IsBox(pos))
            {
                cost += 2;
                if (cost > maxScore)
                {
                    maxScore = cost;
                    maxCell = pos;
                }
            }
        }

        pos.x = box.x;
        for (pos.y = rangeY[0]; pos.y < rangeY[1]; pos.y++) {
            int &cost = m_gridCost[pos.y][pos.x];
            if (cost >= 0 && box.y != pos.y && !m_state.m_grid.IsBox(pos))
            {
                cost += 2;
                if (cost > maxScore)
                {
                    maxScore = cost;
                    maxCell = pos;
                }
            }
        }
    }

    std::ostream& Print(std::ostream& os) const
    {
        for (int y = 0; y < m_state.m_grid.size().y; y++)
        {
            for (int x = 0; x < m_state.m_grid.size().x; x++)
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

            for (int x = 0; x < m_state.m_grid.size().x; x++) {
                os << m_gridCost[y][x] << " ";
            }
            os << std::endl;
        }
        return os;
    }

    Position maxCell;

protected:
    GameState &m_state;
    typedef std::vector<int> CellRowCost;
    typedef std::vector<CellRowCost> GridCost;
    GridSituationPos m_gridSituation;
    const CellRowCost m_zeroRow;
    GridCost m_gridCost;
    int maxScore;
    GameState *m_prediction;

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
        EraseFromVector<size_t>(m_state.m_entitiesList.m_bombsByRoundsToExplode[bomb2.roundsToExplode], bomb2.entityOrder);
        //m_entitiesList.m_bombsByRoundsToExplode[bomb2.roundsToExplode].remove(bomb2.entityOrder);
        bomb2.roundsToExplode = bomb1.roundsToExplode;
        m_state.m_entitiesList.m_bombsByRoundsToExplode[bomb2.roundsToExplode].push_back(bomb2.entityOrder);
    }

    bool ExplosionMeetsObstacle(Position exploded, const Entity &bombEntity) //true means we need to break
    {
        CellSituation &situation = m_gridSituation.Pos(exploded);

        //DEBUG debug() << "(" << m_grid.m_grid[explodedY][explodedX] << ")";

        const char cell = m_state.m_grid.Pos(exploded);
        if (Wall == cell)
        {
            return true;
        }
        else //box or floor
        {
            if (bombEntity.roundsToExplode <= situation.m_roundsToExplode)
            {
                situation.m_roundsToExplode = bombEntity.roundsToExplode;
                if (m_state.m_entitiesList.m_myId == bombEntity.owner) //my bomb
                {
                    situation.m_selfBomb = true;
                }
            }
            if (Floor != cell) //box
            {
                if (1 == situation.m_roundsToExplode && nullptr != m_prediction)
                {
                    m_prediction->m_grid.Pos(exploded) = Floor; //next turn prediction: box explodes
                    //TODO: predict items
                }
                return true;
            }
            else
            {
                for (auto &entity : m_state.m_entitiesList.m_entitiesList)
                {
                    if (entity.pos == exploded) //bomb explodes other entity
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
            const Position exploded = bombEntity.pos + direction * r; //{ bombEntity.pos.x + direction.x * r, bombEntity.pos.y + direction.y * r };
            if (!(exploded >= zero && exploded < m_state.m_grid.size())) //if (exploded.x < 0 || exploded.x >= m_grid.size().x || exploded.y < 0 || exploded.y >= m_grid.size().y)
            {
                break;
            }
            if (ExplosionMeetsObstacle(exploded, bombEntity))
            {
                break;
            }
            if (zero == direction)
            {
                break; //check bomb position only once
            }
        }
        //DEBUG debug() << std::endl;
    }
    
    void CalcExplosions()
    {
        for (size_t rounds = 1; rounds < 8 + 1; rounds++)
        {
            Entities::EntitiesPositions &bombsToExplode = m_state.m_entitiesList.m_bombsByRoundsToExplode[rounds];
            //bombs to explode in N rounds (we don't count 0 - they already exloded)
            for (size_t b = 0; b < bombsToExplode.size(); b++) //not using iterators here as want to add elements to the end of vector in the middle
            {
                const Entity &bombEntity = m_state.m_entitiesList.m_entitiesList[bombsToExplode[b]];

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
    GameState state(rules);
    GridCostEstimator gridCost(state);
    GameState prediction(rules);

    // game loop
    while (1) {
#ifdef _WIN32
        if (inputFile.eof())
        {
            break;
        }
#endif //#ifdef _WIN32

        std::chrono::high_resolution_clock::time_point timeRoundBegin = std::chrono::high_resolution_clock::now();
        input() >> state;

        std::chrono::high_resolution_clock::time_point timeHaveInput = std::chrono::high_resolution_clock::now();
        gridCost.CheckPrediction();

        gridCost.NextTurn();
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