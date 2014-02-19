#pragma once
#ifndef id02E43C5E_F580_4EEB_BBF83F09B52455C6
#define id02E43C5E_F580_4EEB_BBF83F09B52455C6

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <ctime>
#include <limits>
#include <algorithm>
#include <vector>
#include <initializer_list>
#define M_RAD(x) ((x)*M_PI/180)
#define M_DEG(x) ((x)/M_PI*180)

#define number double
#define number_eq(a, b) \
    (       std::abs((a)-(b)) < 2.0*std::numeric_limits<number>::epsilon() \
        ||  std::abs((a)-(b)) < std::min((a),(b))/1.0e15 \
    )
#define number_eqdelta(a, b, delta) (std::abs((a)-(b)) < delta)
#define number_clamp(v, low, high) (std::max(low, std::min(v, high)))
#define number_rand() ((double)rand()/(double)RAND_MAX)
#define randrange(min, max) ((min) + (rand() % (int)((max) - (min) + 1)))

const number number_min = std::numeric_limits<number>::epsilon();
const number number_max = std::numeric_limits<number>::max();

class mat4;
class vec3;
class vec4;
class quat;

class line;
class lineSegment;
class plane;
class triangle;

typedef enum {
    NONE = 0, POINT, LINE, PLANE
} EIntersectType;
//class SIntersectResult;

class mat4 {
    public:
        number v[4*4];
        
        inline mat4() : v{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} { };
        inline mat4(
            number  e1, number  e2, number  e3, number  e4,
            number  e5, number  e6, number  e7, number  e8,
            number  e9, number e10, number e11, number e12,
            number e13, number e14, number e15, number e16
        ) : v{e1,e2,e3,e4,e5,e6,e7,e8,e9,e10,e11,e12,e13,e14,e15,e16} { };
        
        static const mat4 identity; // defined in .cpp file
        
        inline mat4 inverse() {
            // http://stackoverflow.com/a/7596981/837856
            number s0 = v[ 0] * v[ 5] - v[ 4] * v[ 1];
            number s1 = v[ 0] * v[ 6] - v[ 4] * v[ 2];
            number s2 = v[ 0] * v[ 7] - v[ 4] * v[ 3];
            number s3 = v[ 1] * v[ 6] - v[ 5] * v[ 2];
            number s4 = v[ 1] * v[ 7] - v[ 5] * v[ 3];
            number s5 = v[ 2] * v[ 7] - v[ 6] * v[ 3];
            
            number c5 = v[10] * v[15] - v[14] * v[11];
            number c4 = v[ 9] * v[15] - v[13] * v[11];
            number c3 = v[ 9] * v[14] - v[13] * v[10];
            number c2 = v[ 8] * v[15] - v[12] * v[11];
            number c1 = v[ 8] * v[14] - v[12] * v[10];
            number c0 = v[ 8] * v[13] - v[12] * v[ 9];
            
            number det = (s0*c5 -s1*c4 +s2*c3 +s3*c2 -s4*c1 +s5*c0);
            //assert(!number_eq(det, 0.0));
            
            number invdet = 1/det;
            mat4 r(
                ( v[ 5]*c5 -v[ 6]*c4 +v[ 7]*c3)*invdet,
                (-v[ 1]*c5 +v[ 2]*c4 -v[ 3]*c3)*invdet,
                ( v[13]*s5 -v[14]*s4 +v[15]*s3)*invdet,
                (-v[ 9]*s5 +v[10]*s4 -v[11]*s3)*invdet,
                
                (-v[ 4]*c5 +v[ 6]*c2 -v[ 7]*c1)*invdet,
                ( v[ 0]*c5 -v[ 2]*c2 +v[ 3]*c1)*invdet,
                (-v[12]*s5 +v[14]*s2 -v[15]*s1)*invdet,
                ( v[ 8]*s5 -v[10]*s2 +v[11]*s1)*invdet,
                
                ( v[ 4]*c4 -v[ 5]*c2 +v[ 7]*c0)*invdet,
                (-v[ 0]*c4 +v[ 1]*c2 -v[ 3]*c0)*invdet,
                ( v[12]*s4 -v[13]*s2 +v[15]*s0)*invdet,
                (-v[ 8]*s4 +v[ 9]*s2 -v[11]*s0)*invdet,
                
                (-v[ 4]*c3 +v[ 5]*c1 -v[ 6]*c0)*invdet,
                ( v[ 0]*c3 -v[ 1]*c1 +v[ 2]*c0)*invdet,
                (-v[12]*s3 +v[13]*s1 -v[14]*s0)*invdet,
                ( v[ 8]*s3 -v[ 9]*s1 +v[10]*s0)*invdet
            );
            return r;
        }
        
