#include "Model.hpp"
#include <fstream>
#include <stdexcept>
#include <string>
#include <sstream>

#include <iostream>

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
            v.texCoord = {v.pos.x / glm::length(v.pos), -v.pos.y / glm::length(v.pos)};
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
            if (face.size() != 3)
                std::cout << "weird shit detected\n";
            for (int i = 0; i + 2 < face.size(); i++)
            {
                read_indicies.push_back(face[i + 2]);
                read_indicies.push_back(face[i]);
                read_indicies.push_back(face[i + 1]);
            }
        }
    }

    for (auto &i : read_indicies)
    {
        indices.push_back(indices.size());
        vertices.push_back(read_verticies[i - 1]);
    }
}