#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <chrono>
#include <limits>

#define VALIDATE_INPUT
#define EXIT_ON_ERRORS

#ifdef _WIN32
#include <windows.h>
#undef max

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

const Position unknown = { 0xFFFF,  0xFFFF }; //not safe to use
const Position zero  = {  0,  0 }; // same position or left top corner
const Position up    = {  0, -1 }; // dx= 0 dy=-1
const Position left  = { -1,  0 }; // dx=-1 dy= 0
const Position right = { +1,  0 }; // dx=+1 dy= 0
const Position down  = {  0, +1 }; // dx= 0 dy=+1
const Position rightBottomCell = { 12,  10 }; // TODO: get rid of this const

const std::vector<Position> directions = { zero, up, left, right, down };

struct Rules
{
    Position size;
    int myId;

    auto Read(std::istream & in)
    {
        in >> size.x >> size.y >> myId; in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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

    Position size() const { return m_size; }
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

    bool ValidPos(Position pos) const { return (pos >= zero && pos < m_size); }

    bool Equals(const Grid &rhs) const
    {
        return m_grid == rhs.m_grid;
    }
protected:
    void ReadRow(const std::string &row)
    {
        m_grid.push_back(row);
#ifdef VALIDATE_INPUT
        CHECK_RANGE_THROW(row.length(), m_size.x, m_size.x);
#endif //#ifdef VALIDATE_INPUT
    }

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

    const Entity & Me() const
    {
        return m_list[m_myEntity];
    }
    std::ostream& Print(std::ostream& os) const
    {
        os << "----Entities (" << m_list.size() << ") :-------------------" << std::endl;
        os << "Bombs to explode in rounds:" << std::endl;
        for (size_t rounds = 1; rounds < 8 + 1; rounds++)
        {
            const EntitiesPositions &bombsToExplode = m_bombsByRoundsToExplode[rounds];
            os << rounds << " (" << bombsToExplode.size() << ") :";
            for (auto b : bombsToExplode)
            {
                const Entity &bombEntity = m_list[bombsToExplode[b]];
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
        m_list.clear();
    }

    bool CoincidesPrediction(const Entities &prediction)
    {
        std::vector<Entity>::const_iterator entityActual = m_list.begin();
        std::vector<Entity>::const_iterator entityPredicted = prediction.m_list.begin();
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
                    if (++entityActual == m_list.end())
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
        } while (entityActual != m_list.end() && entityPredicted != prediction.m_list.end());
        return true;
    }

    typedef std::vector<size_t> EntitiesPositions;
    std::vector<Entity> m_list;
    int m_myId;
    std::vector<EntitiesPositions> m_bombsByRoundsToExplode;
protected:
    int m_myEntity;

    void AddEntity(Entity &e)
    {

#ifdef VALIDATE_INPUT
        try
        {
            CHECK_RANGE_THROW(e.type, Player, Item);
            CHECK_RANGE_THROW(e.owner, 0, 3);
            CHECK_RANGE_THROW(e.pos, zero, rightBottomCell);
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
        e.entityOrder = m_list.size();
        if (Player == e.type && m_myId == e.owner)
        {
            m_myEntity = e.entityOrder;
        }
        m_list.push_back(e);

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
#ifndef _WIN32
#ifdef EXIT_ON_ERRORS
            throw std::exception();
#endif //#ifdef EXIT_ON_ERRORS
#endif //#ifndef _WIN32
            return false;
        }
        
        return m_entitiesList.CoincidesPrediction(prediction.m_entitiesList);
    }

    Grid m_grid;
    Entities m_entitiesList; //will modify (less rounds to explode bombs)
};

const size_t INF = 0xFFFF;

class CellSituation
{
public:
    CellSituation()
        : analyzed(false)
        , m_prevCell(unknown)
        , m_distanceFromMe(INF)
        , m_distanceToSave(INF)
        , m_safeTimeToStay(-1)  // -1: don't know the path to save;  0: we have to move this round; INF: no bomb 
        , m_itemsValue(0)
        , m_hasBomb(false)
        , m_roundsToExplode(INF)
        , m_waysToExit(0)
        , m_selfBomb(false)
        //, m_boxCost(0)
        , m_bombPlacementValue(0)
    {
    }

    bool analyzed;

    Entities::EntitiesPositions m_entities;

    size_t m_distanceFromMe;
    Position m_prevCell;            //how to get here?
    size_t m_itemsValue;
    bool   m_hasBomb;               //there is bomb in the cell - cannot move there
    
    //threats / explosion:
    size_t m_roundsToExplode;       //when this cell be exploded by any bomb (param1)
    size_t m_waysToExit;            //free cells to move (even to be exploded)
    size_t m_safeWaysToExit;        //free cells to move are safe from explosions
    size_t m_distanceToSave;        //rounds to walk to cell which will not explode (hopefully)
    int    m_safeTimeToStay;        //how many rounds the safe path will still be opened (ending on a safe cell)?

    //explosion:
    bool   m_selfBomb;              //player's bomb is one of the earliest, so the player will score
    //NO NEED - CAN CALC size_t m_boxCost;               //does this cell contain a box? With any item?

    //bomb placement
    size_t m_bombPlacementValue;    //aggregated score, including explosion of unique boxes, another bombs

    static std::ostream& PrintLegend(std::ostream& os)
    {
        os << "" << std::endl;
        os << "px: prevCell.x     st: safeTimeToStay" << std::endl;
        os << "py: prevCell.y    re: roundsToExplode" << std::endl;
        os << "dm: distanceFromMe ds: distanceToSave" << std::endl;
        return os;
    }

    static std::ostream& PrintHeader(std::ostream& os)
    {
        os << "px|py|dm|st|re|ds" << std::endl;
        return os;
    }

    std::ostream& Print(std::ostream& os) const
    {
        os << std::setw(2) << m_prevCell.x << "|" << std::setw(2) << m_prevCell.y << "|";
        if (INF == m_distanceFromMe)  os << " .|"; else os << std::setw(2) << m_distanceFromMe  << "|";
        if (INF == m_safeTimeToStay)  os << " .|"; else os << std::setw(2) << m_safeTimeToStay  << "|";
        if (INF == m_roundsToExplode) os << " .|"; else os << std::setw(2) << m_roundsToExplode << "|";
        if (INF == m_distanceToSave)  os << " .|"; else os << std::setw(2) << m_distanceToSave  << std::endl;
        return os;
    }
};

class GridSituationPos
{
public:
    inline CellSituation operator[](Position pos) const { return Pos(pos); }
    inline CellSituation & operator[](Position pos) { return Pos(pos); }

    GridSituationPos(Position size)
        : m_situaton(size.y, RowSituation(size.x))
    {
    }

    void Reset()
    {
        CellSituation defaultSituation;
        for (auto &row : m_situaton)
        {
            std::fill(row.begin(), row.end(), CellSituation());
        }
    }
protected:
    const CellSituation &Pos(Position pos) const { return m_situaton[pos.y][pos.x]; }
    CellSituation & Pos(Position pos) { return m_situaton[pos.y][pos.x]; }

    typedef std::vector<CellSituation> RowSituation;
    typedef std::vector<RowSituation> GridSituation;
    GridSituation m_situaton;
};

class GridCostEstimator
{
public:
    GridCostEstimator(GameState &state)
        : m_state(state)
        , maxCell(zero)
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
    
    void NextRound()
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

    void EntitiesToGrid()
    {
        for (auto entity : m_state.m_entitiesList.m_list)
        {
            CellSituation &cellSituation = m_gridSituation[entity.pos];
            cellSituation.m_entities.push_back(entity.entityOrder);
            if (Item == entity.entityType)
            {
                cellSituation.m_itemsValue = entity.itemType;
            }
            if (Bomb == entity.entityType)
            {
                cellSituation.m_hasBomb = true;
            }
        }
    }

    void AnalyzeCell(Position pos)
    {
        CellSituation &cellSituation = m_gridSituation[pos];
        char cell = m_state.m_grid.Pos(pos);
        if (Floor == cell)
        {
            size_t boxesWithItems = 0;
            for (size_t d = 1; d <= 4; d++) // check neighbors
            {
                Position check = pos + directions[d];
                if (m_state.m_grid.ValidPos(check))
                {
                    char checkCell = m_state.m_grid.Pos(check);
                    CellSituation checkSituation = m_gridSituation[check];
                    if (Floor == checkCell)
                    {
                        cellSituation.m_waysToExit++;
                        if (INF == checkSituation.m_roundsToExplode)
                        {
                            cellSituation.m_safeWaysToExit++;
                        }
                    }
                    if ('0' <= checkCell && checkCell <= '2')
                    {
                        //TODO: check if I can trigger chain reaction to explode the box earlier / same time as opponents 
                        if (0xFFFF == checkSituation.m_roundsToExplode) //for now only count boxes not in range of bombs
                        {
                            if (checkCell > '0')
                            {
                                boxesWithItems++;
                            }
                            cellSituation.m_bombPlacementValue++;
                        }
                    }
                    //TODO: check entities
                }

            }
            cellSituation.m_bombPlacementValue *= 2;
            if (boxesWithItems > 0)
            {
                cellSituation.m_bombPlacementValue++;
            }
        }
        cellSituation.analyzed = true;
    }

    void Analyze()
    {
        EntitiesToGrid();
        CalcExplosions();
        Position pos = { 0, 0 };
        //TODO: lazy analysis - start with cells surrounding player
        for (pos.y = 0; pos.y < m_state.m_grid.size().y; ++pos.y)
        {
            for (pos.x = 0; pos.x < m_state.m_grid.size().x; ++pos.x)
            {
                AnalyzeCell(pos);
            }
        }
    }

    void CheckAgainstMax(Position player)
    {
        if (player == maxCell)
        {
            return;
        }

        const CellSituation &playerSituation = m_gridSituation[player];
        const CellSituation &maxSituation = m_gridSituation[maxCell];

        if (playerSituation.m_safeTimeToStay <= 2)
        {
            return;
        }

        if (maxSituation.m_roundsToExplode < INF && maxSituation.m_safeTimeToStay <= 0 && playerSituation.m_distanceToSave < maxSituation.m_distanceToSave)
        {
            maxCell = player;
            return;
        }
        
        if (playerSituation.m_itemsValue != maxSituation.m_itemsValue)
        {
            if (playerSituation.m_itemsValue > maxSituation.m_itemsValue && playerSituation.m_distanceFromMe <= maxSituation.m_distanceFromMe )
            {
                maxCell = player;
            }
            return;
        }

        if (playerSituation.m_bombPlacementValue > maxSituation.m_bombPlacementValue)
        {
            maxCell = player;
            return;
        }
    }
    
    void CalcDistance(size_t lookAhead, Position player, Position prev)
    {
        CellSituation &playerSituation = m_gridSituation[player];
        if (playerSituation.m_distanceFromMe > lookAhead)
        {
            playerSituation.m_distanceFromMe = lookAhead;
            playerSituation.m_prevCell = prev;
            if (INF == playerSituation.m_roundsToExplode)
            {
                playerSituation.m_distanceToSave = 0;
                playerSituation.m_safeTimeToStay = INF;
            }

            if (lookAhead < 9) //don't look further than certain depth
            {
                if (playerSituation.m_roundsToExplode != lookAhead + 1) //not explodes when step into
                {
                    for (size_t d = 1; d <= 4; d++) // check neighbors
                    {
                        Position check = player + directions[d];
                        if (m_state.m_grid.ValidPos(check)
                            && Floor == m_state.m_grid.Pos(check)
                            && false == m_gridSituation[check].m_hasBomb) //Players can occupy the same cell as a bomb only when the bomb appears on the same turn as when the player enters the cell.
                        {
                            CalcDistance(lookAhead + 1, check, player); //recursively check every possible direction
                                                        
                            CellSituation checkSituation = m_gridSituation[check];
                            if (playerSituation.m_distanceToSave > 0
                                && checkSituation.m_distanceToSave + 1 < playerSituation.m_distanceToSave)
                            {
                                playerSituation.m_distanceToSave = checkSituation.m_distanceToSave + 1;
                            }

                            if (playerSituation.m_safeTimeToStay < INF) //it is not safe to stay here
                            {
                                if (playerSituation.m_distanceToSave < INF) //there is a way to save
                                {
                                    if (checkSituation.m_safeTimeToStay > 0)
                                    {
                                        size_t safeTimeToStay = std::min<int>((int)playerSituation.m_roundsToExplode, checkSituation.m_safeTimeToStay) - 2;
                                        if (safeTimeToStay > checkSituation.m_safeTimeToStay)
                                        {
                                            checkSituation.m_safeTimeToStay = safeTimeToStay;
                                        }
                                    }
                                }
                            }
                            if (playerSituation.m_safeTimeToStay < INF
                                && checkSituation.m_safeTimeToStay + 1 < playerSituation.m_distanceToSave)
                            {
                                playerSituation.m_distanceToSave = checkSituation.m_distanceToSave + 1;
                            }


                        }
                    }
                }
            }
            CheckAgainstMax(player);
        }
    }

    
    std::ostream& PlayerCommand(std::ostream& cmd)
    {
        const Entity me = m_state.m_entitiesList.Me();
        const CellSituation &mySituation = m_gridSituation[me.pos];
        std::string message;
        //maxCell = me.pos;
        //CellSituation &cellSituation = m_gridSituation.Pos(me.pos);
        CalcDistance(0, me.pos, unknown);

        const CellSituation &maxCellSituation = m_gridSituation[maxCell];

        //should I place a bomb?
        bool placeBomb = false;
        if (me.bombsLeft > 0 && mySituation.m_bombPlacementValue >= 0)
        {
            if (INF == mySituation.m_roundsToExplode) //TODO: not ready to predict chain reaction
            {
                if (mySituation.m_bombPlacementValue >= maxCellSituation.m_bombPlacementValue)
                {
                    placeBomb = true;
                }
            }
        }

        CellSituation::PrintLegend(debug());
        debug() << "x |y |";
        CellSituation::PrintHeader(debug());

        if (maxCell == me.pos)
        {
            debug() << mySituation;
            cmd << (placeBomb ? "BOMB " : "MOVE ") << me.pos.x << " " << me.pos.y << " Where to go?"<< std::endl;
        }
        else
        {
            Position cellToMove = maxCell;
            debug() << std::setw(2) << cellToMove.x << "|" << std::setw(2) << cellToMove.y << "|";
            debug() << m_gridSituation[cellToMove];
            while (!(m_gridSituation[cellToMove].m_prevCell == me.pos))
            {
                cellToMove = m_gridSituation[cellToMove].m_prevCell;
                if (unknown == cellToMove)
                {
                    debug() << "unknown cell, stay still" << std::endl;
                    cellToMove = me.pos;
                    message = "OOPS!";
                    break;
                }
                debug() << std::setw(2) << cellToMove.x << "|" << std::setw(2) << cellToMove.y << "|";
                debug() << m_gridSituation[cellToMove];
            };


            cmd << (placeBomb ? "BOMB " : "MOVE ") << cellToMove.x << " " << cellToMove.y << " " << message << std::endl;
            //if ()
            //{
            //     << cellToMove.x << " " << cellToMove.y << std::endl;
            //}
            //else
        };

        //if (cellSituation.m_roundsToExplode < INF)
        {
            //Staying is not an option
        }
        //if (me.bombsLeft > 0) //TODO: even if player has no bombs they can start moving to new target before than bomb explodes if it is safe
        {
            //char cell = m_state.m_grid.Pos(pos);
            //if (Floor == cell)
            //{
            //    for (size_t d = 1; d <= 4; d++) // check neighbors
            //    {
            //        Position check = pos + directions[d];
            //        if (m_state.m_grid.ValidPos(check))
            //        {
            //            char checkCell = m_state.m_grid.Pos(check);
            //            CellSituation checkSituation = m_gridSituation.Pos(check);
            //            if (Floor == checkCell)
            //            {
            //                cellSituation.m_waysToExit++;
            //            }
            //            if ('0' <= checkCell && checkCell <= '2')
            //            {
            //                //TODO: check if I can trigger chain reaction to explode the box earlier / same time as opponents 
            //                if (0xFFFF == checkSituation.m_roundsToExplode) //for now only count boxes not in range of bombs
            //                {
            //                    size_t bombPlacementValue = checkCell - '0';
            //                    cellSituation.m_bombPlacementValue += (bombPlacementValue);
            //                }
            //            }
            //            //TODO: check entities
            //        }
            //    }
            //}
        }
        
        return cmd;
    }

    std::ostream& Print(std::ostream& os) const
    {
        os <<  "explodes in  ";
        os << "|place bomb ? ";
        //os << "|ways to exit ";
        os << "|distance     ";
        os << std::endl;
        Position pos = { 0,0 };
        for (pos.y = 0; pos.y < m_state.m_grid.size().y; pos.y++)
        {
            for (pos.x = 0; pos.x < m_state.m_grid.size().x; pos.x++)
            {
                const CellSituation &situation = m_gridSituation[pos];
                if (situation.m_roundsToExplode > 9)
                {
                    os << ".";
                }
                else
                {
                    os << situation.m_roundsToExplode;
                }
            }

            os << "|";

            for (pos.x = 0; pos.x < m_state.m_grid.size().x; pos.x++)
            {
                const CellSituation &situation = m_gridSituation[pos];
                switch (m_state.m_grid.Pos(pos))
                {
                case Wall:  os << "X"; break;
                case '0':
                case '1':
                case '2':   os << "#"; break;
                default: os << situation.m_bombPlacementValue;
                }
            }

            os << "|";

            //for (pos.x = 0; pos.x < m_state.m_grid.size().x; pos.x++)
            //{
            //    switch (m_state.m_grid.Pos(pos))
            //    {
            //    case Wall:  os << "X"; break;
            //    case '0':
            //    case '1':
            //    case '2':   os << "#"; break;
            //    default: os << m_gridSituation.Pos(pos).m_waysToExit;
            //    }
            //}
            for (pos.x = 0; pos.x < m_state.m_grid.size().x; pos.x++)
            {
                const CellSituation &situation = m_gridSituation[pos];
                switch (m_state.m_grid.Pos(pos))
                {
                case Wall:  os << "X"; break;
                case '0':
                case '1':
                case '2':   os << "#"; break;
                default:
                    if (situation.m_distanceFromMe > 9)
                    {
                        os << ".";
                    }
                    else
                    {
                        os << situation.m_distanceFromMe;
                    }
                }
            }

            //for (int x = 0; x < m_state.m_grid.size().x; x++) {
            //    os << m_gridCost[y][x] << " ";
            //}
            os << std::endl;
        }
        return os;
    }

protected:
    GameState &m_state;
    GridSituationPos m_gridSituation;
    GameState *m_prediction;
    Position maxCell;

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
        CellSituation &situation = m_gridSituation[exploded];

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
                //TODO: use precalculated CellSituation::m_entities
                for (auto &entity : m_state.m_entitiesList.m_list)
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
                //CellSituation bombSituation = m_gridSituation.Pos(bombEntity.pos);
                //if (bombEntity.roundsToExplode <= bombSituation.m_roundsToExplode)
                //{
                //    bombSituation.m_roundsToExplode = bombEntity.roundsToExplode;
                //    if (m_state.m_entitiesList.m_myId == bombEntity.owner) //my bomb
                //    {
                //        bombSituation.m_selfBomb = true;
                //    }
                //}
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
                const Entity &bombEntity = m_state.m_entitiesList.m_list[bombsToExplode[b]];

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

        gridCost.NextRound();
        gridCost.Analyze();
        // Write an action using cout. DON'T FORGET THE "<< std::endl"
        // To debug: debug() << "Debug messages..." << std::endl;

        gridCost.PlayerCommand(command());
        debug() << gridCost;

        std::chrono::high_resolution_clock::time_point timeHaveOutput = std::chrono::high_resolution_clock::now();
        long long readDuration = std::chrono::duration_cast<std::chrono::microseconds>(timeHaveInput - timeRoundBegin).count();
        long long thinkDuration = std::chrono::duration_cast<std::chrono::microseconds>(timeHaveOutput - timeHaveInput).count();
        debug() << "read: " << readDuration / 1000 << "." << readDuration % 1000
            << " ms; think: " << thinkDuration / 1000 << "." << thinkDuration % 1000 << " ms" << std::endl;
    }
}