        inline void print(const char * s = "") const {
            printf(
                "%smat4(%- 4.3f, %- 4.3f, %- 4.3f, %- 4.3f,\n"
                "     %- 4.3f, %- 4.3f, %- 4.3f, %- 4.3f,\n"
                "     %- 4.3f, %- 4.3f, %- 4.3f, %- 4.3f,\n"
                "     %- 4.3f, %- 4.3f, %- 4.3f, %- 4.3f\n"
                ")\n",
                s,
                v[ 0],v[ 1],v[ 2],v[ 3],
                v[ 4],v[ 5],v[ 6],v[ 7],
                v[ 8],v[ 9],v[10],v[11],
                v[12],v[13],v[14],v[15]
            );
            printf(
                "{{%.3f,%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f,%.3f},{%.3f,%.3f,%.3f,%.3f}}\n",
                v[ 0],v[ 1],v[ 2],v[ 3],
                v[ 4],v[ 5],v[ 6],v[ 7],
                v[ 8],v[ 9],v[10],v[11],
                v[12],v[13],v[14],v[15]
            );
        }
        
        number* operator[](const int iIndex) {
            return v+iIndex*4;
        }
        number &operator()(const int iIndex) {
            return v[iIndex];
        }
        
        operator double*() {
            return v;
        }
};

inline mat4 operator*(mat4 &a, mat4 &b) {
    // https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/mathlib/vmatrix.cpp
    mat4 c(
        a[0][0]*b[0][0] + a[0][1]*b[1][0] + a[0][2]*b[2][0] + a[0][3]*b[3][0],
        a[0][0]*b[0][1] + a[0][1]*b[1][1] + a[0][2]*b[2][1] + a[0][3]*b[3][1],
        a[0][0]*b[0][2] + a[0][1]*b[1][2] + a[0][2]*b[2][2] + a[0][3]*b[3][2],
        a[0][0]*b[0][3] + a[0][1]*b[1][3] + a[0][2]*b[2][3] + a[0][3]*b[3][3],
        
        a[1][0]*b[0][0] + a[1][1]*b[1][0] + a[1][2]*b[2][0] + a[1][3]*b[3][0],
        a[1][0]*b[0][1] + a[1][1]*b[1][1] + a[1][2]*b[2][1] + a[1][3]*b[3][1],
        a[1][0]*b[0][2] + a[1][1]*b[1][2] + a[1][2]*b[2][2] + a[1][3]*b[3][2],
        a[1][0]*b[0][3] + a[1][1]*b[1][3] + a[1][2]*b[2][3] + a[1][3]*b[3][3],
        
        a[2][0]*b[0][0] + a[2][1]*b[1][0] + a[2][2]*b[2][0] + a[2][3]*b[3][0],
        a[2][0]*b[0][1] + a[2][1]*b[1][1] + a[2][2]*b[2][1] + a[2][3]*b[3][1],
        a[2][0]*b[0][2] + a[2][1]*b[1][2] + a[2][2]*b[2][2] + a[2][3]*b[3][2],
        a[2][0]*b[0][3] + a[2][1]*b[1][3] + a[2][2]*b[2][3] + a[2][3]*b[3][3],
        
        a[3][0]*b[0][0] + a[3][1]*b[1][0] + a[3][2]*b[2][0] + a[3][3]*b[3][0],
        a[3][0]*b[0][1] + a[3][1]*b[1][1] + a[3][2]*b[2][1] + a[3][3]*b[3][1],
        a[3][0]*b[0][2] + a[3][1]*b[1][2] + a[3][2]*b[2][2] + a[3][3]*b[3][2],
        a[3][0]*b[0][3] + a[3][1]*b[1][3] + a[3][2]*b[2][3] + a[3][3]*b[3][3]
    );
    return c;
}

inline bool operator==(mat4 &a, mat4 &b) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if(!number_eq(a[i][j], b[i][j])) {
                return false;
            }
        }
    }
    return true;
}
inline bool operator!=(mat4 &a, mat4 &b) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if(!number_eq(a[i][j], b[i][j])) {
                return true;
            }
        }
    }
    return false;
}
inline mat4 operator-(mat4 &a, mat4 &b) {
    mat4 c;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            c[i][j] = a[i][j]-b[i][j];
        }
    }
    return c;
}

class __attribute__ ((__packed__)) vec3 {
    public:
        number x, y, z;
        
        inline vec3() : x(0), y(0), z(0) { };
        inline vec3(number _x, number _y, number _z) : x(_x), y(_y), z(_z) { };
        
        static const vec3 zero; // defined in .cpp file
        static const vec3 unit_x;
        static const vec3 unit_y;
        static const vec3 unit_z;
        
        inline number length() const {
            return sqrt(x*x+y*y+z*z);
        }
        inline number lengthSqr() const {
            return x*x+y*y+z*z;
        }
        inline vec3 unit() const {
            number l = length();
            return vec3(x/l, y/l, z/l);
        }
        inline vec3 abs() const {
            return vec3(
                x < 0 ? -x : x,
                y < 0 ? -y : y,
                z < 0 ? -z : z
            );
        }
        inline number dot(const vec3 &o) const {
            return x*o.x + y*o.y + z*o.z;
        }
        inline vec3 cross(const vec3 &o) const {
            return vec3(
                y*o.z - z*o.y,
                z*o.x - x*o.z,
                x*o.y - y*o.x
            );
        }
        
