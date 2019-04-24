#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
 
const int ENTITY_TYPE_PLAYER = 0;
const int ENTITY_TYPE_BOMB = 1;

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

struct Entity
{
    int entityType;
    int owner;
    int x;
    int y;
    int param1;
    int param2;
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
        return '.' != m_grid[y][x] && 'X' != m_grid[y][x];
    }
    bool IsBoxSafeCheck(int x, int y) const
    { 
        if (x < 0 || x >= m_width || y < 0 || y >= m_height)
            return false;
        else
            return IsBox(x,y);
    }
    
    bool IsBoxAround(int x, int y) const
    {
        return IsBoxSafeCheck(x-1, y) || IsBoxSafeCheck(x+1, y)
        || IsBoxSafeCheck(x, y-1) || IsBoxSafeCheck(x, y+1);
    }
    
    std::vector<string> m_grid;
    const size_t m_width;
    const size_t m_height;
protected:
};

class Entities
{
public:
    Entities(int myId, size_t size=0)
        : m_myId(myId)
        , m_entitiesList(size)
        , m_myEntity(-1)
    {}
    void AddEntity(const Entity &e)
    {
        if (ENTITY_TYPE_PLAYER == e.entityType && m_myId == e.owner)
        {
            m_myEntity = m_entitiesList.size();
        }
        m_entitiesList.push_back(e);
    }
    const Entity & Me() const
    {
        return m_entitiesList[m_myEntity];
    }
//protected:
    int m_myId;
    int m_myEntity;
    std::vector<Entity> m_entitiesList;
};

class GridCostEstimator
{
public:
    GridCostEstimator(const Grid &grid, const Entities &entitiesList)
        : m_grid(grid)
        , m_width(m_grid.m_width)
        , m_height(m_grid.m_height)
        , m_zeroRow(m_width, 0)
        , m_gridCost(m_height, m_zeroRow)
        , max(0)
    {
        size_t bombsLeft = entitiesList.Me().param1;
        size_t range = entitiesList.Me().param2;
        for (int y = 0; y < m_height; y++) {
            for (int x = 0; x < m_width; x++) {
                if (m_grid.IsBox(x,y))
                {
                    AddBox(x,y,3);
                    
                }
            }
        }        
    }
    
    //void AddEntity(Entity &e)
    //{
    //    if (ENTITY_TYPE_BOMB == e.entityType)
    //}
    
    void AddBox(size_t boxX, size_t boxY, size_t bombRange)
    {
        int rangeX[2] = {boxX-bombRange+1, boxX+bombRange-1};
        clip<int>(rangeX[0], 0, m_width);
        clip<int>(rangeX[1], 0, m_width);
        
        int rangeY[2] = {boxY-bombRange+1, boxY+bombRange-1};
        clip<int>(rangeY[0], 0, m_height);
        clip<int>(rangeY[1], 0, m_height);

        for (int x = rangeX[0]; x < rangeX[1]; x++) {
            int y = boxY;
            int &cost = m_gridCost[y][x];
            if (cost >= 0 && boxX != x && !m_grid.IsBox(x,y))
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
            if (cost >=0 && boxY != y && !m_grid.IsBox(x,y))
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
    const CellRowCost m_zeroRow;
    GridCost m_gridCost;
    std::vector<Entity> m_entitiesList;
    
    int max;

};
 
int main()
{
    int width;
    int height;
    int myId;
    cin >> width >> height >> myId; cin.ignore();
    

    // game loop
    while (1) {
        
        Grid grid(width, height);
        for (int i = 0; i < height; i++) {
            string row;
            cin >> row; cin.ignore();
            cerr << row << endl;
            grid.ReadRow(row);
        }
        
        for (int x = 0; x < width; x++)
        {
            cerr << "--";
        };
        cerr << endl;
        
    
        int entities;
        cin >> entities; cin.ignore();
        
        Entities entitiesList(myId, entities);
        
        for (int i = 0; i < entities; i++) {
            Entity e;
            cin >> e.entityType
                >> e.owner
                >> e.x
                >> e.y
                >> e.param1
                >> e.param2;
            entitiesList.AddEntity(e);
            cin.ignore();



            //if (ENTITY_TYPE_BOMB == entityType && 0 == owner && )
            //{
            //    
            //}
        }
        
        GridCostEstimator gridCost(grid, entitiesList);
        cerr << gridCost;

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
        //vec.back().c
        
        if (grid.IsBoxAround(entitiesList.Me().x, entitiesList.Me().y))
        {
            cout << "BOMB " << gridCost.maxCell.x << " " <<  gridCost.maxCell.y << endl;
        }
        else
        {
            cout << "MOVE " << gridCost.maxCell.x << " " <<  gridCost.maxCell.y << endl;
        }
    }
}