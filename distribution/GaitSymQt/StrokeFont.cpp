/* routine to draw a stroke font character */
/* uses routine AddLine(ix1,iy1,ix2,iy2) to draw character */

/* cwidth includes a reasonable space between characters, so don't add extra
 space between characters */

/* cheight is maximum cheight above baseline, but characters may descend some
 distance below this (max 3/7 or 0.43), so add c. 50% of cheight between
 lines to prevent overlap */

/* coordinate system assumes origin at bottom left (I think) */

#include "StrokeFont.h"
#include "PGDMath.h"

#include "ode/ode.h"

#include "GLUtils.h"
#include "SimulationWidget.h"

#include <QOpenGLFunctions_3_3_Core>
#if QT_VERSION >= 0x060000
#include <QOpenGLVersionFunctionsFactory>
#endif

#include <algorithm>

const size_t line_buffer_stride = 7;

StrokeFont::StrokeFont()
{
    dRSetIdentity(m_displayRotation);
    // the line buffer is organised x1,y1,z1,r1,g1,b1,a1,x2,y2,z2,r2,g2,b2,a2
    line_buffer = new float[max_lines * line_buffer_stride * 2];
}

StrokeFont::~StrokeFont()
{
    delete [] line_buffer;
}

void StrokeFont::AddLine(float ix1, float iy1, float iz1, float ix2, float iy2, float iz2)
{
    if (n_lines >= max_lines) return; // at some point I might want to dynamically increase max_lines

    float *fp = line_buffer + n_lines * line_buffer_stride * 2;
    *fp++ = ix1;
    *fp++ = iy1;
    *fp++ = iz1;

    *fp++ = m_r;
    *fp++ = m_g;
    *fp++ = m_b;
    *fp++ = m_a;

    *fp++ = ix2;
    *fp++ = iy2;
    *fp++ = iz2;

    *fp++ = m_r;
    *fp++ = m_g;
    *fp++ = m_b;
    *fp++ = m_a;

    n_lines++;
}

void StrokeFont::AddLine(float ix1, float iy1, float iz1, float ix2, float iy2, float iz2, const float *matrix, const float *translation)
{
    if (n_lines >= max_lines) return; // at some point I might want to dynamically increase max_lines

    // matrix is an ODE style dMatrix3 so multiply first and then add the translation
    float mix1, miy1, miz1, mix2, miy2, miz2;
    if (matrix && translation)
    {
        mix1 = matrix[0] * ix1 + matrix[1] * iy1 + matrix[2] * iz1 + translation[0];
        miy1 = matrix[4] * ix1 + matrix[5] * iy1 + matrix[6] * iz1 + translation[1];
        miz1 = matrix[8] * ix1 + matrix[9] * iy1 + matrix[10] * iz1 + translation[2];
        mix2 = matrix[0] * ix2 + matrix[1] * iy2 + matrix[2] * iz2 + translation[0];
        miy2 = matrix[4] * ix2 + matrix[5] * iy2 + matrix[6] * iz2 + translation[1];
        miz2 = matrix[8] * ix2 + matrix[9] * iy2 + matrix[10] * iz2 + translation[2];
    }
    else if (matrix)
    {
        mix1 = matrix[0] * ix1 + matrix[1] * iy1 + matrix[2] * iz1;
        miy1 = matrix[4] * ix1 + matrix[5] * iy1 + matrix[6] * iz1;
        miz1 = matrix[8] * ix1 + matrix[9] * iy1 + matrix[10] * iz1;
        mix2 = matrix[0] * ix2 + matrix[1] * iy2 + matrix[2] * iz2;
        miy2 = matrix[4] * ix2 + matrix[5] * iy2 + matrix[6] * iz2;
        miz2 = matrix[8] * ix2 + matrix[9] * iy2 + matrix[10] * iz2;
    }
    else if (translation)
    {
        mix1 = ix1 + translation[0];
        miy1 = iy1 + translation[1];
        miz1 = iz1 + translation[2];
        mix2 = ix2 + translation[0];
        miy2 = iz2 + translation[1];
        miz2 = iy2 + translation[2];
    }
    else
    {
        mix1 = ix1;
        miy1 = iy1;
        miz1 = iz1;
        mix2 = ix2;
        miy2 = iy2;
        miz2 = iz2;
    }

    float *fp = line_buffer + n_lines * line_buffer_stride * 2;
    *fp++ = mix1;
    *fp++ = miy1;
    *fp++ = miz1;

    *fp++ = m_r;
    *fp++ = m_g;
    *fp++ = m_b;
    *fp++ = m_a;

    *fp++ = mix2;
    *fp++ = miy2;
    *fp++ = miz2;

    *fp++ = m_r;
    *fp++ = m_g;
    *fp++ = m_b;
    *fp++ = m_a;

    n_lines++;
}

