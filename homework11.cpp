#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <math.h>
#include <time.h>

using namespace std;

#define POPULATION_SIZE 200
#define CROSSOVER_RATE 0.8
#define MUTATION_RATE 0.5

class Location {
public:
    int x;
    int y;
    int z;

    Location(){}

    Location(int _x, int _y, int _z) {
        this->x = _x;
        this->y = _y;
        this->z = _z;
    }
};

struct Path {
    vector<int> path;
    double pathValue;
};

bool lessPath(const Path& p1, const Path& p2) {
    return p1.pathValue > p2.pathValue;
}

// tools function
double getDistance(Location loc1, Location loc2) {
    double dis;
    double deltX = 1.0 * loc1.x - 1.0 * loc2.x;
    double deltY = 1.0 * loc1.y - 1.0 * loc2.y;
    double deltZ = 1.0 * loc1.z - 1.0 * loc2.z;
    dis = pow(deltX, 2) + pow(deltY, 2) + pow(deltZ, 2);
    dis = sqrt(dis);

    return dis;
}

int getFactorial(int n) {
    int res = 1;
    for (int i = n; i >= 1; --i) {
        res *= i;
    }

    return res;
}

// Global variables
int cityNum;

vector<Path> generation;

unordered_map<int, Location> locationMap;
unordered_map<int, unordered_map<int, double>> pathMap;

//*******************************************************************
// helpers
void printPathes() {
    for (int i = 0; i < generation.size(); ++i) {
        cout << "*******************" << endl;
        cout << "Path " << i << " : ";
        for (int j : generation[i].path) {
            cout << j << " ";
        }
        cout << "Path value : " << generation[i].pathValue << endl;
    }
}

double updatePathValue(Path *p) {
    int size = p->path.size();
    p->pathValue = 0;
    for (int i = 1; i < size; ++i) {
        int prev = p->path[i - 1];
        int cur = p->path[i];
        p->pathValue += pathMap[prev][cur];
    }
    p->pathValue = 1.0 / p->pathValue;
}

void insertPathIntoVec(vector<int> path) {
    Path p;
    p.path = path;
    updatePathValue(&p);
    generation.push_back(p);
}

int rouletteWheelSelection(double sum) {
    double randomNum = rand() / (double) RAND_MAX;
    double offset = 0;
    int pick = 0;

    for (int i = 0; i < generation.size(); ++i) {
        offset += generation[i].pathValue / sum;
        if (randomNum < offset) {
            pick = i;
            break;
        }
    }

    return pick;
}

// greedy generate the first path
vector<int> initalPathGenerator(int start) {
    vector<int> res;
    res.push_back(start);
    bool visited[cityNum];
    for (int i = 0; i < cityNum; ++i) {
        visited[i] = false;
    }
    visited[start] = true;
    int prev = start;
    int size = 1;
    while (size < cityNum){
        int minLoc = -1;
        double minDis = -1;
        for (auto& kv : pathMap[prev]) {
            int next = kv.first;
            double nextDist = kv.second;
            if (visited[next]) {
                continue;
            }

            if (minLoc == -1 || nextDist < minDis) {
                minLoc = next;
                minDis = nextDist;
            }
        }
        visited[minLoc] = true;
        res.push_back(minLoc);
        prev = minLoc;
        size++;
    }

    res.push_back(start);

    return res;
}

Path crossOverAndMutation(Path p1, Path p2) {
    int point1;
    int point2;
    Path child;
    for (int i = 0; i < cityNum; ++i) {
        child.path.push_back(-1);
    }

    point1 = rand() % cityNum;
    point2 = rand() % cityNum;
    if (point1 > point2) {
        int tmp = point1;
        point1 = point2;
        point2 = tmp;
    }

    bool visited[cityNum];
    for (int i = 0; i < cityNum; ++i) {
        visited[i] = false;
    }

    for (int i = point1; i <= point2; ++i) {
        child.path[i] = p2.path[i];
        visited[p2.path[i]] = true;
    }

    for (int i = 0; i < cityNum; ++i) {
        int cur = p1.path[i];

        if (i >= point1 && i <= point2) {
            continue;
        }

        if (!visited[cur]) {
            child.path[i] = cur;
            visited[cur] = true;
        }
    }

    int idx = 0;
    for (int i = 0; i < cityNum; ++i) {
        if (child.path[i] != -1) {
            continue;
        }
        while (idx < cityNum && visited[idx]) {
            idx++;
        }
        child.path[i] = idx;
        visited[idx] = true;
    }
    child.path.push_back(child.path[0]);

    double isMutate = rand() / (double) RAND_MAX;
    if (isMutate < MUTATION_RATE) {
        int change1 = rand() % cityNum;
        int change2 = rand() % cityNum;
        if (change1 != change2) {
            int tmp = child.path[change1];
            child.path[change1] = child.path[change2];
            child.path[change2] = tmp;
            if (change1 == 0 || change2 == 0) {
                child.path.pop_back();
                child.path.push_back(child.path[0]);
            }
        }
    }

    updatePathValue(&child);

    return child;
}

