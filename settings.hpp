#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

using namespace std;

extern float activeAreaMargin;
extern int windowHeight;
extern int windowWidth;
extern int frustumHeight;
extern bool useSSBO;
extern float terrainSize;
extern int numVerticesPerRow;
extern float compression;
extern float roughness;
extern float slopeThreshold;
extern int numIterationsBlurNormals;
extern int numIterationsEvenOutSlopes;
extern int numIterationsDisplaceMaterialToContour;
extern float heightmapSize;

bool parseBoolean(string value) {
    if (value == "true" || value == "TRUE") {
        return true;
    } else if (value == "false" || value == "FALSE") {
        return false;
    }
    throw invalid_argument("Invalid boolean value");
}

void applySetting(string setting, string stringValue) {
    if (setting == "numIterationsEvenOutSlopes") {
        numIterationsEvenOutSlopes = stoi(stringValue);
    } else if (setting == "slopeThreshold") {
        slopeThreshold = stof(stringValue);
    } else if (setting == "compression") {
        compression = stof(stringValue);
    } else if (setting == "roughness") {
        roughness = stof(stringValue);
    } else if (setting == "heightmapSize") {
        heightmapSize = stof(stringValue);
    } else if (setting == "terrainSize") {
        terrainSize = stof(stringValue);
    } else if (setting == "numIterationsBlurNormals") {
        numIterationsBlurNormals = stof(stringValue);
    } else if (setting == "numVerticesPerRow") {
        numVerticesPerRow = stof(stringValue);
    } else if (setting == "numIterationsDisplaceMaterialToContour") {
        numIterationsDisplaceMaterialToContour = stof(stringValue);
    } else if (setting == "activeAreaMargin") {
        activeAreaMargin = stof(stringValue);
    } else if (setting == "windowWidth") {
        windowWidth = stoi(stringValue);
    } else if (setting == "windowHeight") {
        windowHeight = stoi(stringValue);
    } else if (setting == "frustumHeight") {
        frustumHeight = stoi(stringValue);
    } else if (setting == "useSSBO") {
        useSSBO = parseBoolean(stringValue);
    } else {
        throw invalid_argument("No setting exists for " + setting);
    }
}

void applySettings(string settingFile) {
    // Read file
    std::ifstream inputFile;
    inputFile.open(settingFile);
    if (!inputFile) {
        cerr << "Could not open file " << settingFile << endl;
        exit(1);
    }
    std::stringstream buffer;
    buffer << inputFile.rdbuf();

    string setting;
    string value;
    while (!buffer.eof()) {
        getline(buffer, setting, '=');
        getline(buffer, value);
        applySetting(setting, value);
    }
}