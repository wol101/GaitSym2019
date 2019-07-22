// Handy geometry classes from Physics for Game Developers
// Altered to produce both double and single precision version

#ifndef PGDMATH
#define PGDMATH

#include <cmath>
#include <cfloat>


#define MYMATH_ABS(a) ((a) >= 0 ? (a) : -(a))
#define MYMATH_CLAMP(value, low, high) (((value)<(low))?(low):(((value)>(high))?(high):(value)))

#ifndef M_PI
#define M_PI       3.14159265358979323846264338327950288
#endif

// wis  - namespace to avoid naming problems
namespace pgd
{

    //------------------------------------------------------------------------//
    // Misc. Constants
    //------------------------------------------------------------------------//

    double   const   pi  = M_PI;
    double const epsilon = DBL_EPSILON; // 1 + this value is detectable
    double const minPositive = DBL_MIN; // the minimum positive number
    double const maxPositive = DBL_MAX; // the maximum positive

    //------------------------------------------------------------------------//
    // Misc. Functions
    //------------------------------------------------------------------------//
    inline  double   DegreesToRadians(double deg);
    inline  double   RadiansToDegrees(double rad);

    inline  double   DegreesToRadians(double deg)
    {
        return deg * pi / 180.0;
    }

    inline  double   RadiansToDegrees(double rad)
    {
        return rad * 180.0 / pi;
    }

    //------------------------------------------------------------------------//
    // Vector Class and vector functions
    //------------------------------------------------------------------------//
    class Vector {
public:
        double x;
        double y;
        double z;

        Vector(void);
        Vector(double xi, double yi, double zi);

        void Set(double xi, double yi, double zi);
        void Set(const double *xyz);

        double Magnitude(void);
        double Magnitude2(void);
        void  Normalize(void);
        void  Reverse(void);

        double Dot(Vector v);
        Vector Cross(Vector v);

        Vector& operator+=(Vector u);   // vector addition
        Vector& operator-=(Vector u);   // vector subtraction
        Vector& operator*=(double s);    // scalar multiply
        Vector& operator/=(double s);    // scalar divide
        Vector& operator=(double *s);    // assign from POD array

        Vector operator-(void); // unary negate

        double *data(void);

    };

    inline  Vector operator+(Vector u, Vector v);
    inline  Vector operator-(Vector u, Vector v);
    inline  Vector operator^(Vector u, Vector v);
    inline  double operator*(Vector u, Vector v);
    inline  Vector operator*(double s, Vector u);
    inline  Vector operator*(Vector u, double s);
    inline  Vector operator/(Vector u, double s);
    inline  double TripleScalarProduct(Vector u, Vector v, Vector w);

    inline Vector::Vector(void)
    {
        x = 0;
        y = 0;
        z = 0;
    }

    inline Vector::Vector(double xi, double yi, double zi)
    {
        x = xi;
        y = yi;
        z = zi;
    }

    inline void Vector::Set(double xi, double yi, double zi)
    {
        x = xi;
        y = yi;
        z = zi;
    }

    inline void Vector::Set(const double *xyz)
    {
        x = xyz[0];
        y = xyz[1];
        z = xyz[2];
    }

    inline  double Vector::Magnitude(void)
    {
        return std::sqrt(x*x + y*y + z*z);
    }

    inline  double Vector::Magnitude2(void)
    {
        return (x*x + y*y + z*z);
    }

    inline  void  Vector::Normalize(void)
    {
        // wis - to cope with very small vectors (quite common) we need to divide by the largest magnitude element
        // to minimise rounding errors. This will make it less good with larger vectors but that's
        // much less common in this application

        double xx = MYMATH_ABS(x);
        double yy = MYMATH_ABS(y);
        double zz = MYMATH_ABS(z);
        double m;
        if (yy >= xx)
        {
            if (zz >= yy) m = zz;
            else m = yy;
        }
        else
        {
            if (zz >= xx) m = zz;
            else m = xx;
        }
        if (m <= minPositive) // too small, need to fix up
        {
            x = 1;
            y = 0;
            z = 0;
            return;
        }

        // divide by maximum element to get all numbers to a sensible size for squaring
        x /= m;
        y /= m;
        z /= m;

        // now do the standard normalisation calculation
        m = std::sqrt(x*x + y*y + z*z);
        x /= m;
        y /= m;
        z /= m;
    }

    inline  void  Vector::Reverse(void)
    {
        x = -x;
        y = -y;
        z = -z;
    }

    inline Vector& Vector::operator+=(Vector u)
    {
        x += u.x;
        y += u.y;
        z += u.z;
        return *this;
    }

    inline  Vector& Vector::operator-=(Vector u)
    {
        x -= u.x;
        y -= u.y;
        z -= u.z;
        return *this;
    }

