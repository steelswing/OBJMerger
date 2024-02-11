/*
Ну вы же понимаете, что код здесь только мой?
Well, you do understand that the code here is only mine?
 */

/* 
 * File:   main.cpp
 * Author: LWJGL2
 *
 * Created on 11 февраля 2024 г., 16:37
 */

#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <filesystem>

class OBJMesh {
public:
    std::string name;
    std::vector<int> indices;
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> textureCoords;

    OBJMesh(const std::string& n, const std::vector<int>& ind, const std::vector<float>& vert,
            const std::vector<float>& norm, const std::vector<float>& texCoords)
    : name(n), indices(ind), vertices(vert), normals(norm), textureCoords(texCoords) {
    }
};

class OBJImporter {
public:

    static OBJMesh importMesh(const std::string& objContent) {
        std::istringstream iss(objContent);
        std::string line;
        std::string name;
        std::vector<int> indicesList;
        std::vector<float> verticesList;
        std::vector<float> normalsList;
        std::vector<float> textureCoordsList;

        while (std::getline(iss, line)) {
            if (!line.empty()) {
                line = trim(line);

                if (line.find("o ") == 0 && line.size() > 2) {
                    name = line.substr(2);
                } else if (line.find("v ") == 0) {
                    processVertexLine(line, verticesList);
                } else if (line.find("vn ") == 0) {
                    processNormalLine(line, normalsList);
                } else if (line.find("vt ") == 0) {
                    processTextureCoordLine(line, textureCoordsList);
                } else if (line.find("f ") == 0) {
                    processFaceLine(line, indicesList);
                }
            }
        }

        int* indices = listToArray(indicesList);
        float* vertices = listToArray(verticesList);
        float* normals = listToArray(normalsList);
        float* textureCoords = listToArray(textureCoordsList);

        return OBJMesh(name, std::vector<int>(indices, indices + indicesList.size()),
                std::vector<float>(vertices, vertices + verticesList.size()),
                std::vector<float>(normals, normals + normalsList.size()),
                std::vector<float>(textureCoords, textureCoords + textureCoordsList.size()));
    }

private:

    static void processVertexLine(const std::string& line, std::vector<float>& verticesList) {
        std::istringstream iss(line);
        std::string token;
        iss >> token; // Skip 'v'
        while (iss >> token) {
            verticesList.push_back(std::stof(token));
        }
    }

    static void processNormalLine(const std::string& line, std::vector<float>& normalsList) {
        std::istringstream iss(line);
        std::string token;
        iss >> token; // Skip 'vn'
        while (iss >> token) {
            normalsList.push_back(std::stof(token));
        }
    }

    static void processTextureCoordLine(const std::string& line, std::vector<float>& textureCoordsList) {
        std::istringstream iss(line);
        std::string token;
        iss >> token; // Skip 'vt'
        while (iss >> token) {
            textureCoordsList.push_back(std::stof(token));
        }
    }

    static void processFaceLine(const std::string& line, std::vector<int>& indicesList) {
        std::istringstream iss(line);
        std::string token;
        iss >> token; // Skip 'f'
        while (iss >> token) {
            std::size_t pos = token.find_first_of('/');
            int index = std::stoi(token.substr(0, pos)) - 1;
            indicesList.push_back(index);
        }
    }

    static float* listToArray(const std::vector<float>& list) {
        float* array = new float[list.size()];
        std::copy(list.begin(), list.end(), array);
        return array;
    }

    static int* listToArray(const std::vector<int>& list) {
        int* array = new int[list.size()];
        std::copy(list.begin(), list.end(), array);
        return array;
    }

    static std::string trim(const std::string& str) {
        std::size_t first = str.find_first_not_of(" \t\n\r\f\v");
        std::size_t last = str.find_last_not_of(" \t\n\r\f\v");
        return str.substr(first, (last - first + 1));
    }
};

class OBJExporter {
public:

