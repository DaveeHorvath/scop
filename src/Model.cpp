#include "Model.hpp"
#include <fstream>
#include <stdexcept>
#include <string>
#include <sstream>

void Model::loadModel()
{
    std::vector<Vertex> read_verticies;
    std::vector<int> read_indicies;
    std::ifstream file{"teapot.obj"};
    if (!file.is_open())
        throw std::runtime_error("Failed to open file");
    for (std::string line; std::getline(file, line);)
    {
        std::stringstream ss{line};
        char type;
        ss >> type;
        if (type == 'v')
        {
            Vertex v;
            ss >> v.pos.x >> v.pos.y >> v.pos.z;
            read_verticies.push_back(v);
        }
        else if (type == 'f')
        {
            std::vector<int> face;
            face.clear();
            for (std::string item; std::getline(ss, item, ' ');)
            {
                if (item != "")
                    face.push_back(std::stoi(item));
            }
            for (int i = 0; i + 2 < face.size(); i++)
            {
                read_indicies.push_back(face[i]);
                read_indicies.push_back(face[i + 1]);
                read_indicies.push_back(face[i + 2]);
            }
        }
    }

    for (auto &i : read_indicies)
    {
        indices.push_back(indices.size());
        vertices.push_back(read_verticies[i - 1]);
    }
}