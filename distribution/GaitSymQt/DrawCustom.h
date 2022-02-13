#ifndef DRAWCUSTOM_H
#define DRAWCUSTOM_H

#include "Drawable.h"

#include <memory>

class DrawCustom : public Drawable
{
public:
    DrawCustom();
    virtual ~DrawCustom();

    virtual void initialise(SimulationWidget *simulationWidget);
    virtual void Draw();
    virtual std::string name();

    void addFacetedObject(std::unique_ptr<FacetedObject> &&facetedObject);

    void setName(const std::string &name);

private:
    std::vector<std::unique_ptr<FacetedObject>> m_facetedObjectStore;
    std::string m_name;
};

#endif // DRAWCUSTOM_H