        inline vec3 lerp(const vec3 &that, number t, bool limit = false) {
            // http://www.arcsynthesis.org/gltut/Positioning/Tut08%20Interpolation.html
            if(limit) {
                t = std::min(std::max(0.0, t), 1.0);
            }
            return vec3(
                x*(1-t)+that.x*t,
                y*(1-t)+that.y*t,
                z*(1-t)+that.z*t
            );
        }
        
        inline void print(const char * s = "") const {
            printf(
                "%svec3(%- 4.3f, %- 4.3f, %- 4.3f)\n",
                s, x, y, z
           );
        }
        
        inline number& operator[](const int iIndex) { return ((number*)this)[iIndex]; }
        inline number operator[](const int iIndex) const { return ((number*)this)[iIndex]; }
        
        inline void operator=(const vec3 &that) {
            x = that.x;
            y = that.y;
            z = that.z;
        }
};
inline vec3 operator+(const vec3 a, const vec3 b) {
    return vec3(a.x+b.x, a.y+b.y, a.z+b.z);
}
inline vec3 operator-(const vec3 a, const vec3 b) {
    return vec3(a.x-b.x, a.y-b.y, a.z-b.z);
}
inline vec3 operator-(const vec3 a) {
    return vec3(-a.x, -a.y, -a.z);
}
inline vec3 operator*(const vec3 a, const vec3 b) {
    return vec3(a.x*b.x, a.y*b.y, a.z*b.z);
}
inline vec3 operator*(const vec3 a, number s) {
    return vec3(a.x*s, a.y*s, a.z*s);
}
inline vec3 operator*(number s, const vec3 a) {
    return vec3(a.x*s, a.y*s, a.z*s);
}
inline vec3 operator/(const vec3 a, number s) {
    return vec3(a.x/s, a.y/s, a.z/s);
}
inline vec3 operator/(number s, const vec3 a) {
    return vec3(s/a.x, s/a.y, s/a.z);
}
inline bool operator==(const vec3 a, const vec3 b) {
    return (
            number_eq(a.x, b.x)
        &&  number_eq(a.y, b.y)
        &&  number_eq(a.z, b.z)
    );
}
inline bool operator!=(const vec3 &a, const vec3 &b) {
    return !(a == b);
}

class vec4 {
    public:
        number x, y, z, w;
        
        inline vec4() : x(0), y(0), z(0), w(0) { };
        inline vec4(number _x, number _y, number _z, number _w) : x(_x), y(_y), z(_z), w(_w) { };
        inline vec4(vec3 v) : x(v.x), y(v.y), z(v.z), w(1) { };
        
        static const vec4 zero; // defined in .cpp file
        static const vec4 unit_x;
        static const vec4 unit_y;
        static const vec4 unit_z;
        static const vec4 unit_w;
        
        inline number length() const {
            return sqrt(x*x+y*y+z*z+w*w);
        }
        inline number lengthSqr() const {
            return x*x+y*y+z*z+w*w;
        }
        inline vec4 unit() const {
            number l = length();
            return vec4(x/l, y/l, z/l, w/l);
        }
        inline number dot(const vec4 &o) const {
            return x*o.x + y*o.y + z*o.z + w*o.w;
        }
        
        inline void print(const char * s = "") const {
            printf(
                "%svec4(%- 4.3f, %- 4.3f, %- 4.3f, %- 4.3f)\n",
                s, x, y, z, w
            );
        }
        
        inline number& operator[](const int iIndex) { return ((number*)this)[iIndex]; }
        inline number operator[](const int iIndex) const { return ((number*)this)[iIndex]; }
};
inline vec4 operator+(const vec4 &a, const vec4 &b) {
    return vec4(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
}
inline vec4 operator-(const vec4 &a, const vec4 &b) {
    return vec4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
}
inline vec4 operator-(const vec4 &a) {
    return vec4(-a.x, -a.y, -a.z, -a.w);
}
inline vec4 operator*(const vec4 &a, number s) {
    return vec4(a.x*s, a.y*s, a.z*s, a.w*s);
}
inline vec4 operator*(number s, const vec4 &a) {
    return vec4(a.x*s, a.y*s, a.z*s, a.w*s);
}
inline vec4 operator/(const vec4 &a, number s) {
    return vec4(a.x/s, a.y/s, a.z/s, a.w/s);
}
inline bool operator==(const vec4 &a, const vec4 &b) {
    return (
            number_eq(a.x, b.x)
        &&  number_eq(a.y, b.y)
        &&  number_eq(a.z, b.z)
        &&  number_eq(a.w, b.w)
    );
}
inline bool operator!=(const vec4 &a, const vec4 &b) {
    return !(a == b);
}
inline vec4 operator*(vec4 v, mat4 m) {
    vec4 r;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            r[i] += v[j]*m[j][i];
        }
    }
    return r;
}

inline vec3 operator*(vec3 v, mat4 m) {
    vec4 r;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            r[i] += v[j]*m[j][i];
        }
    }
    return vec3(r.x, r.y, r.z);
    // return vec3(r.x/r.w, r.y/r.w, r.z/r.w);
}

class quat;
inline quat operator+(const quat a, const quat b);
inline quat operator-(const quat a, const quat b);
inline quat operator*(quat a, number s);
inline quat operator*(number s, quat a);
inline quat operator*(const quat a, const quat b);
inline quat operator/(quat a, number s);
inline quat operator-(const quat a);