//*******************************************************************

// OS operations
void readFile() {
    ifstream inputFile("input.txt");
    int cityNumber = 0;
    if (inputFile.is_open()) {
        inputFile >> cityNum;
        for (int i = 0; i < cityNum; ++i) {
            Location loc;
            inputFile >> loc.x;
            inputFile >> loc.y;
            inputFile >> loc.z;
            locationMap[cityNumber] = loc;
            cityNumber++;
        }
    }
    inputFile.close();
}

void writeFile(const Path& p) {
    ofstream outputFile("output.txt");
    if (outputFile.is_open()) {
        int pPathSize = p.path.size();
        for (int i = 0; i < pPathSize; ++i) {
            Location cur = locationMap[p.path[i]];
            outputFile << cur.x << " ";
            outputFile << cur.y << " ";
            outputFile << cur.z;
            if (i != pPathSize - 1) {
                outputFile << endl;
            }
        }
    }
    outputFile.close();
}

void writeFileWithOutContent() {
    ofstream outputFile("output.txt");
    outputFile.close();
}

// GA algorithm
void buildMap() {
    for (int i = 0; i < cityNum; ++i) {
        for (int j = 0; j < cityNum; ++j) {
            if (i == j) {
                continue;
            }
            double dis = getDistance(locationMap[i], locationMap[j]);
            pathMap[i][j] = dis;
        }
    }
}

int initPopulation(int popuSize) {
    // 12! will exceed the boundary of int in C++
    // to ensure that create the population without duplicates
    if (cityNum < 12) {
        popuSize = min(popuSize, getFactorial(cityNum));
    }

    // generate the initial population with different start point
    int batchNum = popuSize / cityNum;
    int remainder = popuSize % cityNum;
    for (int i = 0; i < cityNum; ++i) {
        vector<int> cur = initalPathGenerator(i);
        insertPathIntoVec(cur);
        auto rng = default_random_engine {};
        for (int j = 1; j < batchNum; ++j) {
            shuffle(begin(cur) + 1, end(cur) - 1, rng);
            insertPathIntoVec(cur);
        }

        if (remainder > 0) {
            shuffle(begin(cur) + 1, end(cur) - 1, rng);
            insertPathIntoVec(cur);
            remainder--;
        }
    }

    sort(generation.begin(), generation.end(), lessPath);

    return popuSize;
}

void nextGeneration(int size) {
//    printPathes();
    double sum = 0;
    for (const Path& p : generation) {
        sum += p.pathValue;
    }

    vector<Path> children;
    for (int i = 0; i < size; ++i) {
        int pick1 = rouletteWheelSelection(sum);
        int pick2 = rouletteWheelSelection(sum);
        Path child = crossOverAndMutation(generation[pick1], generation[pick2]);
        children.push_back(child);
    }

    generation.erase(generation.end() - size);

    generation.insert(generation.end(), children.begin(), children.end());

    sort(generation.begin(), generation.end(), lessPath);
}



void helper(int itrTimes) {
    srand((unsigned )time(NULL));
    readFile();
    if (cityNum == 0) {
        writeFileWithOutContent();
    }
    buildMap();
    int size = initPopulation(POPULATION_SIZE);
    int offspringSize = int(CROSSOVER_RATE * size);
    while (itrTimes-- > 0) {
        nextGeneration(offspringSize);
    }

    Path lowest = generation[0];

    writeFile(lowest);
}

int main() {
    helper(100);
    return 0;
}
