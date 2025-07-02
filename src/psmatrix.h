#pragma once

#pragma once
#include <cmath>
#include <cstdio>

namespace waavs {
    //
	// PSMatrix provides a 2D affine transformation matrix
    struct PSMatrix {
        // Where is the system level PI we can rely on?
        static constexpr double PI = 3.14159265358979323846;
        static constexpr double RADIANS_PER_DEGREE = PI / 180.0;

        // Postscript matrix representation is:
        // | m00 m01 0 |
        // | m10 m11 0 |
        // | m20 m21 1 |
        // We will use a 1D array to represent this matrix, where the
        // elements are stored in row-major order, omitting the last column.
        double m[6]; // m00 m01 m10 m11 m20 m21

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

        constexpr double determinant() const {
            return m[0] * m[3] - m[1] * m[2];
        }

        // Create the inverse of this matrix, and put it into 'out'.
        bool inverse(PSMatrix& out) const {
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

		//=============================================
		// Transformations
		//=============================================
        
        PSMatrix& rotate(double angleDegrees) {
            PSMatrix r = rotation(angleDegrees);
            return preMultiply(r);
        }
        

        constexpr PSMatrix& scale(double sx, double sy) {
            return preMultiply(scaling(sx, sy));
        }

        PSMatrix& translate(double tx, double ty) {
            PSMatrix tr = translation(tx, ty);
            return preMultiply(tr);
        }



        constexpr void transformPoint(double x, double y, double& outX, double& outY) const {
            outX = m[0] * x + m[2] * y + m[4];
            outY = m[1] * x + m[3] * y + m[5];
        }

        constexpr void dtransform(double x, double y, double& outX, double& outY) const {
            outX = m[0] * x + m[2] * y;
            outY = m[1] * x + m[3] * y;
        }

        void print() const {
            printf("%3.2f %3.2f\n%3.2f %3.2f\n%3.2f %3.2f\n",
                m[0], m[1], m[2], m[3], m[4], m[5]);
        }

        //============================================
        // Factory constructors
        // 
        //============================================
        static constexpr PSMatrix identity() {
            return PSMatrix(1, 0, 0, 1, 0, 0);
        }

        static constexpr PSMatrix translation(double tx, double ty) {
            return PSMatrix(1, 0, 0, 1, tx, ty);
        }

        static constexpr PSMatrix scaling(double sx, double sy) {
            return PSMatrix(sx, 0, 0, sy, 0, 0);
        }

        static constexpr PSMatrix rotation(double angleDegrees) {
            double rad = angleDegrees * RADIANS_PER_DEGREE;
            double cosA = std::cos(rad);
            double sinA = std::sin(rad);
            return PSMatrix(cosA, sinA, -sinA, cosA, 0, 0);
        }
    };

} // namespace waavs
