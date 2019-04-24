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
        return '0' == m_grid[y][x];
    }
    bool IsBoxSafeCheck(int x, int y) const
    { 
        if (x < 0 || x >= (int)m_width || y < 0 || y >= (int)m_height)
            return false;
        else
            return '0' == m_grid[y][x];
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

class GridCostEstimator
{
public:
    GridCostEstimator(const Grid &grid)
        : m_grid(grid)
        , m_width(m_grid.m_width)
        , m_height(m_grid.m_height)
        , m_zeroRow(m_width, 0)
        , m_gridCost(m_height, m_zeroRow)
        , max(0)
    {
        for (size_t y = 0; y < m_height; y++) {
            for (size_t x = 0; x < m_width; x++) {
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
        int rangeX[2] = {static_cast<int>(boxX)- static_cast<int>(bombRange)+1, static_cast<int>(boxX)+ static_cast<int>(bombRange)-1};
        clip<int>(rangeX[0], 0, m_width);
        clip<int>(rangeX[1], 0, m_width);
        
        int rangeY[2] = {(int)(boxY)-bombRange+1, (int)(boxY)+bombRange-1};
        clip<int>(rangeY[0], 0, m_height);
        clip<int>(rangeY[1], 0, m_height);

        for (int x = rangeX[0]; x < rangeX[1]; x++) {
            if (boxX != x && !m_grid.IsBox(x,boxY))
            {
                m_gridCost[boxY][x]++;
                if (m_gridCost[boxY][x] > max)
                {
                    max = m_gridCost[boxY][x];
                    maxCell.x = x;
                    maxCell.y = boxY;
                }
            }
        }

        for (int y = rangeY[0]; y < rangeY[1]; y++) {
            if (boxY != y && !m_grid.IsBox(boxX,y))
            {
                m_gridCost[y][boxX]++;
                if (m_gridCost[y][boxX] > max)
                {
                    max = m_gridCost[y][boxX];
                    maxCell.x = boxX;
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
    
    int max;

};
 
int main()
{
    //int width;
    //int height;
    //int myId;
    //cin >> width >> height >> myId; cin.ignore();
    Rules rules;
    cin >> rules;
    

    // game loop
    while (1) {
        
        Grid grid(rules.width, rules.height);
        for (int i = 0; i < rules.height; i++) {
            string row;
            cin >> row; cin.ignore();
            cerr << row << endl;
            grid.ReadRow(row);
        }
        
        for (int x = 0; x < rules.width; x++)
        {
            cerr << "--";
        };
        cerr << endl;
        
        GridCostEstimator gridCost(grid);
        cerr << gridCost;
        
        int entities;
        cin >> entities; cin.ignore();
        
        int myBombRoundsLeft = -1;
        
        
        Entity *me = NULL;
        std::vector<Entity> entitiesList(entities);
        for (int i = 0; i < entities; i++) {
            Entity e;
            cin >> e.entityType
                >> e.owner
                >> e.x
                >> e.y
                >> e.param1
                >> e.param2;
            entitiesList.push_back(e);
            if (ENTITY_TYPE_PLAYER == e.entityType && rules.myId == e.owner)
            {
                me = &(entitiesList.back());
            }
            
            cin.ignore();
            //if (ENTITY_TYPE_BOMB == entityType && 0 == owner && )
            //{
            //    
            //}
        }
        

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
        //vec.back().c
        
        if (grid.IsBoxAround(me->x, me->y))
        {
            cout << "BOMB " << gridCost.maxCell.x << " " <<  gridCost.maxCell.y << endl;
        }
        else
        {
            cout << "MOVE " << gridCost.maxCell.x << " " <<  gridCost.maxCell.y << endl;
        }
    }
}