void StrokeFont::AddPoint(float ix1, float iy1, float iz1)
{
    if (m_start_line_flag)
    {
        m_start_line_flag = false;
        m_last_x = ix1;
        m_last_y = iy1;
        m_last_z = iz1;
    }
    else
    {
        AddLine(m_last_x, m_last_y, m_last_z, ix1, iy1, iz1);
        m_last_x = ix1;
        m_last_y = iy1;
        m_last_z = iz1;
    }
}

void StrokeFont::AddPoint(float ix1, float iy1, float iz1, const float *matrix, const float *translation)
{
    // matrix is an ODE style dMatrix3 so multiply first and then add the translation
    float mix1, miy1, miz1;
    if (matrix && translation)
    {
        mix1 = matrix[0] * ix1 + matrix[1] * iy1 + matrix[2] * iz1 + translation[0];
        miy1 = matrix[4] * ix1 + matrix[5] * iy1 + matrix[6] * iz1 + translation[1];
        miz1 = matrix[8] * ix1 + matrix[9] * iy1 + matrix[10] * iz1 + translation[2];
    }
    else if (matrix)
    {
        mix1 = matrix[0] * ix1 + matrix[1] * iy1 + matrix[2] * iz1;
        miy1 = matrix[4] * ix1 + matrix[5] * iy1 + matrix[6] * iz1;
        miz1 = matrix[8] * ix1 + matrix[9] * iy1 + matrix[10] * iz1;
    }
    else if (translation)
    {
        mix1 = ix1 + translation[0];
        miy1 = iy1 + translation[1];
        miz1 = iz1 + translation[2];
    }
    else
    {
        mix1 = ix1;
        miy1 = iy1;
        miz1 = iz1;
    }

    if (m_start_line_flag)
    {
        m_start_line_flag = false;
        m_last_x = mix1;
        m_last_y = miy1;
        m_last_z = miz1;
    }
    else
    {
        AddLine(m_last_x, m_last_y, m_last_z, mix1, miy1, miz1);
        m_last_x = mix1;
        m_last_y = miy1;
        m_last_z = miz1;
    }
}



void StrokeFont::StrokeString(const char *string,    /* character string */
                  int length,            /* number of characters to draw */
                  float x,               /* x coordinate of bottom left of character */
                  float y,               /* y coordinate ... */
                  float cwidth,          /* cwidth of character cell */
                  float cheight,         /* cheight of character cell */
                  int xJustification,    /* 0 - left, 1 - centre, 2 - right */
                  int yJustification,    /* 0 - bottom, 1 - centre, 2 - top */
                  const float *matrix, const float *translation)
{
    float width = float(length) * cwidth;
    float xOrigin = 0;
    float yOrigin = 0;
    int i;
    float orig_m_z = m_z;

    /* set the origin depending on justifictaion and the plane */

    // z=0 plane
    switch (xJustification)
    {
    case 0: /* left */
        xOrigin = x;
        break;
    case 1: /* centre */
        xOrigin = x - (width / 2);
        break;
    case 2: /* right */
        xOrigin = x - width;
        break;
    }
    switch (yJustification)
    {
    case 0: /* bottom */
        yOrigin = y;
        break;
    case 1: /* centre */
        yOrigin = y - (cheight / 2);
        break;
    case 2: /* right */
        yOrigin = y - cheight;
        break;
    }

    /* loop over the characters */

    for (i = 0; i < length; i++)
    {
        StrokeCharacter(int(string[i]), xOrigin, yOrigin, cwidth, cheight, matrix, translation);
        xOrigin += cwidth;
    }

    m_z = orig_m_z; // m_z may have been changed and we don't want that
}