class quat {
    public:
        number a, b, c, d;
        inline quat() : a(1), b(0), c(0), d(0) { };
        inline quat(number _a, number _b, number _c, number _d) : a(_a), b(_b), c(_c), d(_d) { };
        inline quat(number ang, vec3 axis) {
            number halfang = ang*0.5;
            number halfang_sin = sin(halfang);
            a = cos(halfang);
            b = halfang_sin*axis.x;
            c = halfang_sin*axis.y;
            d = halfang_sin*axis.z;
        }
        
        static const quat identity; // defined in .cpp file
        
        static quat random() {
            // http://mathproofs.blogspot.com.au/2005/05/uniformly-distributed-random-unit.html
            number t0 = 2*M_PI*number_rand();
            number t1 = acos(1-2*number_rand());
            number t2 = 0.5*(M_PI*number_rand()+asin(number_rand())+M_PI*0.5);
            number r = 1; // ?
            return quat(
                r*sin(t0)*sin(t1)*sin(t2),
                r*cos(t0)*sin(t1)*sin(t2),
                r*cos(t1)*sin(t2),
                r*cos(t2)
            ).unit();
        }
        
        
        static quat fromEuler(number theta_z, number theta_y, number theta_x) {
            // http://willperone.net/Code/quaternion.php
            theta_z = M_RAD(theta_z);
            theta_y = M_RAD(theta_y);
            theta_x = M_RAD(theta_x);
            
            number cos_z_2 = cosf(0.5*theta_z);
            number cos_y_2 = cosf(0.5*theta_y);
            number cos_x_2 = cosf(0.5*theta_x);
            number sin_z_2 = sinf(0.5*theta_z);
            number sin_y_2 = sinf(0.5*theta_y);
            number sin_x_2 = sinf(0.5*theta_x);
            
            return quat(
                cos_z_2*cos_y_2*cos_x_2 + sin_z_2*sin_y_2*sin_x_2,
                cos_z_2*cos_y_2*sin_x_2 - sin_z_2*sin_y_2*cos_x_2,
                cos_z_2*sin_y_2*cos_x_2 + sin_z_2*cos_y_2*sin_x_2,
                sin_z_2*cos_y_2*cos_x_2 - cos_z_2*sin_y_2*sin_x_2
            );
        }
        inline vec3 toEuler(bool homogenous = true) {
            // http://willperone.net/Code/quaternion.php
            number sqw = a*a;
            number sqx = b*b;
            number sqy = c*c;
            number sqz = d*d;
            
            vec3 euler;
            if (homogenous) {
                euler.x = M_DEG(atan2(2.f * (b*c + d*a), sqx - sqy - sqz + sqw));
                euler.y = M_DEG(asin(-2.f * (b*d - c*a)));
                euler.z = M_DEG(atan2(2.f * (c*d + b*a), -sqx - sqy + sqz + sqw));
            } else {
                euler.x = M_DEG(atan2(2.f * (d*c + b*a), 1 - 2*(sqx + sqy)));
                euler.y = M_DEG(asin(-2.f * (b*d - c*a)));
                euler.z = M_DEG(atan2(2.f * (b*c + d*a), 1 - 2*(sqy + sqz)));
            }
            return euler;
        }
        
        inline number length() const {
            return sqrt(a*a+b*b+c*c+d*d);
        }
        inline number lengthSqr() const {
            return a*a+b*b+c*c+d*d;
        }
        inline quat unit() const {
            number l = length();
            return quat(a/l, b/l, c/l, d/l);
        }
        inline quat inverse() {
            return (-(*this))/lengthSqr();
        }
            
        
        inline number dot(const quat &o) const {
            return a*o.a + b*o.b + c*o.c + d*o.d;
        }
        inline quat ham(const quat &o) const {
            number
                a = a*o.a - b*o.b - c*o.c - d*o.d,
                b = a*o.b + b*o.a + c*o.d - d*o.c,
                c = a*o.c - b*o.d + c*o.a + d*o.b,
                d = a*o.d + b*o.c - c*o.b + d*o.a
            ;
            return quat(a, b, c, d);
        }
        inline vec3 vecmul(const vec3 &vec) const {
            vec3 qvec = vec3(b, c, d);
            vec3 uvec = qvec.cross(vec);
            vec3 uuvec = qvec.cross(uvec);
            return vec+uvec*2*a+uuvec*2;
        }
        inline vec3 right() const { return vecmul(vec3( 1, 0, 0)); }
        inline vec3 left () const { return vecmul(vec3(-1, 0, 0)); }
        inline vec3 up   () const { return vecmul(vec3( 0, 1, 0)); }
        inline vec3 down () const { return vecmul(vec3( 0,-1, 0)); }
        inline vec3 back () const { return vecmul(vec3( 0, 0, 1)); }
        inline vec3 front() const { return vecmul(vec3( 0, 0,-1)); }
        inline vec3 backward() const { return back(); }
        inline vec3 forward() const { return front(); }
        
