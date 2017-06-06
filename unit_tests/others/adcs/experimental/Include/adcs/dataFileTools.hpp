#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

namespace dataFileTools
{
    static std::vector<float> getRecord(std::istream& str)
    {
        std::vector<float> result;
        std::string line, cell;
        std::getline(str, line);
        std::stringstream lineStream(line);
        float cellf;
        while (getline(lineStream, cell, ','))
        {
            std::stringstream cellStream(cell);
            cellStream >> cellf;
            result.push_back(cellf);
        }
        return result;
    }
}