void StrokeFont::StrokeCharacter(
                     int ichar,            /* character code */
                     float x,              /* x coordinate of bottom left of character */
                     float y,              /* y coordinate ... */
                     float cwidth,         /* cwidth of character cell */
                     float cheight,        /* cheight of character cell */
                     const float *matrix, const float *translation)
{
    int draw;               /* draw flag */
    int istart,iend;        /* character start and end index */
    int istr;               /* index pointer */
    int istrok;             /* stroke value */
    int ix2,iy2;            /* integer stroke components */
    float x2,y2;            /* scaled line ends */
    float x1 = 0.0,y1 = 0.0;            /* scaled line starts */

    /* data for stroke font */

    static unsigned char stroke[706]=
    {

        0x80,
        0x20,0x21,0x80,0x23,0x26,
        0x24,0x26,0x80,0x54,0x56,
        0x20,0x26,0x80,0x40,0x46,0x80,0x04,0x64,0x80,0x02,0x62,
        0x2f,0x27,0x80,0x01,0x10,0x30,0x41,0x42,0x33,0x13,0x04,0x05,0x16,0x36,0x45,
        0x11,0x55,0x80,0x14,0x15,0x25,0x24,0x14,0x80,0x41,0x51,0x52,0x42,0x41,
        0x50,0x14,0x15,0x26,0x36,0x45,0x44,0x11,0x10,0x30,0x52,
        0x34,0x36,
        0x4e,0x11,0x14,0x47,
        0x0e,0x31,0x34,0x07,
        0x30,0x36,0x80,0x11,0x55,0x80,0x15,0x51,0x80,0x03,0x63,
        0x30,0x36,0x80,0x03,0x63,
        0x11,0x20,0x2f,0x0d,
        0x03,0x63,
        0x00,0x01,0x11,0x10,0x00,
        0x00,0x01,0x45,0x46,
        0x01,0x05,0x16,0x36,0x45,0x41,0x30,0x10,0x01,
        0x04,0x26,0x20,0x80,0x00,0x40,
        0x05,0x16,0x36,0x45,0x44,0x00,0x40,0x41,
        0x05,0x16,0x36,0x45,0x44,0x33,0x42,0x41,0x30,0x10,
        0x01,0x80,0x13,0x33,
        0x06,0x03,0x43,0x80,0x20,0x26,
        0x01,0x10,0x30,0x41,0x42,0x33,0x03,0x06,0x46,
        0x02,0x13,0x33,0x42,0x41,0x30,0x10,0x01,0x05,0x16,0x36,0x45,
        0x06,0x46,0x44,0x00,
        0x01,0x02,0x13,0x04,0x05,0x16,0x36,0x45,0x44,0x33,0x42,0x41,0x30,0x10,0x01,0x80,0x13,0x33,
        0x01,0x10,0x30,0x41,0x45,0x36,0x16,0x05,0x04,0x13,0x33,0x44,
        0x15,0x25,0x24,0x14,0x15,0x80,0x12,0x22,0x21,0x11,0x12,
        0x15,0x25,0x24,0x14,0x15,0x80,0x21,0x11,0x12,0x22,0x20,0x1f,
        0x30,0x03,0x36,
        0x02,0x42,0x80,0x04,0x44,
        0x10,0x43,0x16,
        0x06,0x17,0x37,0x46,0x45,0x34,0x24,0x22,0x80,0x21,0x20,
        0x50,0x10,0x01,0x06,0x17,0x57,0x66,0x63,0x52,0x32,0x23,0x24,0x35,0x55,0x64,
        0x00,0x04,0x26,0x44,0x40,0x80,0x03,0x43,
        0x00,0x06,0x36,0x45,0x44,0x33,0x42,0x41,0x30,0x00,0x80,0x03,0x33,
        0x45,0x36,0x16,0x05,0x01,0x10,0x30,0x41,
        0x00,0x06,0x36,0x45,0x41,0x30,0x00,
        0x40,0x00,0x06,0x46,0x80,0x03,0x23,
        0x00,0x06,0x46,0x80,0x03,0x23,
        0x45,0x36,0x16,0x05,0x01,0x10,0x30,0x41,0x43,0x23,
        0x00,0x06,0x80,0x03,0x43,0x80,0x40,0x46,
        0x10,0x30,0x80,0x20,0x26,0x80,0x16,0x36,
        0x01,0x10,0x30,0x41,0x46,
        0x00,0x06,0x80,0x02,0x46,0x80,0x13,0x40,
        0x40,0x00,0x06,
        0x00,0x06,0x24,0x46,0x40,
        0x00,0x06,0x80,0x05,0x41,0x80,0x40,0x46,
        0x01,0x05,0x16,0x36,0x45,0x41,0x30,0x10,0x01,
        0x00,0x06,0x36,0x45,0x44,0x33,0x03,
        0x12,0x30,0x10,0x01,0x05,0x16,0x36,0x45,0x41,0x30,
        0x00,0x06,0x36,0x45,0x44,0x33,0x03,0x80,0x13,0x40,
        0x01,0x10,0x30,0x41,0x42,0x33,0x13,0x04,0x05,0x16,0x36,0x45,
        0x06,0x46,0x80,0x20,0x26,
        0x06,0x01,0x10,0x30,0x41,0x46,
        0x06,0x02,0x20,0x42,0x46,
        0x06,0x00,0x22,0x40,0x46,
        0x00,0x01,0x45,0x46,0x80,0x40,0x41,0x05,0x06,
        0x06,0x24,0x20,0x80,0x24,0x46,
        0x06,0x46,0x45,0x01,0x00,0x40,
        0x37,0x17,0x1f,0x3f,
        0x06,0x05,0x41,0x40,
        0x17,0x37,0x3f,0x1f,
        0x04,0x26,0x44,
        0x0f,0x7f,
        0x54,0x36,
        0x40,0x43,0x34,0x14,0x03,0x01,0x10,0x30,0x41,
        0x06,0x01,0x10,0x30,0x41,0x43,0x34,0x14,0x03,
        0x41,0x30,0x10,0x01,0x03,0x14,0x34,0x43,
        0x46,0x41,0x30,0x10,0x01,0x03,0x14,0x34,0x43,
        0x41,0x30,0x10,0x01,0x03,0x14,0x34,0x43,0x42,0x02,
        0x20,0x25,0x36,0x46,0x55,0x80,0x03,0x43,
        0x41,0x30,0x10,0x01,0x03,0x14,0x34,0x43,0x4f,0x3e,0x1e,0x0f,
        0x00,0x06,0x80,0x03,0x14,0x34,0x43,0x40,
        0x20,0x23,0x80,0x25,0x26,
        0x46,0x45,0x80,0x43,0x4f,0x3e,0x1e,0x0f,
        0x00,0x06,0x80,0x01,0x34,0x80,0x12,0x30,
        0x20,0x26,
        0x00,0x04,0x80,0x03,0x14,0x23,0x34,0x43,0x40,
        0x00,0x04,0x80,0x03,0x14,0x34,0x43,0x40,
        0x01,0x03,0x14,0x34,0x43,0x41,0x30,0x10,0x01,
        0x04,0x0e,0x80,0x01,0x10,0x30,0x41,0x43,0x34,0x14,0x03,
        0x41,0x30,0x10,0x01,0x03,0x14,0x34,0x43,0x80,0x44,0x4e,
        0x00,0x04,0x80,0x03,0x14,0x44,
        0x01,0x10,0x30,0x41,0x32,0x12,0x03,0x14,0x34,0x43,
        0x04,0x44,0x80,0x26,0x21,0x30,0x40,0x51,
        0x04,0x01,0x10,0x30,0x41,0x80,0x44,0x40,
        0x04,0x02,0x20,0x42,0x44,
        0x04,0x00,0x22,0x40,0x44,
        0x00,0x44,0x80,0x04,0x40,
        0x04,0x01,0x10,0x30,0x41,0x80,0x44,0x4f,0x3e,0x1e,0x0f,
        0x04,0x44,0x00,0x40,
        0x40,0x11,0x32,0x03,0x34,0x15,0x46,
        0x20,0x23,0x80,0x25,0x27,
        0x00,0x31,0x12,0x43,0x14,0x35,0x06,
        0x06,0x27,0x46,0x67,
        0x07,0x77
    };

    /* index to stroke font */

    static int index[97]=
    {

        1,2,7,12,23,38,52,63,65,69,73,84,89,93,95,100,104,
        113,119,127,141,147,156,168,172,190,202,213,225,228,233,236,
        247,262,270,283,291,298,305,311,321,329,337,342,350,353,358,
        366,375,382,392,402,414,419,425,430,435,444,450,456,460,464,
        468,471,473,475,484,493,501,510,520,528,540,548,553,561,569,
        571,580,588,597,608,619,625,635,643,651,656,661,666,677,681,
        688,693,700,704,706
    };


    /* test range */

    if (ichar<32  || ichar >127) ichar = 128; /* set to dummy character */

    /* set initial values */

    draw=0;
    istart=index[ichar-32]-1;
    iend=index[ichar-31]-2;

    /* loop round strokes */

    for (istr=istart;istr<=iend;istr++)
    {
        istrok=int(stroke[istr]);

        /* test for move */

        if (istrok==0x80)
        {
            draw=0;
            continue;
        }

        ix2=istrok/16;
        x2=float(ix2)*cwidth/7.0f+x;
        iy2=istrok%16;
        if (iy2>7) iy2=(iy2%8)-8;
        y2=float(iy2)*cheight/7.0f+y;

        /* draw vector if needed */

        if (draw!=0)
        {
            AddLine(x1, y1, m_z, x2, y2, m_z, matrix, translation);
        }

        /* set ix1,iy1 */

        x1=x2;
        y1=y2;
        draw=1;
    }
}