    static std::string writeMultiple(const std::string& mtl, const std::vector<OBJMesh>& meshes) {
        std::ostringstream str;
        int vertexCount = 1;
        int uvCount = 1;

        str << "mtllib ./ogo.mtl" << "\n";

        for (const auto& objMesh : meshes) {
            str << "o " << objMesh.name << "\n";
            str << "usemtl " << mtl << "\n";

            // Vertices
            str << "# VERTICES \n";
            for (size_t i = 0; i < objMesh.vertices.size(); i += 3) {
                str << "v " << objMesh.vertices[i] << " "
                        << objMesh.vertices[i + 1] << " "
                        << objMesh.vertices[i + 2] << "\n";
            }

            if (!objMesh.normals.empty()) {
                // Normals
                str << "# NORMALS \n";
                for (size_t i = 0; i < objMesh.normals.size(); i += 3) {
                    str << "vn " << objMesh.normals[i] << " "
                            << objMesh.normals[i + 1] << " "
                            << objMesh.normals[i + 2] << "\n";
                }
            }

            if (!objMesh.textureCoords.empty()) {
                // Texture coords
                str << "# UV \n";
                for (size_t i = 0; i < objMesh.textureCoords.size(); i += 2) {
                    str << "vt " << objMesh.textureCoords[i] << " "
                            << 1.0f - objMesh.textureCoords[i + 1] << "\n";
                }
            }

            // Indices
            str << "# FACES \n";
            for (size_t i = 0; i < objMesh.indices.size(); i += 3) {
                int v1 = objMesh.indices[i] + vertexCount;
                int uv1 = objMesh.indices[i] + uvCount;
                int v2 = objMesh.indices[i + 1] + vertexCount;
                int uv2 = objMesh.indices[i + 1] + uvCount;
                int v3 = objMesh.indices[i + 2] + vertexCount;
                int uv3 = objMesh.indices[i + 2] + uvCount;

                std::string face = "f " + std::to_string(v1) + "/" + std::to_string(uv1) + " "
                        + std::to_string(v2) + "/" + std::to_string(uv2) + " "
                        + std::to_string(v3) + "/" + std::to_string(uv3);
                str << face << "\n";
            }
            str << "\n";

            vertexCount += objMesh.vertices.size() / 3;
            uvCount += objMesh.textureCoords.size() / 2;
        }

        return str.str();
    }
};

using namespace std;
namespace fs = std::filesystem;

std::vector<std::string> getOBJFilesInDirectory(const std::string& directoryPath) {
    std::vector<std::string> objFiles;

    try {
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".obj") {
                objFiles.push_back(entry.path().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
    }

    return objFiles;
}

void writeStringToFile(const std::string& content, const std::string& filePath) {
    std::ofstream outputFile(filePath);

    if (outputFile.is_open()) {
        outputFile << content;
        outputFile.close();
        std::cout << "Successfully wrote to file: " << filePath << std::endl;
    } else {
        std::cerr << "Error: Unable to open file " << filePath << " for writing." << std::endl;
    }
}

std::string readFileToString(const std::string& filePath) {
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open file " << filePath << std::endl;
        return "";
    }

    std::ostringstream contentStream;
    contentStream << inputFile.rdbuf();
    inputFile.close();

    return contentStream.str();
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Use: objmerger.exe <obj dir> output.obj" << std::endl;
        return 0;
    }
    std::string directoryPath = std::string(argv[1]);
    std::cout << "DirectoryPath: " << directoryPath.c_str() << std::endl;

    std::vector<std::string> objFiles = getOBJFilesInDirectory(directoryPath);
    std::cout << "ObjFiles: " << std::to_string(objFiles.size()).c_str() << std::endl;

    std::vector<OBJMesh> meshes;
    for (std::string objFilePath : objFiles) {
        std::cout << "Reading obj: " << objFilePath.c_str();

        std::string objContent = readFileToString(objFilePath);
        OBJMesh mesh = OBJImporter::importMesh(objContent);

        std::cout << " | Mesh vertices: " << std::to_string(mesh.vertices.size()).c_str() << ", indices: " << std::to_string(mesh.indices.size()).c_str() << std::endl;
        meshes.push_back(mesh);
    }
    std::cout << "Merging..." << std::endl;
    std::string mergedObj = OBJExporter::writeMultiple("default", meshes);
    writeStringToFile(mergedObj, argv[2]);
    return 0;
}
