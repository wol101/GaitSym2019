#include "DrawCustom.h"

#include <FacetedObject.h>

DrawCustom::DrawCustom()
{
}

void DrawCustom::initialise(SimulationWidget * /*simulationWidget*/)
{
}

void DrawCustom::Draw()
{
    for (auto &&it : m_facetedObjectList) it->Draw();
}

void DrawCustom::addFacetedObject(std::unique_ptr<FacetedObject> &&facetedObject)
{
    m_facetedObjectList.push_back(facetedObject.get());
    m_facetedObjectStore.push_back(std::move(facetedObject));
}

std::string DrawCustom::name()
{
    return m_name;
}

void DrawCustom::setName(const std::string &name)
{
    m_name = name;
}