void StrokeFont::StrokeMarker(
                         MarkerCode code,       /* marker code */
                         float x,               /* x coordinate of centre of marker */
                         float y,               /* y coordinate ... */
                         float cwidth,          /* cwidth of character cell */
                         float cheight,         /* cheight of character cell */
                         const float *matrix, const float *translation)
{
    float x1, y1, x2, y2;
    switch (code)
    {
    case XShape:
        x1 = x - cwidth / 2;
        x2 = x + cwidth / 2;
        y1 = y - cheight / 2;
        y2 = y + cheight / 2;
        AddLine(x1, y1, m_z, x2, y2, m_z, matrix, translation);
        AddLine(x1, y2, m_z, x2, y1, m_z, matrix, translation);
        break;
    }
}


void StrokeFont::SetDisplayPosition(double x, double y, double z)
{
    m_displayPosition[0] = x;
    m_displayPosition[1] = y;
    m_displayPosition[2] = z;
}

void StrokeFont::SetDisplayRotation(const dMatrix3 R, bool fast)
{
    if (fast)
    {
        std::copy_n(R, dM3E__MAX, m_displayRotation);
    }
    else
    {
        dQuaternion q;
        dRtoQ (R, q);
        dNormalize4 (q);
        dQtoR (q, m_displayRotation);
    }
}

