#pragma once

namespace waavs {
    enum class PSPaintKind : int {
        RGB,
        GRAY,
        CMYK
    };

    struct PSPaint {
        PSPaintKind  kind{ PSPaintKind::RGB };

        union {
            struct { double r, g, b, a; };
            struct { double gray; };
            struct { double c, m, y, k; };
        };

        bool isRGB() const { return kind == PSPaintKind::RGB; }
        bool isGray() const { return kind == PSPaintKind::GRAY; }
        bool isCMYK() const { return kind == PSPaintKind::CMYK; }

        static PSPaint fromRGBA(double r, double g, double b, double a) {
            PSPaint p;
            p.kind = PSPaintKind::RGB;
            p.r = r; 
            p.g = g; 
            p.b = b; 
            p.a = a;

            return p;
        }

        static PSPaint fromRGB(double r, double g, double b) {
            return fromRGBA(r, g, b, 1.0);
        }

        static PSPaint fromGray(double gray) {
            PSPaint p;
            p.kind = PSPaintKind::GRAY;
            p.gray = gray;
            return p;
        }

        static PSPaint fromCMYK(double c, double m, double y, double k) {
            PSPaint p;
            p.kind = PSPaintKind::CMYK;
            p.c = c; 
            p.m = m; 
            p.y = y; 
            p.k = k;

            return p;
        }
    };

}