        inline quat lerp(const quat &that, const number t) {
            // http://www.arcsynthesis.org/gltut/Positioning/Tut08%20Interpolation.html
            vec4 v0(b, c, d, a);
            vec4 v1(that.b, that.c, that.d, that.a);
            vec4 u = v0+(v1-v0)*t;
            vec4 r = (v0+(v1-v0)*t).unit();
            return quat(r.w, r.x, r.y, r.z);
        }
        
        /* inline quat slerp(const quat &that, const number t) {
            // http://www.arcsynthesis.org/gltut/Positioning/Tut08%20Interpolation.html
            quat v0 = *this;
            quat v1 = that;
            number dot = v0.dot(v1);
            if(dot > 0.9995) {
                return lerp(that, t);
            }
            dot = number_clamp(dot, -1.0, 1.0);
            number theta_0 = acos(dot);
            number theta = theta_0*t;
            
            quat v2 = (v1-v0*dot).unit();
            
            return (
                    quat(v0.a*cos(theta), v0.b*cos(theta), v0.c*cos(theta), v0.d*cos(theta))
                +   quat(v2.a*sin(theta), v2.b*sin(theta), v2.c*sin(theta), v2.d*sin(theta))
            );
        } */
        inline quat slerp(quat &that, number t) {
            // http://pastebin.com/PkVzFkf4
            quat &a = *this;
            quat &b = that;
            if(t <= 0) return a;
            if(t >= 1) return b;
            number coshalftheta = a.dot(b);
            quat c(b);
            if(coshalftheta < 0) {
                coshalftheta = -coshalftheta;
                c = -c;
            }
            if(coshalftheta > 0.99) {
                quat r = a.lerp(c, t);
                return r;
            }
            number halftheta = acos(coshalftheta);
            number sintheta = sin(halftheta);
            return (sin((1.0-t)*halftheta)*a + sin(t*halftheta)*c)/sin(halftheta);
        }
        
        static inline quat vecRotation(const vec3 vSrc, const vec3 vDst, const vec3 vFallbackAxis = vec3::zero) {
            // https://bitbucket.org/mogre/mogre/src/4327903e1bed/Main/src/Custom/MogreVector3.cpp
            quat q;
            vec3 v0 = vSrc.unit();
            vec3 v1 = vDst.unit();
            number d = v0.dot(v1);
            
            if(d >= 1.0f) {
                return quat();
            }
            if(d < -1.0f) {
                d = -1.0f;
            }
            number s = sqrt((1+d)*2);
            if(s < 1e-6f) {
                if(vFallbackAxis != vec3::zero) {
                    q = quat(M_PI, vFallbackAxis);
                } else {
                    vec3 axis = vec3::unit_y.cross(v0);
                    if(number_eq(axis.length(), 0.0)) {
                        axis = vec3::unit_x.cross(v0);
                    }
                    q = quat(M_PI, axis);
                }
            } else {
                number invs = 1/s;
                vec3 c = v0.cross(v1);
                q.a = s * 0.5;
                q.b = c.x * invs;
                q.c = c.y * invs;
                q.d = c.z * invs;
                q =q.unit();
            }
            return q;
        }
        
        inline void mat(mat4 &m) const {
            quat q = unit();
            m( 0)=1.0-2.0*q.c*q.c-2.0*q.d*q.d; m( 1)=    2.0*q.b*q.c-2.0*q.d*q.a; m( 2)=    2.0*q.b*q.d+2.0*q.c*q.a; m( 3)=0.0;
            m( 4)=    2.0*q.b*q.c+2.0*q.d*q.a; m( 5)=1.0-2.0*q.b*q.b-2.0*q.d*q.d; m( 6)=    2.0*q.c*q.d-2.0*q.b*q.a; m( 7)=0.0;
            m( 8)=    2.0*q.b*q.d-2.0*q.c*q.a; m( 9)=    2.0*q.c*q.d+2.0*q.b*q.a; m(10)=1.0-2.0*q.b*q.b-2.0*q.c*q.c; m(11)=0.0;
            m(12)=0.0                        ; m(13)=0.0                        ; m(14)=0.0                        ; m(15)=1.0;
        }
        
        inline number& operator[](const int iIndex) { return ((number*)this)[iIndex]; }
        inline number operator[](const int iIndex) const { return ((number*)this)[iIndex]; }
        
        // C++ wouldn't find these unless they were member functions..
        // inline quat operator+(const quat that) {
            // return quat(a+that.a, b+that.b, c+that.c, d+that.d);
        // }
        // inline quat operator-(const quat that) const {
            // return quat(a-that.a, b-that.b, c-that.c, d-that.d);
        // }
        // inline quat operator*(number s) const {
            // return quat(a*s, b*s, c*s, d*s);
        // }
};
inline quat operator+(const quat a, const quat b) {
    return quat(a.a+b.a, a.b+b.b, a.c+b.c, a.d+b.d);
}
inline quat operator-(const quat a, const quat b) {
    return quat(a.a-b.a, a.b-b.b, a.c-b.c, a.d-b.d);
}
inline quat operator*(const quat a, number s) {
    return quat(a.a*s, a.b*s, a.c*s, a.d*s);
}
inline quat operator*(number s, const quat a) {
    return quat(a.a*s, a.b*s, a.c*s, a.d*s);
}
inline quat operator/(const quat a, number s) {
    return quat(a.a/s, a.b/s, a.c/s, a.d/s);
}