void StrokeFont::SetDisplayRotationFromQuaternion(const dQuaternion q, bool fast)
{
    if (fast == false)
    {
        dQuaternion qq;
        std::copy_n(q, dQUE__MAX, qq);
        dNormalize4 (qq);
        dQtoR(qq, m_displayRotation);
    }
    else
        dQtoR(q, m_displayRotation);
}

// move the object
// note this must be used before first draw call
void StrokeFont::Move(double x, double y, double z)
{
    float *ptr = line_buffer;
    float dx = float(x);
    float dy = float(y);
    float dz = float(z);
    for (size_t i = 0; i < n_lines; i++)
    {
        *ptr += dx; ptr++;
        *ptr += dy; ptr++;
        *ptr += dz; ptr++;
        ptr += 4;
        *ptr += dx; ptr++;
        *ptr += dy; ptr++;
        *ptr += dz; ptr++;
        ptr += 4;
    }
}

// scale the object
// note this must be used before first draw call
void StrokeFont::Scale(double x, double y, double z)
{
    float *ptr = line_buffer;
    float dx = float(x);
    float dy = float(y);
    float dz = float(z);
    for (size_t i = 0; i < n_lines; i++)
    {
        *ptr *= dx; ptr++;
        *ptr *= dy; ptr++;
        *ptr *= dz; ptr++;
        ptr += 4;
        *ptr *= dx; ptr++;
        *ptr *= dy; ptr++;
        *ptr *= dz; ptr++;
        ptr += 4;
    }
}

