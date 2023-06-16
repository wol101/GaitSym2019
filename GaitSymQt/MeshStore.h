/*
 *  MeshStore.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 22/04/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef MESHSTORE_H
#define MESHSTORE_H

#include "ode/ode.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <memory>

struct MeshStoreObject
{
    uint64_t size() const
    {
        return (vertexList.size() * sizeof(double) +
                normalList.size() * sizeof(double) +
                colourList.size() * sizeof(double) +
                uvList.size() * sizeof(double) +
                sizeof(MeshStoreObject));
    }
    std::string path;
    std::vector<double> vertexList;
    std::vector<double> normalList;
    std::vector<double> colourList;
    std::vector<double> uvList;
    dVector3 lowerBound = {DBL_MAX, DBL_MAX, DBL_MAX, 0};
    dVector3 upperBound = {-DBL_MAX, -DBL_MAX, -DBL_MAX, 0};
};

class MeshStore
{
public:
    MeshStore();

    MeshStoreObject *getMesh(const std::string &path);
    void addMesh(const MeshStoreObject &meshStoreObject);
    void addMesh(const std::string &path, const std::vector<double> &vertexList, const std::vector<double> &normalList,
                 const std::vector<double> &colourList, const std::vector<double> &uvList,
                 const dVector3 &lowerBound, const dVector3 &upperBound);
    void clear();

    uint64_t getCurrentMemory();
    void setTargetMemory(uint64_t targetMemory);
    void setTargetMemory(double targetMemoryFraction);
    uint64_t getTotalSystemMemory();


private:
    std::unordered_map<std::string, std::unique_ptr<MeshStoreObject>> m_meshMap;
    std::map<uint64_t, std::string> m_lastAccessedMapByTime;
    std::unordered_map<std::string, uint64_t> m_lastAccessedMapByName;
    uint64_t m_timeCount = 0;
    uint64_t m_targetMemory = 0;
};

#endif // MESHSTORE_H