inline quat operator*(const quat a, const quat b) {
    return a.ham(b);
}
inline quat operator-(const quat a) {
    return quat(-a.a, -a.b, -a.c, -a.d);
}

class ang3 {
    // Euler angles have issues, but they're easy to work with
    public:
        number y, p, r;
        
        inline ang3() {
            y = 0;
            p = 0;
            r = 0;
        }
        inline ang3(number _y, number _p, number _r) {
            y = fmod(_y, 2*M_PI);
            p = fmod(_p, 2*M_PI);
            r = fmod(_r, 2*M_PI);
        }
        
        inline number& operator[](const int iIndex) { return ((number*)this)[iIndex]; }
        inline number operator[](const int iIndex) const { return ((number*)this)[iIndex]; }
};
inline ang3 operator+(const ang3 &a, const ang3 &b) {
    return ang3(a.y+b.y, a.p+b.p, a.r+b.r);
}
inline ang3 operator-(const ang3 &a, const ang3 &b) {
    return ang3(a.y-b.y, a.p-b.p, a.r-b.r);
}

class line {
    public:
        vec3 a, b;
        
        inline line() : a(), b() { };
        inline line(vec3 _a, vec3 _b) : a(_a), b(_b) { };
        
        inline number& operator[](const int iIndex) { return ((number*)this)[iIndex]; }
        inline number operator[](const int iIndex) const { return ((number*)this)[iIndex]; }
};

class lineSegment {
    public:
        vec3 a, b;
        
        inline lineSegment() : a(), b() { };
        inline lineSegment(vec3 _a, vec3 _b) : a(_a), b(_b) { };
        
        inline number& operator[](const int iIndex) { return ((number*)this)[iIndex]; }
        inline number operator[](const int iIndex) const { return ((number*)this)[iIndex]; }
};

class plane {
    public:
        vec3 n;
        number d;
        
        inline plane() : n(0,0,0), d(0) { };
        inline plane(vec3 _n, number _d) : n(_n), d(_d) { };
        inline plane(vec3 a, vec3 b, vec3 c) {
            n = (b-a).cross(c-a).unit();
            d = (-n).dot(a);
        }
        /*plane(straightThing l1, straightThing l2) {
            vec3 a = l1.getFirst(), b = l1.getSecond();
            vec3 c = l2.getFirst(), d = l2.getSecond();
            assert(number_eq((b-a).dot((c-a).cross(d-a)), 0.0));
            
            n = (b-a).cross(c-a);
            d = a.length;
        }*/
        
        inline number dist(vec3 r) {
            return n.dot(r)+d;
        }
        
        inline EIntersectType intersect(line l, vec3 &vIntersectPoint) {
            // http://math.stackexchange.com/a/84088
            //SIntersectResult stResult;
            
            vec3 v = l.a-l.b;
            number dota = n.dot(l.a);
            number dotv = n.dot(v);
            
            if(number_eq(dotv, 0.0)) {
                return EIntersectType::NONE;
            }
            // TODO: Line lies in plane
            
            vIntersectPoint = l.a+(((d-dota)/dotv)*v);
            return EIntersectType::POINT;
        }
};

class triangle {
    public:
        vec3 a, b, c;
        
        inline triangle() : a(), b(), c() { };
        inline triangle(vec3 _a, vec3 _b, vec3 _c) : a(_a), b(_b), c(_c) { };
        
        inline EIntersectType intersect(line &l, vec3 &vIntersectPoint) {
            // http://www.geometrictools.com/LibMathematics/Intersection/Intersection.html
            //SIntersectResult stResult;
            
            vec3 dir = l.b-l.a;
            vec3 diff = l.a-a;
            vec3 edge1 = b-a;
            vec3 edge2 = c-a;
            vec3 normal = edge1.cross(edge2);
            
            // Solve Q + t*D = b1*E1 + b2*E2 (Q = kDiff, D = ray direction,
            // E1 = kEdge1, E2 = kEdge2, N = Cross(E1,E2)) by
            //   |Dot(D,N)|*b1 = sign(Dot(D,N))*Dot(D,Cross(Q,E2))
            //   |Dot(D,N)|*b2 = sign(Dot(D,N))*Dot(D,Cross(E1,Q))
            //   |Dot(D,N)|*t = -sign(Dot(D,N))*Dot(Q,N)
            number dot1 = dir.dot(normal);
            number sign;
            if (number_eq(dot1, 0.0)) {
                return EIntersectType::NONE;
            }
            sign = (dot1 > 0.0 ? 1 : -1);
            
            number cross1 = sign*dir.dot(diff.cross(edge2));
            if(cross1 >= 0.0) {
                number cross2 = sign*dir.dot(edge1.cross(diff));
                if(cross2 >= 0) {
                    if(cross1+cross2 < dot1) {
                        number dot2 = -sign*diff.dot(normal);
                        if(dot2 >= 0.0) {
                            number inv = 1.0/dot1;
                            
                            vIntersectPoint = l.a+(l.b-l.a)*dot2*inv;
                            return  EIntersectType::POINT;
                        }
                    }
                }
            }
            // TODO: Line lies in triangle
            return EIntersectType::NONE;
        }
        
