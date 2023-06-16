/*
 *  MeshStore.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 22/04/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "MeshStore.h"
#include "FacetedObject.h"

MeshStore FacetedObject::m_meshStore;

MeshStore::MeshStore()
{
}

void MeshStore::clear()
{
    m_meshMap.clear();
    m_lastAccessedMapByTime.clear();
    m_lastAccessedMapByName.clear();
    m_timeCount = 0;
    m_targetMemory = 0;
}

MeshStoreObject *MeshStore::getMesh(const std::string &path)
{
    auto it = m_meshMap.find(path);
    if (it != m_meshMap.end())
    {
        auto timeIt = m_lastAccessedMapByName.find(path);
        m_lastAccessedMapByTime.erase(timeIt->second);
        timeIt->second = m_timeCount;
        m_lastAccessedMapByTime[m_timeCount] = path;
        m_timeCount++;
        return it->second.get();
    }
    return nullptr;
}


void MeshStore::addMesh(const MeshStoreObject &meshStoreObject)
{
    if (meshStoreObject.size() > m_targetMemory) return;

    auto mesh = std::make_unique<MeshStoreObject>();
    *mesh = meshStoreObject;

    auto timeIt = m_lastAccessedMapByName.find(mesh->path);
    if (timeIt != m_lastAccessedMapByName.end())
    {
        m_lastAccessedMapByTime.erase(timeIt->second);
        timeIt->second = m_timeCount;
    }
    else m_lastAccessedMapByName[mesh->path] = m_timeCount;
    m_lastAccessedMapByTime[m_timeCount] = mesh->path;
    m_timeCount++;

    m_meshMap[meshStoreObject.path] = std::move(mesh);
}

void MeshStore::addMesh(const std::string &path, const std::vector<double> &vertexList, const std::vector<double> &normalList,
                        const std::vector<double> &colourList, const std::vector<double> &uvList, const dVector3 &lowerBound, const dVector3 &upperBound)
{
    if ((vertexList.size() * sizeof(double) +
         normalList.size() * sizeof(double) +
         colourList.size() * sizeof(double) +
         uvList.size() * sizeof(double) +
         sizeof(MeshStoreObject)) > m_targetMemory) return;

    auto mesh = std::make_unique<MeshStoreObject>();
    mesh->path = path;
    mesh->vertexList = vertexList;
    mesh->normalList = normalList;
    mesh->colourList = colourList;
    mesh->uvList = uvList;
    mesh->lowerBound[0] = lowerBound[0];
    mesh->lowerBound[1] = lowerBound[1];
    mesh->lowerBound[2] = lowerBound[2];
    mesh->upperBound[0] = upperBound[0];
    mesh->upperBound[1] = upperBound[1];
    mesh->upperBound[2] = upperBound[2];

    auto timeIt = m_lastAccessedMapByName.find(mesh->path);
    if (timeIt != m_lastAccessedMapByName.end())
    {
        m_lastAccessedMapByTime.erase(timeIt->second);
        timeIt->second = m_timeCount;
    }
    else m_lastAccessedMapByName[mesh->path] = m_timeCount;
    m_lastAccessedMapByTime[m_timeCount] = mesh->path;
    m_timeCount++;

    m_meshMap[path] = std::move(mesh);
}

uint64_t MeshStore::getCurrentMemory()
{
    size_t totalMemory = 0;
    for (auto it = m_meshMap.begin(); it != m_meshMap.end(); it++)
    {
        totalMemory += it->second->size();
    }
    return totalMemory;
}

void MeshStore::setTargetMemory(uint64_t targetMemory)
{
    m_targetMemory = targetMemory;
    while (getCurrentMemory() > targetMemory)
    {
        auto it = m_lastAccessedMapByTime.begin();
        if (it == m_lastAccessedMapByTime.end()) break;
        m_lastAccessedMapByName.erase(it->second);
        m_meshMap.erase(it->second);
        it = m_lastAccessedMapByTime.erase(it);
    }
}

void MeshStore::setTargetMemory(double targetMemoryFraction)
{
    uint64_t targetMemory = uint64_t(getTotalSystemMemory() * targetMemoryFraction);
    setTargetMemory(targetMemory);
}


#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>

uint64_t MeshStore::getTotalSystemMemory()
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return uint64_t(status.ullTotalPhys);
}
#else
#include <unistd.h>

uint64_t MeshStore::getTotalSystemMemory()
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return uint64_t(pages) * uint64_t(page_size);
}
#endif
