/*
 *  Colour.h
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 29/3/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#ifndef COLOUR_H
#define COLOUR_H

#include "SmartEnum.h"

#include <string>

class Colour
{
public:
    SMART_ENUM(ColourMap, colourMapStrings, colourMapCount,  GreyColourMap, CoolColourMap, HotColourMap, HSVColourMap, JetColourMap, ParulaColourMap);
//    enum ColourMap
//    {
//        GreyColourMap,
//        CoolColourMap,
//        HotColourMap,
//        HSVColourMap,
//        JetColourMap,
//        ParulaColourMap
//    };

    Colour();
    Colour(float rf, float gf, float bf, float af) { m_r = rf;  m_g = gf;  m_b = bf;  m_alpha = af; }
    Colour(double rf, double gf, double bf, double af) { m_r = float(rf);  m_g = float(gf);  m_b = float(bf);  m_alpha = float(af); }
    Colour(int rf, int gf, int bf, int af) { m_r = float(rf) / 255.f;  m_g = float(gf) / 255.f;  m_b = float(bf) / 255.f;  m_alpha = float(af) / 255.f; }
    Colour(const Colour &c) { m_r = c.m_r;  m_g = c.m_g;  m_b = c.m_b;  m_alpha = c.m_alpha; }
    Colour(float *c) { m_r = c[0]; m_g = c[1]; m_b = c[2]; m_alpha = c[3]; }
    Colour(double *c) { m_r = float(c[0]); m_g = float(c[1]); m_b = float(c[2]); m_alpha = float(c[3]); }
    Colour(int *c) { m_r = float(c[0]) / 255.f; m_g = float(c[1]) / 255.f; m_b = float(c[2]) / 255.f; m_alpha = float(c[3]) / 255.f; }
    Colour(float index, ColourMap m, bool invert = false);
    Colour(const std::string &str);

    void SetColour(float rf, float gf, float bf, float af) { m_r = rf;  m_g = gf;  m_b = bf;  m_alpha = af; }
    void SetColour(double rf, double gf, double bf, double af) { m_r = float(rf);  m_g = float(gf);  m_b = float(bf);  m_alpha = float(af); }
    void SetColour(int rf, int gf, int bf, int af) { m_r = float(rf) / 255.f;  m_g = float(gf) / 255.f;  m_b = float(bf) / 255.f;  m_alpha = float(af) / 255.f; }
    void SetColour(const Colour &c) { m_r = c.m_r;  m_g = c.m_g;  m_b = c.m_b;  m_alpha = c.m_alpha; }
    void SetColour(float *c) { m_r = c[0]; m_g = c[1]; m_b = c[2]; m_alpha = c[3]; }
    void SetColour(double *c) { m_r = float(c[0]); m_g = float(c[1]); m_b = float(c[2]); m_alpha = float(c[3]); }
    void SetColour(float index, ColourMap m, bool invert = false);
    bool SetColour(const std::string &name);
    float *data() { return &m_r; }
    explicit operator float*() { return &m_r; }
    std::string GetFloatColourRGBA();
    std::string GetIntColourRGBA();
    std::string GetHexArgb();

    static void SetColourFromMap(float index, ColourMap m, Colour *mappedColour, bool invert = false);
    static bool SetColourFromName(const std::string &name, Colour *namedColour);
    static bool SetColourFromString(const std::string &str, Colour *stringColour);
    static bool IsInt(const std::string &s);
    static bool IsNumber(const std::string& s);

    float r() const;
    void setR(float r);

    float g() const;
    void setG(float g);

    float b() const;
    void setB(float b);

    float alpha() const;
    void setAlpha(float alpha);

private:
    float m_r = {0};
    float m_g = {0};
    float m_b = {0};
    float m_alpha = {1};
};


#endif // COLOUR_H