// draw a circle using line segments
void StrokeFont::AddCircle(float cx, float cy, float cz, float r, int num_segments)
{
    float theta = 2.0f * 3.1415926f / float(num_segments);
    float tangetial_factor = tanf(theta); //calculate the tangential factor
    float radial_factor = cosf(theta); //calculate the radial factor

    float x = r; //we start at angle = 0
    float y = 0;

    StartLine();
    AddPoint(x + cx, y + cy, cz);
    for(int ii = 0; ii < num_segments; ii++)
    {
        //calculate the tangential vector
        //remember, the radial vector is (x, y)
        //to get the tangential vector we flip those coordinates and negate one of them

        float tx = -y;
        float ty = x;

        //add the tangential vector

        x += tx * tangetial_factor;
        y += ty * tangetial_factor;

        //correct using the radial factor

        x *= radial_factor;
        y *= radial_factor;

        AddPoint(x + cx, y + cy, cz);
    }
}

void StrokeFont::AddArc(float cx, float cy, float cz, float r, float start_angle, float arc_angle, int num_segments)
{
    float theta = arc_angle / float(num_segments);//theta is now calculated from the arc angle instead
    float tangetial_factor = tanf(theta);
    float radial_factor = cosf(theta);

    float x = r * cosf(start_angle);//we now start at the start angle
    float y = r * sinf(start_angle);

    StartLine();
    AddPoint(x + cx, y + cy, cz);
    for(int ii = 0; ii < num_segments; ii++)
    {
        float tx = -y;
        float ty = x;

        x += tx * tangetial_factor;
        y += ty * tangetial_factor;

        x *= radial_factor;
        y *= radial_factor;

        AddPoint(x + cx, y + cy, cz);
    }
}