    inline  Vector& Vector::operator*=(double s)
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    inline  Vector& Vector::operator/=(double s)
    {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    inline  Vector& Vector::operator=(double *s)
    {
        x = s[0];
        y = s[1];
        z = s[2];
        return *this;
    }

    inline  Vector Vector::operator-(void)
    {
        return Vector(-x, -y, -z);
    }

    inline double *Vector::data(void)
    {
        return &x;
    }


    inline  Vector operator+(Vector u, Vector v)
    {
        return Vector(u.x + v.x, u.y + v.y, u.z + v.z);
    }

    inline  Vector operator-(Vector u, Vector v)
    {
        return Vector(u.x - v.x, u.y - v.y, u.z - v.z);
    }

    // Vector cross product (u cross v)
    inline  Vector operator^(Vector u, Vector v)
    {
        return Vector(  u.y*v.z - u.z*v.y,
                        -u.x*v.z + u.z*v.x,
                        u.x*v.y - u.y*v.x );
    }

    inline  Vector Vector::Cross(Vector v)
    {
        return Vector(  y*v.z - z*v.y,
                        -x*v.z + z*v.x,
                        x*v.y - y*v.x );
    }

    // Vector dot product
    inline  double operator*(Vector u, Vector v)
    {
        return (u.x*v.x + u.y*v.y + u.z*v.z);
    }

    inline double Vector::Dot(Vector v)
    {
        return (x*v.x + y*v.y + z*v.z);
    }

    inline  Vector operator*(double s, Vector u)
    {
        return Vector(u.x*s, u.y*s, u.z*s);
    }

    inline  Vector operator*(Vector u, double s)
    {
        return Vector(u.x*s, u.y*s, u.z*s);
    }

    inline  Vector operator/(Vector u, double s)
    {
        return Vector(u.x/s, u.y/s, u.z/s);
    }

    // triple scalar product (u dot (v cross w))
    inline  double TripleScalarProduct(Vector u, Vector v, Vector w)
    {
        return double(   (u.x * (v.y*w.z - v.z*w.y)) +
                        (u.y * (-v.x*w.z + v.z*w.x)) +
                        (u.z * (v.x*w.y - v.y*w.x)) );
        //return u*(v^w);

    }

    //------------------------------------------------------------------------//
    // Quaternion Class and Quaternion functions
    //------------------------------------------------------------------------//

    class Quaternion {
public:
        double   n;  // number (scalar) part
        Vector  v;  // vector part: v.x, v.y, v.z

        Quaternion(void);
        Quaternion(double e0, double e1, double e2, double e3);

        void Set(double e0, double e1, double e2, double e3);
        void Set(const double *q_nxyz);

        double   Magnitude(void);
        Vector  GetVector(void);
        double   GetScalar(void);
        void Normalize();
        Quaternion  operator+=(Quaternion q);
        Quaternion  operator-=(Quaternion q);
        Quaternion operator*=(double s);
        Quaternion operator/=(double s);
        Quaternion  operator~(void) const { return Quaternion(n, -v.x, -v.y, -v.z);}
        Quaternion  operator-(void) const { return Quaternion(-n, -v.x, -v.y, -v.z);}

        double *data();
    };

    inline  Quaternion operator+(Quaternion q1, Quaternion q2);
    inline  Quaternion operator-(Quaternion q1, Quaternion q2);
    inline  Quaternion operator*(Quaternion q1, Quaternion q2);
    inline  Quaternion operator*(Quaternion q, double s);
    inline  Quaternion operator*(double s, Quaternion q);
    inline  Quaternion operator*(Quaternion q, Vector v);
    inline  Quaternion operator*(Vector v, Quaternion q);
    inline  Quaternion operator/(Quaternion q, double s);
    inline  double QGetAngle(Quaternion q);
    inline  Vector QGetAxis(Quaternion q);
    inline  Quaternion QRotate(Quaternion q1, Quaternion q2);
    inline  Vector  QVRotate(Quaternion q, Vector v);
    inline  Quaternion  MakeQFromEulerAngles(double x, double y, double z);
    inline  Vector  MakeEulerAnglesFromQ(Quaternion q);
    inline  Quaternion  MakeQFromAxis(double x, double y, double z, double angle);
    inline Quaternion FindRotation(Quaternion qa, Quaternion qb);
    inline double FindAngle(Quaternion qa, Quaternion qb);
    inline Vector FindAxis(Quaternion qa, Quaternion qb);
    inline Quaternion FindRotation(Vector v1, Vector v2);

    inline  Quaternion::Quaternion(void)
    {
        n = 0;
        v.x = 0;
        v.y = 0;
        v.z = 0;
    }


    inline  Quaternion::Quaternion(double e0, double e1, double e2, double e3)
    {
        n = e0;
        v.x = e1;
        v.y = e2;
        v.z = e3;
    }

    inline  void Quaternion::Set(double e0, double e1, double e2, double e3)
    {
        n = e0;
        v.x = e1;
        v.y = e2;
        v.z = e3;
    }

    inline  void Quaternion::Set(const double *q_nxyz)
    {
        n = q_nxyz[0];
        v.x = q_nxyz[1];
        v.y = q_nxyz[2];
        v.z = q_nxyz[3];
    }

    inline  double   Quaternion::Magnitude(void)
    {
        return std::sqrt(n*n + v.x*v.x + v.y*v.y + v.z*v.z);
    }

    inline  Vector  Quaternion::GetVector(void)
    {
        return Vector(v.x, v.y, v.z);
    }

    inline  double   Quaternion::GetScalar(void)
    {
        return n;
    }

    inline  Quaternion  Quaternion::operator+=(Quaternion q)
    {
        n += q.n;
        v.x += q.v.x;
        v.y += q.v.y;
        v.z += q.v.z;
        return *this;
    }

    inline  Quaternion  Quaternion::operator-=(Quaternion q)
    {
        n -= q.n;
        v.x -= q.v.x;
        v.y -= q.v.y;
        v.z -= q.v.z;
        return *this;
    }

    inline  Quaternion Quaternion::operator*=(double s)
    {
        n *= s;
        v.x *= s;
        v.y *= s;
        v.z *= s;
        return *this;
    }

    inline  Quaternion Quaternion::operator/=(double s)
    {
        n /= s;
        v.x /= s;
        v.y /= s;
        v.z /= s;
        return *this;
    }

    inline double *Quaternion::data()
    {
        return &n;
    }

    inline  Quaternion operator+(Quaternion q1, Quaternion q2)
    {
        return  Quaternion( q1.n + q2.n,
                            q1.v.x + q2.v.x,
                            q1.v.y + q2.v.y,
                            q1.v.z + q2.v.z);
    }

    inline  Quaternion operator-(Quaternion q1, Quaternion q2)
    {
        return  Quaternion( q1.n - q2.n,
                            q1.v.x - q2.v.x,
                            q1.v.y - q2.v.y,
                            q1.v.z - q2.v.z);
    }

    inline  Quaternion operator*(Quaternion q1, Quaternion q2)
    {
        return  Quaternion( q1.n*q2.n - q1.v.x*q2.v.x - q1.v.y*q2.v.y - q1.v.z*q2.v.z,
                            q1.n*q2.v.x + q1.v.x*q2.n + q1.v.y*q2.v.z - q1.v.z*q2.v.y,
                            q1.n*q2.v.y + q1.v.y*q2.n + q1.v.z*q2.v.x - q1.v.x*q2.v.z,
                            q1.n*q2.v.z + q1.v.z*q2.n + q1.v.x*q2.v.y - q1.v.y*q2.v.x);
    }

    inline  Quaternion operator*(Quaternion q, double s)
    {
        return  Quaternion(q.n*s, q.v.x*s, q.v.y*s, q.v.z*s);
    }

    inline  Quaternion operator*(double s, Quaternion q)
    {
        return  Quaternion(q.n*s, q.v.x*s, q.v.y*s, q.v.z*s);
    }

    inline  Quaternion operator*(Quaternion q, Vector v)
    {
        return  Quaternion( -(q.v.x*v.x + q.v.y*v.y + q.v.z*v.z),
                            q.n*v.x + q.v.y*v.z - q.v.z*v.y,
                            q.n*v.y + q.v.z*v.x - q.v.x*v.z,
                            q.n*v.z + q.v.x*v.y - q.v.y*v.x);
    }

    inline  Quaternion operator*(Vector v, Quaternion q)
    {
        return  Quaternion( -(q.v.x*v.x + q.v.y*v.y + q.v.z*v.z),
                            q.n*v.x + q.v.z*v.y - q.v.y*v.z,
                            q.n*v.y + q.v.x*v.z - q.v.z*v.x,
                            q.n*v.z + q.v.y*v.x - q.v.x*v.y);
    }

    inline  Quaternion operator/(Quaternion q, double s)
    {
        return  Quaternion(q.n/s, q.v.x/s, q.v.y/s, q.v.z/s);
    }


    inline  double QGetAngle(Quaternion q)
    {
        if (q.n <= -1) return 0; // 2 * pi
        if (q.n >= 1) return 0; // 2 * 0
        return  (2*std::acos(q.n));
    }

    inline  Vector QGetAxis(Quaternion q)
    {
        Vector v = q.GetVector();
        v.Normalize();
        return v;
    }

    inline  Quaternion QRotate(Quaternion q1, Quaternion q2)
    {
        return  q1*q2*(~q1);
    }

    inline  Vector  QVRotate(Quaternion q, Vector v)
    {
#ifdef EASY_TO_READ
        Quaternion t;


        t = q*v*(~q);

        return  t.GetVector();
#else
    // optimisation based on OpenSG code
    double rx,ry,rz;
    double QwQx, QwQy, QwQz, QxQy, QxQz, QyQz;

    QwQx = q.n * q.v.x;
    QwQy = q.n * q.v.y;
    QwQz = q.n * q.v.z;
    QxQy = q.v.x * q.v.y;
    QxQz = q.v.x * q.v.z;
    QyQz = q.v.y * q.v.z;

    rx = 2* (v.y * (-QwQz + QxQy) + v.z *( QwQy + QxQz));
    ry = 2* (v.x * ( QwQz + QxQy) + v.z *(-QwQx + QyQz));
    rz = 2* (v.x * (-QwQy + QxQz) + v.y *( QwQx + QyQz));

    double QwQw, QxQx, QyQy, QzQz;

    QwQw = q.n * q.n;
    QxQx = q.v.x * q.v.x;
    QyQy = q.v.y * q.v.y;
    QzQz = q.v.z * q.v.z;

    rx+= v.x * (QwQw + QxQx - QyQy - QzQz);
    ry+= v.y * (QwQw - QxQx + QyQy - QzQz);
    rz+= v.z * (QwQw - QxQx - QyQy + QzQz);

    return Vector(rx,ry,rz);

#endif
    }

    // these are intrinsic Euler XYZ angles (or fixed axis ZYX)
    inline  Quaternion  MakeQFromEulerAngles(double x, double y, double z)
    {
        Quaternion  q;
        double   roll = DegreesToRadians(x);
        double   pitch = DegreesToRadians(y);
        double   yaw = DegreesToRadians(z);

        double   cyaw, cpitch, croll, syaw, spitch, sroll;
        double   cyawcpitch, syawspitch, cyawspitch, syawcpitch;

        cyaw = std::cos(0.5 * yaw);
        cpitch = std::cos(0.5 * pitch);
        croll = std::cos(0.5 * roll);
        syaw = std::sin(0.5 * yaw);
        spitch = std::sin(0.5 * pitch);
        sroll = std::sin(0.5 * roll);

        cyawcpitch = cyaw*cpitch;
        syawspitch = syaw*spitch;
        cyawspitch = cyaw*spitch;
        syawcpitch = syaw*cpitch;

        q.n = (cyawcpitch * croll + syawspitch * sroll);
        q.v.x = (cyawcpitch * sroll - syawspitch * croll);
        q.v.y = (cyawspitch * croll + syawcpitch * sroll);
        q.v.z = (syawcpitch * croll - cyawspitch * sroll);

        return q;
    }

    // these are intrinsic Euler XYZ angles (or fixed axis ZYX)
    inline  Vector  MakeEulerAnglesFromQ(Quaternion q)
    {
        double   r11, r21, r31, r32, r33, r12, r13;
        double   q00, q11, q22, q33;
        double   tmp;
        Vector  u;

        q00 = q.n * q.n;
        q11 = q.v.x * q.v.x;
        q22 = q.v.y * q.v.y;
        q33 = q.v.z * q.v.z;

        r11 = q00 + q11 - q22 - q33;
        r21 = 2 * (q.v.x*q.v.y + q.n*q.v.z);
        r31 = 2 * (q.v.x*q.v.z - q.n*q.v.y);
        r32 = 2 * (q.v.y*q.v.z + q.n*q.v.x);
        r33 = q00 - q11 - q22 + q33;

        tmp = MYMATH_ABS(r31);
        if(tmp > (1 - epsilon))
        {
            r12 = 2 * (q.v.x*q.v.y - q.n*q.v.z);
            r13 = 2 * (q.v.x*q.v.z + q.n*q.v.y);

            u.x = RadiansToDegrees(0.0); //roll
            u.y = RadiansToDegrees((-(pi/2) * r31/tmp)); // pitch
            u.z = RadiansToDegrees(std::atan2(-r12, -r31*r13)); // yaw
            return u;
        }

        u.x = RadiansToDegrees(std::atan2(r32, r33)); // roll
        u.y = RadiansToDegrees(std::asin(-r31));      // pitch
        u.z = RadiansToDegrees(std::atan2(r21, r11)); // yaw
        return u;


    }

    // wis  - new routine to make a Quaternion from an axis and a rotation angle in radians
    inline  Quaternion  MakeQFromAxis(double x, double y, double z, double angle)
    {
        Quaternion  q;

        Vector v(x, y, z);
        v.Normalize();

        while (angle > pi) angle -= (2 * pi);
        while (angle < -pi) angle += (2 * pi);

        double sin_a = std::sin( angle / 2 );
        double cos_a = std::cos( angle / 2 );

        q.v.x    = v.x * sin_a;
        q.v.y    = v.y * sin_a;
        q.v.z    = v.z * sin_a;
        q.n    = cos_a;

        return q;
    }

    // wis - new routines to calculate the quaternion which rotates qa to qb
    //
    // based on http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/index.htm
    // if q is the quaternion which rotates from qa to qb then:
    // qb = qa * q
    //
    // multiplying both sides by conj(qa) gives:
    // q = conj(qa) * qb
    //
    // The real part of a multiplication is:
    // real((qa.w + i qa.x + j qa.y + k qa.z)*(qb.w + i qb.x + j qb.y + k qb.z)) = qa.w * qb.w - qa.x*qb.x - qa.y*qb.y- qa.z*qb.z
    //
    // So using the conjugate of qa gives:
    // real((qa.w - i qa.x - j qa.y - k qa.z)*(qb.w + i qb.x + j qb.y + k qb.z)) = qa.w*qb.w + qa.x*qb.x + qa.y*qb.y+ qa.z*qb.z
    //
    // real(q) = std::cos(t/2)
    //
    // Therefore
    // std::cos(theta/2) = qa.w*qb.w + qa.x*qb.x + qa.y*qb.y+ qa.z*qb.z
    inline Quaternion FindRotation(Quaternion qa, Quaternion qb)
    {
        return ((~qa) * qb);
    }
    inline double FindAngle(Quaternion qa, Quaternion qb)
    {
        double v = qa.n*qb.n + qa.v.x*qb.v.x + qa.v.y*qb.v.y+ qa.v.z*qb.v.z;
        if (v <= -1) return 0; // 2 * pi
        if (v >= 1) return 0; // 2 * 0
        double angle = 2 * std::acos(v);
        if (angle < -pi) angle += (2 * pi);
        else if (angle > pi) angle -= (2 * pi);
        return angle;
    }
    inline Vector FindAxis(Quaternion qa, Quaternion qb)
    {
        return QGetAxis((~qa) * qb);
    }

    // this is useful when reading in quaternions since there is inevitable precision loss
    // which means the quaternion will be slightly denormal
    inline void Quaternion::Normalize()
    {
        double l = std::sqrt(n*n + v.x*v.x + v.y*v.y + v.z*v.z);
        n /= l;
        v.x /= l;
        v.y /= l;
        v.z /= l;
    }

    // this routine returns the quaternion that rotates v1 to v2 via the shortest path
    inline Quaternion FindRotation(Vector v1, Vector v2)
    {
        Quaternion q;
        q.n = std::sqrt((v1.Magnitude2()) * (v2.Magnitude2())) + (v1 * v2);
        if (q.n < epsilon) // this only occurs if a 180 degree rotation is needed
        {
            Vector perp; // this is a perpendicular vector (v1 dot perp = 0)
            if (fabs(v1.z) > epsilon) perp = Vector(0, -v1.z, v1.y);
            else perp = Vector(-v1.y, v1.x, 0);
            q = MakeQFromAxis(perp.x, perp.y, perp.z, pi);
        }
        else
        {
            q.v = v1 ^ v2;
            q.Normalize();
        }
        return q;
    }


    //------------------------------------------------------------------------//
    // Matrix Class and matrix functions
    //------------------------------------------------------------------------//

    class Matrix3x3 {
public:
        // elements eij: i -> row, j -> column
        double   e11, e12, e13, e21, e22, e23, e31, e32, e33;

        Matrix3x3(void);
        Matrix3x3(  double r1c1, double r1c2, double r1c3,
                    double r2c1, double r2c2, double r2c3,
                    double r3c1, double r3c2, double r3c3 );
        Matrix3x3(Quaternion q);

        void Set(   double r1c1, double r1c2, double r1c3,
                    double r2c1, double r2c2, double r2c3,
                    double r3c1, double r3c2, double r3c3 );
        void Set(const double *mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3);

        double   det(void);
        Matrix3x3   Transpose(void);
        Matrix3x3   Inverse(void);

        Matrix3x3& operator+=(Matrix3x3 m);
        Matrix3x3& operator-=(Matrix3x3 m);
        Matrix3x3& operator*=(double s);
        Matrix3x3& operator/=(double s);

        double *data();
    };

    inline  Matrix3x3 operator+(Matrix3x3 m1, Matrix3x3 m2);
    inline  Matrix3x3 operator-(Matrix3x3 m1, Matrix3x3 m2);
    inline  Matrix3x3 operator/(Matrix3x3 m, double s);
    inline  Matrix3x3 operator*(Matrix3x3 m1, Matrix3x3 m2);
    inline  Matrix3x3 operator*(Matrix3x3 m, double s);
    inline  Matrix3x3 operator*(double s, Matrix3x3 m);
    inline  Vector operator*(Matrix3x3 m, Vector u);
    inline  Vector operator*(Vector u, Matrix3x3 m);

    inline  Matrix3x3::Matrix3x3(void)
    {
        e11 = 0;
        e12 = 0;
        e13 = 0;
        e21 = 0;
        e22 = 0;
        e23 = 0;
        e31 = 0;
        e32 = 0;
        e33 = 0;
    }

    inline  Matrix3x3::Matrix3x3(   double r1c1, double r1c2, double r1c3,
                                    double r2c1, double r2c2, double r2c3,
                                    double r3c1, double r3c2, double r3c3 )
    {
        e11 = r1c1;
        e12 = r1c2;
        e13 = r1c3;
        e21 = r2c1;
        e22 = r2c2;
        e23 = r2c3;
        e31 = r3c1;
        e32 = r3c2;
        e33 = r3c3;
    }

    inline  void Matrix3x3::Set( double r1c1, double r1c2, double r1c3,
                                 double r2c1, double r2c2, double r2c3,
                                 double r3c1, double r3c2, double r3c3 )
    {
        e11 = r1c1;
        e12 = r1c2;
        e13 = r1c3;
        e21 = r2c1;
        e22 = r2c2;
        e23 = r2c3;
        e31 = r3c1;
        e32 = r3c2;
        e33 = r3c3;
    }

    inline void Matrix3x3::Set(const double *mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3)
    {
        e11 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[0];
        e12 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[1];
        e13 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[2];
        e21 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[3];
        e22 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[4];
        e23 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[5];
        e31 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[6];
        e32 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[7];
        e33 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[8];
    }

    inline double *Matrix3x3::data()
    {
        return &e11;
    }

    // initialise a matrix from a quaternion
    // based on code from
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
    // untested!
    inline  Matrix3x3::Matrix3x3(Quaternion q)
    {
        double sqw = q.n*q.n;
        double sqx = q.v.x*q.v.x;
        double sqy = q.v.y*q.v.y;
        double sqz = q.v.z*q.v.z;

        // invs (inverse square length) is only required if quaternion is not already normalised
        double invs = 1 / (sqx + sqy + sqz + sqw);
        e11 = ( sqx - sqy - sqz + sqw)*invs ; // since sqw + sqx + sqy + sqz =1/invs*invs
        e22 = (-sqx + sqy - sqz + sqw)*invs ;
        e33 = (-sqx - sqy + sqz + sqw)*invs ;

        double tmp1 = q.v.x*q.v.y;
        double tmp2 = q.v.z*q.n;
        e21 = 2.0 * (tmp1 + tmp2)*invs ;
        e12 = 2.0 * (tmp1 - tmp2)*invs ;

        tmp1 = q.v.x*q.v.z;
        tmp2 = q.v.y*q.n;
        e31 = 2.0 * (tmp1 - tmp2)*invs ;
        e13 = 2.0 * (tmp1 + tmp2)*invs ;
        tmp1 = q.v.y*q.v.z;
        tmp2 = q.v.x*q.n;
        e32 = 2.0 * (tmp1 + tmp2)*invs ;
        e23 = 2.0 * (tmp1 - tmp2)*invs ;
    }

    inline  double   Matrix3x3::det(void)
    {
        return  e11*e22*e33 -
        e11*e32*e23 +
        e21*e32*e13 -
        e21*e12*e33 +
        e31*e12*e23 -
        e31*e22*e13;
    }

    inline  Matrix3x3   Matrix3x3::Transpose(void)
    {
        return Matrix3x3(e11,e21,e31,e12,e22,e32,e13,e23,e33);
    }

    inline  Matrix3x3   Matrix3x3::Inverse(void)
    {
        double   d = e11*e22*e33 -
        e11*e32*e23 +
        e21*e32*e13 -
        e21*e12*e33 +
        e31*e12*e23 -
        e31*e22*e13;

        if (std::fabs(d) < minPositive) d = 1;

        return  Matrix3x3(  (e22*e33-e23*e32)/d,
                            -(e12*e33-e13*e32)/d,
                            (e12*e23-e13*e22)/d,
                            -(e21*e33-e23*e31)/d,
                            (e11*e33-e13*e31)/d,
                            -(e11*e23-e13*e21)/d,
                            (e21*e32-e22*e31)/d,
                            -(e11*e32-e12*e31)/d,
                            (e11*e22-e12*e21)/d );
    }

    inline  Matrix3x3& Matrix3x3::operator+=(Matrix3x3 m)
    {
        e11 += m.e11;
        e12 += m.e12;
        e13 += m.e13;
        e21 += m.e21;
        e22 += m.e22;
        e23 += m.e23;
        e31 += m.e31;
        e32 += m.e32;
        e33 += m.e33;
        return *this;
    }

    inline  Matrix3x3& Matrix3x3::operator-=(Matrix3x3 m)
    {
        e11 -= m.e11;
        e12 -= m.e12;
        e13 -= m.e13;
        e21 -= m.e21;
        e22 -= m.e22;
        e23 -= m.e23;
        e31 -= m.e31;
        e32 -= m.e32;
        e33 -= m.e33;
        return *this;
    }

    inline  Matrix3x3& Matrix3x3::operator*=(double s)
    {
        e11 *= s;
        e12 *= s;
        e13 *= s;
        e21 *= s;
        e22 *= s;
        e23 *= s;
        e31 *= s;
        e32 *= s;
        e33 *= s;
        return *this;
    }

    inline  Matrix3x3& Matrix3x3::operator/=(double s)
    {
        e11 /= s;
        e12 /= s;
        e13 /= s;
        e21 /= s;
        e22 /= s;
        e23 /= s;
        e31 /= s;
        e32 /= s;
        e33 /= s;
        return *this;
    }

    inline  Matrix3x3 operator+(Matrix3x3 m1, Matrix3x3 m2)
    {
        return  Matrix3x3(  m1.e11+m2.e11,
                            m1.e12+m2.e12,
                            m1.e13+m2.e13,
                            m1.e21+m2.e21,
                            m1.e22+m2.e22,
                            m1.e23+m2.e23,
                            m1.e31+m2.e31,
                            m1.e32+m2.e32,
                            m1.e33+m2.e33);
    }

    inline  Matrix3x3 operator-(Matrix3x3 m1, Matrix3x3 m2)
    {
        return  Matrix3x3(  m1.e11-m2.e11,
                            m1.e12-m2.e12,
                            m1.e13-m2.e13,
                            m1.e21-m2.e21,
                            m1.e22-m2.e22,
                            m1.e23-m2.e23,
                            m1.e31-m2.e31,
                            m1.e32-m2.e32,
                            m1.e33-m2.e33);
    }

    inline  Matrix3x3 operator/(Matrix3x3 m, double s)
    {
        return  Matrix3x3(  m.e11/s,
                            m.e12/s,
                            m.e13/s,
                            m.e21/s,
                            m.e22/s,
                            m.e23/s,
                            m.e31/s,
                            m.e32/s,
                            m.e33/s);
    }

    inline  Matrix3x3 operator*(Matrix3x3 m1, Matrix3x3 m2)
    {
        return Matrix3x3(   m1.e11*m2.e11 + m1.e12*m2.e21 + m1.e13*m2.e31,
                            m1.e11*m2.e12 + m1.e12*m2.e22 + m1.e13*m2.e32,
                            m1.e11*m2.e13 + m1.e12*m2.e23 + m1.e13*m2.e33,
                            m1.e21*m2.e11 + m1.e22*m2.e21 + m1.e23*m2.e31,
                            m1.e21*m2.e12 + m1.e22*m2.e22 + m1.e23*m2.e32,
                            m1.e21*m2.e13 + m1.e22*m2.e23 + m1.e23*m2.e33,
                            m1.e31*m2.e11 + m1.e32*m2.e21 + m1.e33*m2.e31,
                            m1.e31*m2.e12 + m1.e32*m2.e22 + m1.e33*m2.e32,
                            m1.e31*m2.e13 + m1.e32*m2.e23 + m1.e33*m2.e33 );
    }

    inline  Matrix3x3 operator*(Matrix3x3 m, double s)
    {
        return  Matrix3x3(  m.e11*s,
                            m.e12*s,
                            m.e13*s,
                            m.e21*s,
                            m.e22*s,
                            m.e23*s,
                            m.e31*s,
                            m.e32*s,
                            m.e33*s);
    }

    inline  Matrix3x3 operator*(double s, Matrix3x3 m)
    {
        return  Matrix3x3(  m.e11*s,
                            m.e12*s,
                            m.e13*s,
                            m.e21*s,
                            m.e22*s,
                            m.e23*s,
                            m.e31*s,
                            m.e32*s,
                            m.e33*s);
    }

    inline  Vector operator*(Matrix3x3 m, Vector u)
    {
        return Vector(  m.e11*u.x + m.e12*u.y + m.e13*u.z,
                        m.e21*u.x + m.e22*u.y + m.e23*u.z,
                        m.e31*u.x + m.e32*u.y + m.e33*u.z);
    }

    inline  Vector operator*(Vector u, Matrix3x3 m)
    {
        return Vector(  u.x*m.e11 + u.y*m.e21 + u.z*m.e31,
                        u.x*m.e12 + u.y*m.e22 + u.z*m.e32,
                        u.x*m.e13 + u.y*m.e23 + u.z*m.e33);
    }

    inline Matrix3x3 MakeMFromQ(Quaternion q)
    {
        Matrix3x3 m;

        double qq1 = 2*q.v.x*q.v.x;
        double qq2 = 2*q.v.y*q.v.y;
        double qq3 = 2*q.v.z*q.v.z;
        m.e11 = 1 - qq2 - qq3;
        m.e12 = 2*(q.v.x*q.v.y - q.n*q.v.z);
        m.e13 = 2*(q.v.x*q.v.z + q.n*q.v.y);
        m.e21 = 2*(q.v.x*q.v.y + q.n*q.v.z);
        m.e22 = 1 - qq1 - qq3;
        m.e23 = 2*(q.v.y*q.v.z - q.n*q.v.x);
        m.e31 = 2*(q.v.x*q.v.z - q.n*q.v.y);
        m.e32 = 2*(q.v.y*q.v.z + q.n*q.v.x);
        m.e33 = 1 - qq1 - qq2;
        return m;
    }

    inline Quaternion MakeQfromM (Matrix3x3 R)
    {
        Quaternion q;
        double tr,s;
        tr = R.e11 + R.e22 + R.e33;
        if (tr >= 0)
        {
            s = std::sqrt (tr + 1);
            q.n = 0.5 * s;
            s = 0.5 * (1.0/s);
            q.v.x = (R.e32 - R.e23) * s;
            q.v.y = (R.e13 - R.e31) * s;
            q.v.z = (R.e21 - R.e12) * s;
        }
        else
        {
            // find the largest diagonal element and jump to the appropriate case
            if (R.e22 > R.e11)
            {
                if (R.e33 > R.e22) goto case_2;
                goto case_1;
            }
            if (R.e33 > R.e11) goto case_2;
            goto case_0;

case_0:
            s = std::sqrt((R.e11 - (R.e22 + R.e33)) + 1);
            q.v.x = 0.5 * s;
            s = 0.5 * (1.0/s);
            q.v.y = (R.e12 + R.e21) * s;
            q.v.z = (R.e31 + R.e13) * s;
            q.n = (R.e32 - R.e23) * s;
            return q;

case_1:
            s = std::sqrt((R.e22 - (R.e33 + R.e11)) + 1);
            q.v.y = 0.5 * s;
            s = 0.5 * (1.0/s);
            q.v.z = (R.e23 + R.e32) * s;
            q.v.x = (R.e12 + R.e21) * s;
            q.n = (R.e13 - R.e31) * s;
            return q;

case_2:
            s = std::sqrt((R.e33 - (R.e11 + R.e22)) + 1);
            q.v.z = 0.5 * s;
            s = 0.5 * (1.0/s);
            q.v.x = (R.e31 + R.e13) * s;
            q.v.y = (R.e23 + R.e32) * s;
            q.n = (R.e21 - R.e12) * s;
            return q;
        }
        return q;
    }

    // find the closest point to point P on a line defined as origin B and direction M
    // using formulae from www.geometrictools.com
    inline Vector ClosestPoint(Vector P, Vector B, Vector M)
    {
        double t0 = (M * (P - B)) / (M * M);
        Vector Q = B + (t0 * M);
        return Q;
    }

    //  generates a quaternion between two given quaternions in proportion to the variable t
    // if t=0 then qm=qa, if t=1 then qm=qb, if t is between them then qm will interpolate between them
    inline Quaternion slerp(Quaternion v0, Quaternion v1, double t, bool normalise = true)
    {
        if (normalise)
        {
            // Only unit quaternions are valid rotations.
            // Normalize to avoid undefined behavior.
            v0.Normalize();
            v1.Normalize();
        }

        // Compute the cosine of the angle between the two vectors.
        double dot = v0.v * v1.v; // dot product

        const double DOT_THRESHOLD = 0.9995;
        if (dot > DOT_THRESHOLD)
        {
            // If the inputs are too close for comfort, linearly interpolate
            // and normalize the result.

            Quaternion result = v0 + t * (v1 - v0);
            result.Normalize();
            return result;
        }

        // If the dot product is negative, the quaternions
        // have opposite handed-ness and slerp won't take
        // the shorter path. Fix by reversing one quaternion.
        if (dot < 0.0)
        {
            v1 = -v1;
            dot = -dot;
        }

        MYMATH_CLAMP(dot, -1, 1);    // Robustness: Stay within domain of std::acos()
        double theta_0 = std::acos(dot);  // theta_0 = angle between input vectors
        double theta = theta_0*t;    // theta = angle between v0 and result

        Quaternion v2 = v1 - v0*dot;
        v2.Normalize();              // { v0, v2 } is now an orthonormal basis

        return v0*std::cos(theta) + v2*std::sin(theta);
    }

}

namespace pgd_float
{