        inline number& operator[](const int iIndex) { return ((number*)this)[iIndex]; }
        inline number operator[](const int iIndex) const { return ((number*)this)[iIndex]; }
};

class box_aabb {
    public:
        vec3 min, max;
        
        inline box_aabb() : min(), max() { };
        inline box_aabb(vec3 a, vec3 b) {
            set(a, b);
        }
        
        inline void set(vec3 a, vec3 b) {
            min.x = std::min(a.x, b.x); max.x = std::max(a.x, b.x);
            min.y = std::min(a.y, b.y); max.y = std::max(a.y, b.y);
            min.z = std::min(a.z, b.z); max.z = std::max(a.z, b.z);
        }
        
        inline void setFromCloud(std::vector<vec3> l) {
            min = vec3(number_max, number_max, number_max);
            max = vec3(number_min, number_min, number_min);
            for(auto it = l.begin(); it != l.end(); it++) {
                vec3 &p = *it;
                min.x = std::min(min.x, p.x); max.x = std::max(max.x, p.x);
                min.y = std::min(min.y, p.y); max.y = std::max(max.y, p.y);
                min.z = std::min(min.z, p.z); max.z = std::max(max.z, p.z);
            }
        }
        
        inline void getCorners(
            vec3 &v_rub, vec3 &v_ruf, vec3 &v_rdb, vec3 &v_rdf,
            vec3 &v_lub, vec3 &v_luf, vec3 &v_ldb, vec3 &v_ldf
        ) {
            v_rub = vec3(max.x, max.y, max.z); v_lub = vec3(min.x, max.y, max.z);
            v_ruf = vec3(max.x, max.y, min.z); v_luf = vec3(min.x, max.y, min.z);
            v_rdb = vec3(max.x, min.y, max.z); v_ldb = vec3(min.x, min.y, max.z);
            v_rdf = vec3(max.x, min.y, min.z); v_ldf = vec3(min.x, min.y, min.z);
        }
        inline void getCorners(
            vec3 *varray
        ) {
            varray[0] = vec3(max.x, max.y, max.z); varray[4] = vec3(min.x, max.y, max.z);
            varray[1] = vec3(max.x, max.y, min.z); varray[5] = vec3(min.x, max.y, min.z);
            varray[2] = vec3(max.x, min.y, max.z); varray[6] = vec3(min.x, min.y, max.z);
            varray[3] = vec3(max.x, min.y, min.z); varray[7] = vec3(min.x, min.y, min.z);
        }
        
        inline void getPN(vec3 normal, vec3 &p, vec3 &n) {
            p.x = normal.x >= 0 ? max.x : min.x;
            p.y = normal.y >= 0 ? max.y : min.y;
            p.z = normal.z >= 0 ? max.z : min.z;
            n.x = normal.x >= 0 ? min.x : max.x;
            n.y = normal.y >= 0 ? min.y : max.y;
            n.z = normal.z >= 0 ? min.z : max.z;
        }
        
        inline EIntersectType intersectRay(vec3 pos, vec3 dir, number &t, number &tmin, number &tmax) {
            // http://gamedev.stackexchange.com/a/18459
            vec3 dirfrac = 1.0 / dir.unit();
            if(number_eq(dirfrac.x, (number)0)) dirfrac.x += std::numeric_limits<number>::epsilon();
            if(number_eq(dirfrac.y, (number)0)) dirfrac.y += std::numeric_limits<number>::epsilon();
            if(number_eq(dirfrac.z, (number)0)) dirfrac.z += std::numeric_limits<number>::epsilon();
            double t1 = (min.x - pos.x)*dirfrac.x;
            double t2 = (max.x - pos.x)*dirfrac.x;
            double t3 = (min.y - pos.y)*dirfrac.y;
            double t4 = (max.y - pos.y)*dirfrac.y;
            double t5 = (min.z - pos.z)*dirfrac.z;
            double t6 = (max.z - pos.z)*dirfrac.z;
            
            tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
            tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));
            
            //printf("%f %f\n", tmin, tmax);
            
            if (tmax < 0) { // box is behind ray
                t = tmax;
                return EIntersectType::NONE;
            }
            if (tmin > tmax) {
                t = tmax;
                return EIntersectType::NONE;
            }
            t = tmin;
            return EIntersectType::POINT;
        }
        
        vec3 clamp(vec3 p) {
            return vec3(
                std::min(max.x, std::max(min.x, p.x)),
                std::min(max.y, std::max(min.y, p.y)),
                std::min(max.z, std::max(min.z, p.z))
            );
        }
};

class frustum {
    // http://zach.in.tu-clausthal.de/teaching/cg_literatur/lighthouse3d_view_frustum_culling/index.html
    public:
        enum {
            TOP = 0, BOTTOM, LEFT,
            RIGHT, NEAR, FAR
        };
        typedef enum {
            OUTSIDE,
            INTERSECT,
            INSIDE
        } EResult;
        
