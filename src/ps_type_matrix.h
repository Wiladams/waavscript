#pragma once

#include <cmath>
#include <cstdio>
#include <memory>


namespace waavs {

    static constexpr double PI = 3.14159265358979323846;
    static constexpr double DEG_TO_RAD = 3.14159265358979323846 / 180.0;
    static constexpr double QUARTER_ARC = 3.14159265358979323846 / 2.0;

    //
	// PSMatrix provides a 2D affine transformation matrix
    struct PSMatrix {
        // Where is the system level PI we can rely on?


        // Postscript matrix representation is:
        // | m00 m01 0 |
        // | m10 m11 0 |
        // | m20 m21 1 |
        // We will use a 1D array to represent this matrix, where the
        // elements are stored in row-major order, omitting the last column.
        
        union {
            double m[6]; // for easier access
            struct {
                double m00, m01, m10, m11, m20, m21;
            };
            struct {
                double a, b, c, d, e, f;
            };

        };
        //double m[6]; // m00 m01 m10 m11 m20 m21

		//===========================================
		// Constructors
		//===========================================
        // default constructor makes an Identity matrix
        constexpr PSMatrix()
            : m{ 1, 0, 0, 1, 0, 0 } {
        } 

        constexpr PSMatrix(double m00, double m01, double m10,
            double m11, double m20, double m21)
            : m{ m00, m01, m10, m11, m20, m21 } {
        }

        // Reset the matrix to Identity
        constexpr void reset() {
            m[0] = 1; m[1] = 0; 
            m[2] = 0; m[3] = 1; 
            m[4] = 0; m[5] = 0;
        }



		//============================================
        // Instance methods
		//============================================
        // copy constructor
        PSMatrix clone() const {
            return PSMatrix(m[0], m[1], m[2], m[3], m[4], m[5]);
        }

        // determinant
        // Needed to create the inverse
        constexpr double determinant() const {
            return m[0] * m[3] - m[1] * m[2];
        }

        // Create the inverse of this matrix, and put it into 'out'.
        bool inverse(PSMatrix& out) const 
        {
            double d = determinant();
            if (d == 0.0) return false;

            double t00 = m[3] / d;
            double t01 = -m[1] / d;
            double t10 = -m[2] / d;
            double t11 = m[0] / d;
            double t20 = -(m[4] * t00 + m[5] * t10);
            double t21 = -(m[4] * t01 + m[5] * t11);

            out = PSMatrix(t00, t01, t10, t11, t20, t21);
            return true;
        }

        /*
        // The ChatGPT way - which is not correct, how it handles the translation part is wrong.
        constexpr PSMatrix& preMultiply(const PSMatrix& other) {
            double a = other.m[0] * m[0] + other.m[1] * m[2];
            double b = other.m[0] * m[1] + other.m[1] * m[3];
            double c = other.m[2] * m[0] + other.m[3] * m[2];
            double d = other.m[2] * m[1] + other.m[3] * m[3];
            double tx = other.m[0] * m[4] + other.m[1] * m[5] + other.m[4];
            double ty = other.m[2] * m[4] + other.m[3] * m[5] + other.m[5];

            m[0] = a; m[1] = b; m[2] = c;
            m[3] = d; m[4] = tx; m[5] = ty;

            return *this;
        }
        */

        
        constexpr PSMatrix& preMultiply(const PSMatrix& other) {
            double a = other.m[0] * m[0] + other.m[1] * m[2];
            double b = other.m[0] * m[1] + other.m[1] * m[3];
            double c = other.m[2] * m[0] + other.m[3] * m[2];
            double d = other.m[2] * m[1] + other.m[3] * m[3];
            double tx = other.m[4] * m[0] + other.m[5] * m[2] + m[4];
            double ty = other.m[4] * m[1] + other.m[5] * m[3] + m[5];

            m[0] = a; m[1] = b; m[2] = c;
            m[3] = d; m[4] = tx; m[5] = ty;

            return *this;
        }
        
        bool resetToRotation(double rads, double cx, double cy) noexcept
        {
            double cosA = std::cos(rads);
            double sinA = std::sin(rads);
            
            m[0] = cosA; m[1] = sinA;
            m[2] = -sinA; m[3] = cosA;
            m[4] = cx;
            m[5] = cy;
            return true;
        }

		//=============================================
		// Transform the matrix
		//=============================================
        
        PSMatrix& rotate(double angleDegrees) noexcept
        {
            double angle = angleDegrees * DEG_TO_RAD;
            double as = std::sin(angle);
            double ac = std::cos(angle);

            double t00 = as * m10 + ac * m00;
            double t01 = as * m11 + ac * m01;
            double t10 = ac * m10 - as * m00;
            double t11 = ac * m11 - as * m01;
            
            m00 = t00;
            m01 = t01;

            m10 = t10;
            m11 = t11;
            
            return *this;

            //PSMatrix r = makeRotation(angleDegrees);
            //return preMultiply(r);

        }
        

        constexpr PSMatrix& scale(double sx, double sy) noexcept
        {
            m00 *= sx;
            m01 *= sx;
            m10 *= sy;
            m11 *= sy;

            return *this;

            //return preMultiply(scaling(sx, sy));
        }

        PSMatrix& translate(double tx, double ty) noexcept
        {
            m20 += tx * m00 + ty * m10;
            m21 += tx * m01 + ty * m11;

            //this->e += tx;
            //this->f += ty;
            return *this;

            //PSMatrix tr = translation(tx, ty);
            //return preMultiply(tr);
        }


        //====================================================
        // Transform Points
        //====================================================
        constexpr void transformPoint(double x, double y, double& outX, double& outY) const 
        {
            outX = a * x + c * y + e;
            outY = b * x + d * y + f;

            //outX = m[0] * x + m[2] * y + m[4];
            //outY = m[1] * x + m[3] * y + m[5];
        }

        constexpr void dtransform(double x, double y, double& outX, double& outY) const {
            
            outX = a * x + c * y;
            outY = b * x + d * y;

            //outX = m[0] * x + m[2] * y;
            //outY = m[1] * x + m[3] * y;
        }

        void print() const {
            printf("%3.2f %3.2f\n%3.2f %3.2f\n%3.2f %3.2f\n",
                m[0], m[1], m[2], m[3], m[4], m[5]);
        }

        //============================================
        // Factory constructors
        // 
        //============================================
        static constexpr PSMatrix makeIdentity() 
        {
            return PSMatrix(1, 0, 0, 1, 0, 0);
        }

        static constexpr PSMatrix makeTranslation(double tx, double ty) 
        {
            return PSMatrix(1, 0, 0, 1, tx, ty);
        }

        static constexpr PSMatrix makeScaling(double sx, double sy) 
        {
            return PSMatrix(sx, 0, 0, sy, 0, 0);
        }

        static PSMatrix makeRotation(double angleDegrees) 
        {
            double rad = angleDegrees * DEG_TO_RAD;
            PSMatrix m;
            m.resetToRotation(rad, 0, 0);
        }

        static std::shared_ptr<PSMatrix> create() 
        {
            return std::shared_ptr<PSMatrix>(new PSMatrix());
        }
    };

} // namespace waavs