    //------------------------------------------------------------------------//
    // Misc. Constants
    //------------------------------------------------------------------------//

    float   const   pi  = float(M_PI);

    float const epsilon = FLT_EPSILON; // 1 + this value is detectable
    float const minPositive = FLT_MIN; // the minimum positive number
    float const maxPositive = FLT_MAX; // the maximum positive

    //------------------------------------------------------------------------//
    // Misc. Functions
    //------------------------------------------------------------------------//
    inline  float   DegreesToRadians(float deg);
    inline  float   RadiansToDegrees(float rad);

    inline  float   DegreesToRadians(float deg)
    {
        return deg * pi / 180.0f;
    }

    inline  float   RadiansToDegrees(float rad)
    {
        return rad * 180.0f / pi;
    }

    //------------------------------------------------------------------------//
    // Vector Class and vector functions
    //------------------------------------------------------------------------//
    class Vector {
public:
        float x;
        float y;
        float z;

        Vector(void);
        Vector(float xi, float yi, float zi);

        void Set(float xi, float yi, float zi);
        void Set(const float *xyz);

        float Magnitude(void);
        float Magnitude2(void);
        void  Normalize(void);
        void  Reverse(void);

        float Dot(Vector v);
        Vector Cross(Vector v);

        Vector& operator+=(Vector u);   // vector addition
        Vector& operator-=(Vector u);   // vector subtraction
        Vector& operator*=(float s);    // scalar multiply
        Vector& operator/=(float s);    // scalar divide
        Vector& operator=(float *s);    // assign from POD array

