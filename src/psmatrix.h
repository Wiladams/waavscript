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

        double m[6]; // m00 m01 m10 m11 m20 m21

		//===========================================
		// Constructors
		//===========================================
        PSMatrix()
            : m{ 1, 0, 0, 1, 0, 0 } {
        } // Identity

        PSMatrix(double m00, double m01, double m10,
            double m11, double m20, double m21)
            : m{ m00, m01, m10, m11, m20, m21 } {
        }

		//============================================
        // Factory constructors
		//============================================
        static PSMatrix identity() {
            return PSMatrix(1, 0, 0, 1, 0, 0);
        }

        static PSMatrix translation(double tx, double ty) {
            return PSMatrix(1, 0, 0, 1, tx, ty);
        }

        static PSMatrix scaling(double sx, double sy) {
            return PSMatrix(sx, 0, 0, sy, 0, 0);
        }

        static PSMatrix rotation(double angleDegrees, double cx = 0, double cy = 0) {
            double rad = angleDegrees * (PI / 180.0);
            double cosA = std::cos(rad);
            double sinA = std::sin(rad);
            return PSMatrix(cosA, sinA, -sinA, cosA, cx, cy);
        }

		//============================================
        // Instance methods
		//============================================

        PSMatrix clone() const {
            return PSMatrix(m[0], m[1], m[2], m[3], m[4], m[5]);
        }

        double determinant() const {
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

        PSMatrix& preMultiply(const PSMatrix& other) {
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
        PSMatrix& rotate(double angleDegrees, double cx = 0, double cy = 0) {
            PSMatrix r = rotation(angleDegrees, cx, cy);
            return preMultiply(r);
        }

        PSMatrix& scale(double sx, double sy) {
            m[0] *= sx; m[1] *= sx;
            m[2] *= sy; m[3] *= sy;
            return *this;
        }

        PSMatrix& translate(double tx, double ty) {
            double x, y;
            transformPoint(tx, ty, x, y);
            m[4] += x;
            m[5] += y;
            return *this;
        }

        void transformPoint(double x, double y, double& outX, double& outY) const {
            outX = m[0] * x + m[2] * y + m[4];
            outY = m[1] * x + m[3] * y + m[5];
        }

        void dtransform(double x, double y, double& outX, double& outY) const {
            outX = m[0] * x + m[2] * y;
            outY = m[1] * x + m[3] * y;
        }

        void print() const {
            printf("%3.2f %3.2f\n%3.2f %3.2f\n%3.2f %3.2f\n",
                m[0], m[1], m[2], m[3], m[4], m[5]);
        }
    };

} // namespace waavs