void StrokeFont::Draw()
{
    if (m_glWidget && n_lines)
    {
        if (m_BufferObjectsAllocated == false)
        {
            // data is already sensibly ordered so just setup our vertex buffer object.
            m_VBO.create();
            m_VBO.bind();
            m_VBO.allocate(line_buffer, int(n_lines * line_buffer_stride * 2 * sizeof(GLfloat)));
            m_BufferObjectsAllocated = true;
        }

        QMatrix4x4 translationRotation(
                    static_cast<float>(m_displayRotation[0]), static_cast<float>(m_displayRotation[1]), static_cast<float>(m_displayRotation[2]),  static_cast<float>(m_displayPosition[0]),
                    static_cast<float>(m_displayRotation[4]), static_cast<float>(m_displayRotation[5]), static_cast<float>(m_displayRotation[6]),  static_cast<float>(m_displayPosition[1]),
                    static_cast<float>(m_displayRotation[8]), static_cast<float>(m_displayRotation[9]), static_cast<float>(m_displayRotation[10]), static_cast<float>(m_displayPosition[2]),
                    0,                                        0,                                        0,                                         1);
        QMatrix4x4 scale(
                    static_cast<float>(m_displayScale[0]),  0,                                      0,                                      0,
                    0,                                      static_cast<float>(m_displayScale[1]),  0,                                      0,
                    0,                                      0,                                      static_cast<float>(m_displayScale[2]),  0,
                    0,                                      0,                                      0,                                      1);
        // ModelMatrix = Translation * Rotation * Scale
        QMatrix4x4 model = translationRotation * scale;

        // Store the vertex attribute bindings for the program.
        m_VBO.bind();
#if QT_VERSION >= 0x060000
        QOpenGLFunctions_3_3_Core *f = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(QOpenGLContext::currentContext());
#else
    QOpenGLFunctions_3_3_Core *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
#endif

        f->glEnableVertexAttribArray(0);
        f->glEnableVertexAttribArray(1);
        f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, line_buffer_stride * sizeof(GLfloat), nullptr);
        f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, line_buffer_stride * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));
        m_VBO.release();

        m_glWidget->fixedColourObjectShader()->bind();
        m_glWidget->fixedColourObjectShader()->setUniformValue("mvpMatrix", m_vpMatrix * model);

        f->glDrawArrays(GL_LINES, 0, GLsizei(n_lines * 2));

    }
}

QMatrix4x4 StrokeFont::vpMatrix() const
{
    return m_vpMatrix;
}

void StrokeFont::setVpMatrix(const QMatrix4x4 &vpMatrix)
{
    m_vpMatrix = vpMatrix;
}

SimulationWidget *StrokeFont::glWidget() const
{
    return m_glWidget;
}

void StrokeFont::setGlWidget(SimulationWidget *glWidget)
{
    m_glWidget = glWidget;
}

void StrokeFont::Debug()
{
    float *fp ;
    QMatrix4x4 translationRotation(
                static_cast<float>(m_displayRotation[0]), static_cast<float>(m_displayRotation[1]), static_cast<float>(m_displayRotation[2]),  static_cast<float>(m_displayPosition[0]),
                static_cast<float>(m_displayRotation[4]), static_cast<float>(m_displayRotation[5]), static_cast<float>(m_displayRotation[6]),  static_cast<float>(m_displayPosition[1]),
                static_cast<float>(m_displayRotation[8]), static_cast<float>(m_displayRotation[9]), static_cast<float>(m_displayRotation[10]), static_cast<float>(m_displayPosition[2]),
                0,                                        0,                                        0,                                         1);
    QMatrix4x4 scale(
                static_cast<float>(m_displayScale[0]),  0,                                      0,                                      0,
                0,                                      static_cast<float>(m_displayScale[1]),  0,                                      0,
                0,                                      0,                                      static_cast<float>(m_displayScale[2]),  0,
                0,                                      0,                                      0,                                      1);
    // ModelMatrix = Translation * Rotation * Scale
    QMatrix4x4 model = translationRotation * scale;
    QMatrix4x4 mvpMatrix = m_vpMatrix * model;
    qDebug("n_lines = %d\n", n_lines);
    for (size_t i = 0; i < n_lines; i++)
    {
        fp = line_buffer + i * line_buffer_stride * 2;
        QVector4D start(*fp, *(fp + 1), *(fp + 2), 1);
        QVector4D finish(*(fp + 7), *(fp + 8), *(fp + 9), 1);
        QVector4D start2 = mvpMatrix * start;
        QVector4D finish2 = mvpMatrix * finish;
        qDebug("%d (%f,%f,%f) to (%f,%f,%f) screen (%f,%f,%f) to (%f,%f,%f)\n", i,
               double(start.x()), double(start.y()), double(start.z()), double(finish.x()), double(finish.y()), double(finish.z()),
               double(start2.x()), double(start2.y()), double(start2.z()), double(finish2.x()), double(finish2.y()), double(finish2.z()));
    }
}