        Vector operator-(void); // unary negate

        float *data(void);
    };

    inline  Vector operator+(Vector u, Vector v);
    inline  Vector operator-(Vector u, Vector v);
    inline  Vector operator^(Vector u, Vector v);
    inline  float operator*(Vector u, Vector v);
    inline  Vector operator*(float s, Vector u);
    inline  Vector operator*(Vector u, float s);
    inline  Vector operator/(Vector u, float s);
    inline  float TripleScalarProduct(Vector u, Vector v, Vector w);

    inline Vector::Vector(void)
    {
        x = 0;
        y = 0;
        z = 0;
    }

    inline Vector::Vector(float xi, float yi, float zi)
    {
        x = xi;
        y = yi;
        z = zi;
    }

    inline void Vector::Set(float xi, float yi, float zi)
    {
        x = xi;
        y = yi;
        z = zi;
    }

    inline void Vector::Set(const float *xyz)
    {
        x = xyz[0];
        y = xyz[1];
        z = xyz[2];
    }

    inline  float Vector::Magnitude(void)
    {
        return std::sqrt(x*x + y*y + z*z);
    }

    inline  float Vector::Magnitude2(void)
    {
        return (x*x + y*y + z*z);
    }

    inline  void  Vector::Normalize(void)
    {
        // wis - to cope with very small vectors (quite common) we need to divide by the largest magnitude element
        // to minimise rounding errors. This will make it less good with larger vectors but that's
        // much less common in this application

        float xx = MYMATH_ABS(x);
        float yy = MYMATH_ABS(y);
        float zz = MYMATH_ABS(z);
        float m;
        if (yy >= xx)
        {
            if (zz >= yy) m = zz;
            else m = yy;
        }
        else
        {
            if (zz >= xx) m = zz;
            else m = xx;
        }
        if (m <= minPositive) // too small, need to fix up
        {
            x = 1;
            y = 0;
            z = 0;
            return;
        }

        // divide by maximum element to get all numbers to a sensible size for squaring
        x /= m;
        y /= m;
        z /= m;

        // now do the standard normalisation calculation
        m = std::sqrt(x*x + y*y + z*z);
        x /= m;
        y /= m;
        z /= m;
    }