        plane planes[6];
        vec3 ntl, ntr, nbl, nbr, ftl, ftr, fbl, fbr;
        number dnear, dfar, ratio, angle, tang;
        number nw, nh, fw, fh;
        
        inline frustum() { };
        
        inline void setPerspective(number _angle, number _ratio, number _dnear, number _dfar) {
            angle = _angle;
            ratio = _ratio;
            dnear = _dnear;
            dfar = _dfar;
            
            tang = (double)tan(M_RAD(_angle)*0.5);
            nh = dnear*tang;
            nw = nh*ratio;
            fh = dfar*tang;
            fw = fh*ratio;
        }
        inline void setCoordinates(vec3 pos, vec3 dir, vec3 up) {
            vec3 axisX, axisY, axisZ;
            vec3 nearcentre, farcentre;
            
            axisZ = -dir.unit(); // pointing behind
            axisX = up.cross(axisZ).unit(); // pointing to side
            axisY = axisZ.cross(axisX); // point up
            
            nearcentre = pos-axisZ*dnear;
            farcentre  = pos-axisZ*dfar;
            
            // compute the 4 corners of the frustum on the near plane
            ntl = nearcentre +axisY*nh -axisX*nw;
            ntr = nearcentre +axisY*nh +axisX*nw;
            nbl = nearcentre -axisY*nh -axisX*nw;
            nbr = nearcentre -axisY*nh +axisX*nw;
            
            // compute the 4 corners of the frustum on the far plane
            ftl = farcentre  +axisY*fh -axisX*fw;
            ftr = farcentre  +axisY*fh +axisX*fw;
            fbl = farcentre  -axisY*fh -axisX*fw;
            fbr = farcentre  -axisY*fh +axisX*fw;
            
            // compute the six planes
            planes[TOP   ] = plane(ntr, ntl, ftl);
            planes[BOTTOM] = plane(nbl, nbr, fbr);
            planes[LEFT  ] = plane(ntl, nbl, fbl);
            planes[RIGHT ] = plane(nbr, ntr, fbr);
            planes[NEAR  ] = plane(ntl, ntr, nbr);
            planes[FAR   ] = plane(ftr, ftl, fbl);
        }
        
        inline EResult intersect(vec3 &p) {
            for(int planei = 0; planei < 6; planei++) {
                if(planes[planei].dist(p) < 0) {
                    return OUTSIDE;
                }
            }
            return INSIDE;
        }
        //inline EResult intersect(vec3 p) { return intersect(p); }
        /* inline EResult intersect(box_aabb &box) {
            EResult result = INSIDE;
            
            vec3 corners[8];
            stTestBox.getCorners(corners);
            for(int planei = 0; planei < 6; planei++) {
                int in = 0, out = 0;
                for(int corneri = 0; corneri < 8; corneri++) {
                    if(planes[planei].dist(corners[corneri]) < 0) {
                        out++; // corner lays outside of this plane
                    } else {
                        in++; // corner lays inside of this plane
                    }
                    if(in > 0 && out > 0) break; // we've found both an inside and outside point
                }
                if(in == 0) {
                    result = OUTSIDE; // no points are inside this plane, so cull it
                    break;
                } else if(out > 0) {
                    result = INTERSECT; // there are points inside and outside, so it's partial
                }
            }
            return result;
        } */
        inline EResult intersect(box_aabb &box) {
            EResult result = INSIDE;
            for(int planei = 0; planei < 6; planei++) {
                vec3 p, n;
                box.getPN(planes[planei].n, p, n);
                if(planes[planei].dist(p) < 0) {
                    result = OUTSIDE;
                    break;
                } else if (planes[planei].dist(n) < 0) {
                    result = INTERSECT;
                }
            }
            return result;
        }
        //inline EResult intersect(box_aabb box) { return intersect(box); }
};

class col3 {
    public:
        float r, g, b;
};
class col4 {
    public:
        float r, g, b, a;
};

inline void hsvToRgb(double h, double s, double v, double rgb[]) {
    // https://github.com/ratkins/RGBConverter/blob/master/RGBConverter.cpp
    double r, g, b;
    int i = int(h * 6);
    double f = h * 6 - i;
    double p = v * (1 - s);
    double q = v * (1 - f * s);
    double t = v * (1 - (1 - f) * s);
    switch(i % 6){
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}

/*
class SIntersectResult {
    EIntersectType eResultType;
    union {
        vec3 point;
        line line;
        plane plane;
    } uResultValue;
    inline bool getSucceeded() const {
        return eResultType != EIntersectType::NONE;
    }
    inline bool getPoint(vec3 &vOut) const {
        // TODO: Properly handle non-point intersections (probably not needed for this project :) )
        if(eResultType == EIntersectType::NONE) {
            return false;
        }
        // } else if(eResultType == EIntersectType::POINT) {
            // vOut = uResultValue.point;
        // } else if(eResultType == EIntersectType::LINE) {
            // vOut = uResultValue.line.a;
        // } else if(eResultType == EIntersectType::PLANE) {
            // vOut = uResultValue.plane.n*uResultValue.plane.d;
        // }
        vOut = uResultValue.point;
        return true;
    }
};
*/
#endif // header