    inline  void  Vector::Reverse(void)
    {
        x = -x;
        y = -y;
        z = -z;
    }

    inline Vector& Vector::operator+=(Vector u)
    {
        x += u.x;
        y += u.y;
        z += u.z;
        return *this;
    }

    inline  Vector& Vector::operator-=(Vector u)
    {
        x -= u.x;
        y -= u.y;
        z -= u.z;
        return *this;
    }

    inline  Vector& Vector::operator*=(float s)
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    inline  Vector& Vector::operator/=(float s)
    {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    inline  Vector& Vector::operator=(float *s)
    {
        x = s[0];
        y = s[1];
        z = s[2];
        return *this;
    }

    inline  Vector Vector::operator-(void)
    {
        return Vector(-x, -y, -z);
    }

    inline float *Vector::data(void)
    {
        return &x;
    }

    inline  Vector operator+(Vector u, Vector v)
    {
        return Vector(u.x + v.x, u.y + v.y, u.z + v.z);
    }

    inline  Vector operator-(Vector u, Vector v)
    {
        return Vector(u.x - v.x, u.y - v.y, u.z - v.z);
    }

    // Vector cross product (u cross v)
    inline  Vector operator^(Vector u, Vector v)
    {
        return Vector(  u.y*v.z - u.z*v.y,
                        -u.x*v.z + u.z*v.x,
                        u.x*v.y - u.y*v.x );
    }

    inline  Vector Vector::Cross(Vector v)
    {
        return Vector(  y*v.z - z*v.y,
                        -x*v.z + z*v.x,
                        x*v.y - y*v.x );
    }

    // Vector dot product
    inline  float operator*(Vector u, Vector v)
    {
        return (u.x*v.x + u.y*v.y + u.z*v.z);
    }

    inline float Vector::Dot(Vector v)
    {
        return (x*v.x + y*v.y + z*v.z);
    }


    inline  Vector operator*(float s, Vector u)
    {
        return Vector(u.x*s, u.y*s, u.z*s);
    }

    inline  Vector operator*(Vector u, float s)
    {
        return Vector(u.x*s, u.y*s, u.z*s);
    }

    inline  Vector operator/(Vector u, float s)
    {
        return Vector(u.x/s, u.y/s, u.z/s);
    }

    // triple scalar product (u dot (v cross w))
    inline  float TripleScalarProduct(Vector u, Vector v, Vector w)
    {
        return float(   (u.x * (v.y*w.z - v.z*w.y)) +
                        (u.y * (-v.x*w.z + v.z*w.x)) +
                        (u.z * (v.x*w.y - v.y*w.x)) );
        //return u*(v^w);

    }

    //------------------------------------------------------------------------//
    // Quaternion Class and Quaternion functions
    //------------------------------------------------------------------------//

    class Quaternion {
public:
        float   n;  // number (scalar) part
        Vector  v;  // vector part: v.x, v.y, v.z

        Quaternion(void);
        Quaternion(float e0, float e1, float e2, float e3);

        void Set(float e0, float e1, float e2, float e3);
        void Set(const float *q_nxyz);

        float   Magnitude(void);
        Vector  GetVector(void);
        float   GetScalar(void);
        void Normalize();
        Quaternion  operator+=(Quaternion q);
        Quaternion  operator-=(Quaternion q);
        Quaternion operator*=(float s);
        Quaternion operator/=(float s);
        Quaternion  operator~(void) const { return Quaternion(n, -v.x, -v.y, -v.z);}
        Quaternion  operator-(void) const { return Quaternion(-n, -v.x, -v.y, -v.z);}

        float *data(); // return a float list

    };

    inline  Quaternion operator+(Quaternion q1, Quaternion q2);
    inline  Quaternion operator-(Quaternion q1, Quaternion q2);
    inline  Quaternion operator*(Quaternion q1, Quaternion q2);
    inline  Quaternion operator*(Quaternion q, float s);
    inline  Quaternion operator*(float s, Quaternion q);
    inline  Quaternion operator*(Quaternion q, Vector v);
    inline  Quaternion operator*(Vector v, Quaternion q);
    inline  Quaternion operator/(Quaternion q, float s);
    inline  float QGetAngle(Quaternion q);
    inline  Vector QGetAxis(Quaternion q);
    inline  Quaternion QRotate(Quaternion q1, Quaternion q2);
    inline  Vector  QVRotate(Quaternion q, Vector v);
    inline  Quaternion  MakeQFromEulerAngles(float x, float y, float z);
    inline  Vector  MakeEulerAnglesFromQ(Quaternion q);
    inline  Quaternion  MakeQFromAxis(float x, float y, float z, float angle);
    inline Quaternion FindRotation(Quaternion qa, Quaternion qb);
    inline float FindAngle(Quaternion qa, Quaternion qb);
    inline Vector FindAxis(Quaternion qa, Quaternion qb);
    inline Quaternion FindRotation(Vector v1, Vector v2);

    inline  Quaternion::Quaternion(void)
    {
        n = 0;
        v.x = 0;
        v.y = 0;
        v.z = 0;
    }


    inline  Quaternion::Quaternion(float e0, float e1, float e2, float e3)
    {
        n = e0;
        v.x = e1;
        v.y = e2;
        v.z = e3;
    }

    inline  void Quaternion::Set(float e0, float e1, float e2, float e3)
    {
        n = e0;
        v.x = e1;
        v.y = e2;
        v.z = e3;
    }

    inline  void Quaternion::Set(const float *q_nxyz)
    {
        n = q_nxyz[0];
        v.x = q_nxyz[1];
        v.y = q_nxyz[2];
        v.z = q_nxyz[3];
    }

    inline  float   Quaternion::Magnitude(void)
    {
        return std::sqrt(n*n + v.x*v.x + v.y*v.y + v.z*v.z);
    }

    inline  Vector  Quaternion::GetVector(void)
    {
        return Vector(v.x, v.y, v.z);
    }

    inline  float   Quaternion::GetScalar(void)
    {
        return n;
    }

    inline  Quaternion  Quaternion::operator+=(Quaternion q)
    {
        n += q.n;
        v.x += q.v.x;
        v.y += q.v.y;
        v.z += q.v.z;
        return *this;
    }

    inline  Quaternion  Quaternion::operator-=(Quaternion q)
    {
        n -= q.n;
        v.x -= q.v.x;
        v.y -= q.v.y;
        v.z -= q.v.z;
        return *this;
    }

    inline  Quaternion Quaternion::operator*=(float s)
    {
        n *= s;
        v.x *= s;
        v.y *= s;
        v.z *= s;
        return *this;
    }

    inline  Quaternion Quaternion::operator/=(float s)
    {
        n /= s;
        v.x /= s;
        v.y /= s;
        v.z /= s;
        return *this;
    }

    inline float *Quaternion::data()
    {
        return &n;
    }


    inline  Quaternion operator+(Quaternion q1, Quaternion q2)
    {
        return  Quaternion( q1.n + q2.n,
                            q1.v.x + q2.v.x,
                            q1.v.y + q2.v.y,
                            q1.v.z + q2.v.z);
    }

    inline  Quaternion operator-(Quaternion q1, Quaternion q2)
    {
        return  Quaternion( q1.n - q2.n,
                            q1.v.x - q2.v.x,
                            q1.v.y - q2.v.y,
                            q1.v.z - q2.v.z);
    }

    inline  Quaternion operator*(Quaternion q1, Quaternion q2)
    {
        return  Quaternion( q1.n*q2.n - q1.v.x*q2.v.x - q1.v.y*q2.v.y - q1.v.z*q2.v.z,
                            q1.n*q2.v.x + q1.v.x*q2.n + q1.v.y*q2.v.z - q1.v.z*q2.v.y,
                            q1.n*q2.v.y + q1.v.y*q2.n + q1.v.z*q2.v.x - q1.v.x*q2.v.z,
                            q1.n*q2.v.z + q1.v.z*q2.n + q1.v.x*q2.v.y - q1.v.y*q2.v.x);
    }

    inline  Quaternion operator*(Quaternion q, float s)
    {
        return  Quaternion(q.n*s, q.v.x*s, q.v.y*s, q.v.z*s);
    }

    inline  Quaternion operator*(float s, Quaternion q)
    {
        return  Quaternion(q.n*s, q.v.x*s, q.v.y*s, q.v.z*s);
    }

    inline  Quaternion operator*(Quaternion q, Vector v)
    {
        return  Quaternion( -(q.v.x*v.x + q.v.y*v.y + q.v.z*v.z),
                            q.n*v.x + q.v.y*v.z - q.v.z*v.y,
                            q.n*v.y + q.v.z*v.x - q.v.x*v.z,
                            q.n*v.z + q.v.x*v.y - q.v.y*v.x);
    }

    inline  Quaternion operator*(Vector v, Quaternion q)
    {
        return  Quaternion( -(q.v.x*v.x + q.v.y*v.y + q.v.z*v.z),
                            q.n*v.x + q.v.z*v.y - q.v.y*v.z,
                            q.n*v.y + q.v.x*v.z - q.v.z*v.x,
                            q.n*v.z + q.v.y*v.x - q.v.x*v.y);
    }

    inline  Quaternion operator/(Quaternion q, float s)
    {
        return  Quaternion(q.n/s, q.v.x/s, q.v.y/s, q.v.z/s);
    }

    inline  float QGetAngle(Quaternion q)
    {
        if (q.n <= -1) return 0; // 2 * pi
        if (q.n >= 1) return 0; // 2 * 0
        return  (2*std::acos(q.n));
    }

    inline  Vector QGetAxis(Quaternion q)
    {
        Vector v = q.GetVector();
        v.Normalize();
        return v;
    }

    inline  Quaternion QRotate(Quaternion q1, Quaternion q2)
    {
        return  q1*q2*(~q1);
    }

    inline  Vector  QVRotate(Quaternion q, Vector v)
    {
#ifdef EASY_TO_READ
        Quaternion t;


        t = q*v*(~q);

        return  t.GetVector();
#else
    // optimisation based on OpenSG code
    float rx,ry,rz;
    float QwQx, QwQy, QwQz, QxQy, QxQz, QyQz;

    QwQx = q.n * q.v.x;
    QwQy = q.n * q.v.y;
    QwQz = q.n * q.v.z;
    QxQy = q.v.x * q.v.y;
    QxQz = q.v.x * q.v.z;
    QyQz = q.v.y * q.v.z;

    rx = 2* (v.y * (-QwQz + QxQy) + v.z *( QwQy + QxQz));
    ry = 2* (v.x * ( QwQz + QxQy) + v.z *(-QwQx + QyQz));
    rz = 2* (v.x * (-QwQy + QxQz) + v.y *( QwQx + QyQz));

    float QwQw, QxQx, QyQy, QzQz;

    QwQw = q.n * q.n;
    QxQx = q.v.x * q.v.x;
    QyQy = q.v.y * q.v.y;
    QzQz = q.v.z * q.v.z;

    rx+= v.x * (QwQw + QxQx - QyQy - QzQz);
    ry+= v.y * (QwQw - QxQx + QyQy - QzQz);
    rz+= v.z * (QwQw - QxQx - QyQy + QzQz);

    return Vector(rx,ry,rz);

#endif
    }

    // these are intrinsic Euler XYZ angles (or fixed axis ZYX)
    inline  Quaternion  MakeQFromEulerAngles(float x, float y, float z)
    {
        Quaternion  q;
        float   roll = DegreesToRadians(x);
        float   pitch = DegreesToRadians(y);
        float   yaw = DegreesToRadians(z);

        float   cyaw, cpitch, croll, syaw, spitch, sroll;
        float   cyawcpitch, syawspitch, cyawspitch, syawcpitch;

        cyaw = std::cos(0.5f * yaw);
        cpitch = std::cos(0.5f * pitch);
        croll = std::cos(0.5f * roll);
        syaw = std::sin(0.5f * yaw);
        spitch = std::sin(0.5f * pitch);
        sroll = std::sin(0.5f * roll);

        cyawcpitch = cyaw*cpitch;
        syawspitch = syaw*spitch;
        cyawspitch = cyaw*spitch;
        syawcpitch = syaw*cpitch;

        q.n = (cyawcpitch * croll + syawspitch * sroll);
        q.v.x = (cyawcpitch * sroll - syawspitch * croll);
        q.v.y = (cyawspitch * croll + syawcpitch * sroll);
        q.v.z = (syawcpitch * croll - cyawspitch * sroll);

        return q;
    }

    // these are intrinsic Euler XYZ angles (or fixed axis ZYX)
    inline  Vector  MakeEulerAnglesFromQ(Quaternion q)
    {
        float   r11, r21, r31, r32, r33, r12, r13;
        float   q00, q11, q22, q33;
        float   tmp;
        Vector  u;

        q00 = q.n * q.n;
        q11 = q.v.x * q.v.x;
        q22 = q.v.y * q.v.y;
        q33 = q.v.z * q.v.z;

        r11 = q00 + q11 - q22 - q33;
        r21 = 2 * (q.v.x*q.v.y + q.n*q.v.z);
        r31 = 2 * (q.v.x*q.v.z - q.n*q.v.y);
        r32 = 2 * (q.v.y*q.v.z + q.n*q.v.x);
        r33 = q00 - q11 - q22 + q33;

        tmp = MYMATH_ABS(r31);
        if(tmp > (1 - epsilon))
        {
            r12 = 2 * (q.v.x*q.v.y - q.n*q.v.z);
            r13 = 2 * (q.v.x*q.v.z + q.n*q.v.y);

            u.x = RadiansToDegrees(0.0); //roll
            u.y = RadiansToDegrees((-(pi/2) * r31/tmp)); // pitch
            u.z = RadiansToDegrees(std::atan2(-r12, -r31*r13)); // yaw
            return u;
        }

        u.x = RadiansToDegrees(std::atan2(r32, r33)); // roll
        u.y = RadiansToDegrees(std::asin(-r31));      // pitch
        u.z = RadiansToDegrees(std::atan2(r21, r11)); // yaw
        return u;


    }

    // wis  - new routine to make a Quaternion from an axis and a rotation angle in radians
    inline  Quaternion  MakeQFromAxis(float x, float y, float z, float angle)
    {
        Quaternion  q;

        Vector v(x, y, z);
        v.Normalize();

        while (angle > pi) angle -= (2 * pi);
        while (angle < -pi) angle += (2 * pi);

        float sin_a = std::sin( angle / 2 );
        float cos_a = std::cos( angle / 2 );

        q.v.x    = v.x * sin_a;
        q.v.y    = v.y * sin_a;
        q.v.z    = v.z * sin_a;
        q.n    = cos_a;

        return q;
    }

    // wis - new routines to calculate the quaternion which rotates qa to qb
    //
    // based on http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/index.htm
    // if q is the quaternion which rotates from qa to qb then:
    // qb = qa * q
    //
    // multiplying both sides by conj(qa) gives:
    // q = conj(qa) * qb
    //
    // The real part of a multiplication is:
    // real((qa.w + i qa.x + j qa.y + k qa.z)*(qb.w + i qb.x + j qb.y + k qb.z)) = qa.w * qb.w - qa.x*qb.x - qa.y*qb.y- qa.z*qb.z
    //
    // So using the conjugate of qa gives:
    // real((qa.w - i qa.x - j qa.y - k qa.z)*(qb.w + i qb.x + j qb.y + k qb.z)) = qa.w*qb.w + qa.x*qb.x + qa.y*qb.y+ qa.z*qb.z
    //
    // real(q) = std::cos(t/2)
    //
    // Therefore
    // std::cos(theta/2) = qa.w*qb.w + qa.x*qb.x + qa.y*qb.y+ qa.z*qb.z
    inline Quaternion FindRotation(Quaternion qa, Quaternion qb)
    {
        return ((~qa) * qb);
    }
    inline float FindAngle(Quaternion qa, Quaternion qb)
    {
        float v = qa.n*qb.n + qa.v.x*qb.v.x + qa.v.y*qb.v.y+ qa.v.z*qb.v.z;
        if (v <= -1) return 0; // 2 * pi
        if (v >= 1) return 0; // 2 * 0
        float angle = 2 * std::acos(v);
        if (angle < -pi) angle += (2 * pi);
        else if (angle > pi) angle -= (2 * pi);
        return angle;
    }
    inline Vector FindAxis(Quaternion qa, Quaternion qb)
    {
        return QGetAxis((~qa) * qb);
    }

    // this is useful when reading in quaternions since there is inevitable precision loss
    // which means the quaternion will be slightly denormal
    inline void Quaternion::Normalize()
    {
        float l = std::sqrt(n*n + v.x*v.x + v.y*v.y + v.z*v.z);
        n /= l;
        v.x /= l;
        v.y /= l;
        v.z /= l;
    }

    // this routine returns the quaternion that rotates v1 to v2 via the shortest path
    inline Quaternion FindRotation(Vector v1, Vector v2)
    {
        Quaternion q;
        q.n = std::sqrt((v1.Magnitude2()) * (v2.Magnitude2())) + (v1 * v2);
        if (q.n < epsilon) // this only occurs if a 180 degree rotation is needed
        {
            Vector perp; // this is a perpendicular vector (v1 dot perp = 0)
            if (fabs(v1.z) > epsilon) perp = Vector(0, -v1.z, v1.y);
            else perp = Vector(-v1.y, v1.x, 0);
            q = MakeQFromAxis(perp.x, perp.y, perp.z, pi);
        }
        else
        {
            q.v = v1 ^ v2;
            q.Normalize();
        }
        return q;
    }


    //------------------------------------------------------------------------//
    // Matrix Class and matrix functions
    //------------------------------------------------------------------------//

    class Matrix3x3 {
public:
        // elements eij: i -> row, j -> column
        float   e11, e12, e13, e21, e22, e23, e31, e32, e33;

        Matrix3x3(void);
        Matrix3x3(  float r1c1, float r1c2, float r1c3,
                    float r2c1, float r2c2, float r2c3,
                    float r3c1, float r3c2, float r3c3 );
        Matrix3x3(Quaternion q);

        void Set(   float r1c1, float r1c2, float r1c3,
                    float r2c1, float r2c2, float r2c3,
                    float r3c1, float r3c2, float r3c3 );

        void Set(const float *mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3);

        float *data();

        float   det(void);
        Matrix3x3   Transpose(void);
        Matrix3x3   Inverse(void);

        Matrix3x3& operator+=(Matrix3x3 m);
        Matrix3x3& operator-=(Matrix3x3 m);
        Matrix3x3& operator*=(float s);
        Matrix3x3& operator/=(float s);
    };

    inline  Matrix3x3 operator+(Matrix3x3 m1, Matrix3x3 m2);
    inline  Matrix3x3 operator-(Matrix3x3 m1, Matrix3x3 m2);
    inline  Matrix3x3 operator/(Matrix3x3 m, float s);
    inline  Matrix3x3 operator*(Matrix3x3 m1, Matrix3x3 m2);
    inline  Matrix3x3 operator*(Matrix3x3 m, float s);
    inline  Matrix3x3 operator*(float s, Matrix3x3 m);
    inline  Vector operator*(Matrix3x3 m, Vector u);
    inline  Vector operator*(Vector u, Matrix3x3 m);

    inline  Matrix3x3::Matrix3x3(void)
    {
        e11 = 0;
        e12 = 0;
        e13 = 0;
        e21 = 0;
        e22 = 0;
        e23 = 0;
        e31 = 0;
        e32 = 0;
        e33 = 0;
    }

    inline  Matrix3x3::Matrix3x3(   float r1c1, float r1c2, float r1c3,
                                    float r2c1, float r2c2, float r2c3,
                                    float r3c1, float r3c2, float r3c3 )
    {
        e11 = r1c1;
        e12 = r1c2;
        e13 = r1c3;
        e21 = r2c1;
        e22 = r2c2;
        e23 = r2c3;
        e31 = r3c1;
        e32 = r3c2;
        e33 = r3c3;
    }

    inline  void Matrix3x3::Set( float r1c1, float r1c2, float r1c3,
                                 float r2c1, float r2c2, float r2c3,
                                 float r3c1, float r3c2, float r3c3 )
    {
        e11 = r1c1;
        e12 = r1c2;
        e13 = r1c3;
        e21 = r2c1;
        e22 = r2c2;
        e23 = r2c3;
        e31 = r3c1;
        e32 = r3c2;
        e33 = r3c3;
    }

    inline void Matrix3x3::Set(const float *mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3)
    {
        e11 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[0];
        e12 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[1];
        e13 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[2];
        e21 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[3];
        e22 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[4];
        e23 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[5];
        e31 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[6];
        e32 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[7];
        e33 = mat_r1c1r1c2r1c3_r2c1r2c2r2c3_r3c1r3c2r3c3[8];
    }

    inline float *Matrix3x3::data()
    {
        return &e11;
    }

    // initialise a matrix from a quaternion
    // based on code from
    // http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
    // untested!
    inline  Matrix3x3::Matrix3x3(Quaternion q)
    {
        float sqw = q.n*q.n;
        float sqx = q.v.x*q.v.x;
        float sqy = q.v.y*q.v.y;
        float sqz = q.v.z*q.v.z;

        // invs (inverse square length) is only required if quaternion is not already normalised
        float invs = 1 / (sqx + sqy + sqz + sqw);
        e11 = ( sqx - sqy - sqz + sqw)*invs ; // since sqw + sqx + sqy + sqz =1/invs*invs
        e22 = (-sqx + sqy - sqz + sqw)*invs ;
        e33 = (-sqx - sqy + sqz + sqw)*invs ;

        float tmp1 = q.v.x*q.v.y;
        float tmp2 = q.v.z*q.n;
        e21 = 2.0f * (tmp1 + tmp2)*invs ;
        e12 = 2.0f * (tmp1 - tmp2)*invs ;

        tmp1 = q.v.x*q.v.z;
        tmp2 = q.v.y*q.n;
        e31 = 2.0f * (tmp1 - tmp2)*invs ;
        e13 = 2.0f * (tmp1 + tmp2)*invs ;
        tmp1 = q.v.y*q.v.z;
        tmp2 = q.v.x*q.n;
        e32 = 2.0f * (tmp1 + tmp2)*invs ;
        e23 = 2.0f * (tmp1 - tmp2)*invs ;
    }

    inline  float   Matrix3x3::det(void)
    {
        return  e11*e22*e33 -
        e11*e32*e23 +
        e21*e32*e13 -
        e21*e12*e33 +
        e31*e12*e23 -
        e31*e22*e13;
    }

    inline  Matrix3x3   Matrix3x3::Transpose(void)
    {
        return Matrix3x3(e11,e21,e31,e12,e22,e32,e13,e23,e33);
    }

    inline  Matrix3x3   Matrix3x3::Inverse(void)
    {
        float   d = e11*e22*e33 -
        e11*e32*e23 +
        e21*e32*e13 -
        e21*e12*e33 +
        e31*e12*e23 -
        e31*e22*e13;

        if (std::fabs(d) < minPositive) d = 1;

        return  Matrix3x3(  (e22*e33-e23*e32)/d,
                            -(e12*e33-e13*e32)/d,
                            (e12*e23-e13*e22)/d,
                            -(e21*e33-e23*e31)/d,
                            (e11*e33-e13*e31)/d,
                            -(e11*e23-e13*e21)/d,
                            (e21*e32-e22*e31)/d,
                            -(e11*e32-e12*e31)/d,
                            (e11*e22-e12*e21)/d );
    }

    inline  Matrix3x3& Matrix3x3::operator+=(Matrix3x3 m)
    {
        e11 += m.e11;
        e12 += m.e12;
        e13 += m.e13;
        e21 += m.e21;
        e22 += m.e22;
        e23 += m.e23;
        e31 += m.e31;
        e32 += m.e32;
        e33 += m.e33;
        return *this;
    }

    inline  Matrix3x3& Matrix3x3::operator-=(Matrix3x3 m)
    {
        e11 -= m.e11;
        e12 -= m.e12;
        e13 -= m.e13;
        e21 -= m.e21;
        e22 -= m.e22;
        e23 -= m.e23;
        e31 -= m.e31;
        e32 -= m.e32;
        e33 -= m.e33;
        return *this;
    }

    inline  Matrix3x3& Matrix3x3::operator*=(float s)
    {
        e11 *= s;
        e12 *= s;
        e13 *= s;
        e21 *= s;
        e22 *= s;
        e23 *= s;
        e31 *= s;
        e32 *= s;
        e33 *= s;
        return *this;
    }

    inline  Matrix3x3& Matrix3x3::operator/=(float s)
    {
        e11 /= s;
        e12 /= s;
        e13 /= s;
        e21 /= s;
        e22 /= s;
        e23 /= s;
        e31 /= s;
        e32 /= s;
        e33 /= s;
        return *this;
    }

    inline  Matrix3x3 operator+(Matrix3x3 m1, Matrix3x3 m2)
    {
        return  Matrix3x3(  m1.e11+m2.e11,
                            m1.e12+m2.e12,
                            m1.e13+m2.e13,
                            m1.e21+m2.e21,
                            m1.e22+m2.e22,
                            m1.e23+m2.e23,
                            m1.e31+m2.e31,
                            m1.e32+m2.e32,
                            m1.e33+m2.e33);
    }

    inline  Matrix3x3 operator-(Matrix3x3 m1, Matrix3x3 m2)
    {
        return  Matrix3x3(  m1.e11-m2.e11,
                            m1.e12-m2.e12,
                            m1.e13-m2.e13,
                            m1.e21-m2.e21,
                            m1.e22-m2.e22,
                            m1.e23-m2.e23,
                            m1.e31-m2.e31,
                            m1.e32-m2.e32,
                            m1.e33-m2.e33);
    }

    inline  Matrix3x3 operator/(Matrix3x3 m, float s)
    {
        return  Matrix3x3(  m.e11/s,
                            m.e12/s,
                            m.e13/s,
                            m.e21/s,
                            m.e22/s,
                            m.e23/s,
                            m.e31/s,
                            m.e32/s,
                            m.e33/s);
    }

    inline  Matrix3x3 operator*(Matrix3x3 m1, Matrix3x3 m2)
    {
        return Matrix3x3(   m1.e11*m2.e11 + m1.e12*m2.e21 + m1.e13*m2.e31,
                            m1.e11*m2.e12 + m1.e12*m2.e22 + m1.e13*m2.e32,
                            m1.e11*m2.e13 + m1.e12*m2.e23 + m1.e13*m2.e33,
                            m1.e21*m2.e11 + m1.e22*m2.e21 + m1.e23*m2.e31,
                            m1.e21*m2.e12 + m1.e22*m2.e22 + m1.e23*m2.e32,
                            m1.e21*m2.e13 + m1.e22*m2.e23 + m1.e23*m2.e33,
                            m1.e31*m2.e11 + m1.e32*m2.e21 + m1.e33*m2.e31,
                            m1.e31*m2.e12 + m1.e32*m2.e22 + m1.e33*m2.e32,
                            m1.e31*m2.e13 + m1.e32*m2.e23 + m1.e33*m2.e33 );
    }

    inline  Matrix3x3 operator*(Matrix3x3 m, float s)
    {
        return  Matrix3x3(  m.e11*s,
                            m.e12*s,
                            m.e13*s,
                            m.e21*s,
                            m.e22*s,
                            m.e23*s,
                            m.e31*s,
                            m.e32*s,
                            m.e33*s);
    }

    inline  Matrix3x3 operator*(float s, Matrix3x3 m)
    {
        return  Matrix3x3(  m.e11*s,
                            m.e12*s,
                            m.e13*s,
                            m.e21*s,
                            m.e22*s,
                            m.e23*s,
                            m.e31*s,
                            m.e32*s,
                            m.e33*s);
    }

    inline  Vector operator*(Matrix3x3 m, Vector u)
    {
        return Vector(  m.e11*u.x + m.e12*u.y + m.e13*u.z,
                        m.e21*u.x + m.e22*u.y + m.e23*u.z,
                        m.e31*u.x + m.e32*u.y + m.e33*u.z);
    }

    inline  Vector operator*(Vector u, Matrix3x3 m)
    {
        return Vector(  u.x*m.e11 + u.y*m.e21 + u.z*m.e31,
                        u.x*m.e12 + u.y*m.e22 + u.z*m.e32,
                        u.x*m.e13 + u.y*m.e23 + u.z*m.e33);
    }

    inline Matrix3x3 MakeMFromQ(Quaternion q)
    {
        Matrix3x3 m;

        float qq1 = 2*q.v.x*q.v.x;
        float qq2 = 2*q.v.y*q.v.y;
        float qq3 = 2*q.v.z*q.v.z;
        m.e11 = 1 - qq2 - qq3;
        m.e12 = 2*(q.v.x*q.v.y - q.n*q.v.z);
        m.e13 = 2*(q.v.x*q.v.z + q.n*q.v.y);
        m.e21 = 2*(q.v.x*q.v.y + q.n*q.v.z);
        m.e22 = 1 - qq1 - qq3;
        m.e23 = 2*(q.v.y*q.v.z - q.n*q.v.x);
        m.e31 = 2*(q.v.x*q.v.z - q.n*q.v.y);
        m.e32 = 2*(q.v.y*q.v.z + q.n*q.v.x);
        m.e33 = 1 - qq1 - qq2;
        return m;
    }

    inline Quaternion MakeQfromM (Matrix3x3 R)
    {
        Quaternion q;
        float tr,s;
        tr = R.e11 + R.e22 + R.e33;
        if (tr >= 0)
        {
            s = std::sqrt (tr + 1);
            q.n = 0.5f * s;
            s = 0.5f * (1.0f/s);
            q.v.x = (R.e32 - R.e23) * s;
            q.v.y = (R.e13 - R.e31) * s;
            q.v.z = (R.e21 - R.e12) * s;
        }
        else
        {
            // find the largest diagonal element and jump to the appropriate case
            if (R.e22 > R.e11)
            {
                if (R.e33 > R.e22) goto case_2;
                goto case_1;
            }
            if (R.e33 > R.e11) goto case_2;
            goto case_0;

case_0:
            s = std::sqrt((R.e11 - (R.e22 + R.e33)) + 1);
            q.v.x = 0.5f * s;
            s = 0.5f * (1.0f/s);
            q.v.y = (R.e12 + R.e21) * s;
            q.v.z = (R.e31 + R.e13) * s;
            q.n = (R.e32 - R.e23) * s;
            return q;

case_1:
            s = std::sqrt((R.e22 - (R.e33 + R.e11)) + 1);
            q.v.y = 0.5f * s;
            s = 0.5f * (1.0f/s);
            q.v.z = (R.e23 + R.e32) * s;
            q.v.x = (R.e12 + R.e21) * s;
            q.n = (R.e13 - R.e31) * s;
            return q;

case_2:
            s = std::sqrt((R.e33 - (R.e11 + R.e22)) + 1);
            q.v.z = 0.5f * s;
            s = 0.5f * (1.0f/s);
            q.v.x = (R.e31 + R.e13) * s;
            q.v.y = (R.e23 + R.e32) * s;
            q.n = (R.e21 - R.e12) * s;
            return q;
        }
        return q;
    }

    // find the closest point to point P on a line defined as origin B and direction M
    // using formulae from www.geometrictools.com
    inline Vector ClosestPoint(Vector P, Vector B, Vector M)
    {
        float t0 = (M * (P - B)) / (M * M);
        Vector Q = B + (t0 * M);
        return Q;
    }

    //  generates a quaternion between two given quaternions in proportion to the variable t
    // if t=0 then qm=qa, if t=1 then qm=qb, if t is between them then qm will interpolate between them
    inline Quaternion slerp(Quaternion v0, Quaternion v1, float t, bool normalise = true)
    {
        if (normalise)
        {
            // Only unit quaternions are valid rotations.
            // Normalize to avoid undefined behavior.
            v0.Normalize();
            v1.Normalize();
        }

        // Compute the cosine of the angle between the two vectors.
        float dot = v0.v * v1.v; // dot product

        const float DOT_THRESHOLD = 0.9995f;
        if (dot > DOT_THRESHOLD)
        {
            // If the inputs are too close for comfort, linearly interpolate
            // and normalize the result.

            Quaternion result = v0 + t * (v1 - v0);
            result.Normalize();
            return result;
        }

        // If the dot product is negative, the quaternions
        // have opposite handed-ness and slerp won't take
        // the shorter path. Fix by reversing one quaternion.
        if (dot < 0.0f)
        {
            v1 = -v1;
            dot = -dot;
        }

        MYMATH_CLAMP(dot, -1, 1);    // Robustness: Stay within domain of std::acos()
        float theta_0 = std::acos(dot);  // theta_0 = angle between input vectors
        float theta = theta_0*t;    // theta = angle between v0 and result

        Quaternion v2 = v1 - v0*dot;
        v2.Normalize();              // { v0, v2 } is now an orthonormal basis

        return v0*std::cos(theta) + v2*std::sin(theta);
    }

}


#endif
