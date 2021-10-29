// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <cmath>
#include <algorithm>
#include <vector> 
#include <cmath>
#include <cstdlib>

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h" 

#ifndef M_2PI
    #define M_2PI (3.1415926535897932384626433 * 2.0)
#endif

#ifndef M_PI
    #define M_PI (3.1415926535897932384626433)
#endif

#include "spline.h"

namespace Graph {

    enum FollowMode {
        FollowLine  = 0,
        DoNotFollow = 1
    };

    enum PolygonMode {
        Default   = 0,
        AutoSortX = 1
    };

    bool isSame(double _p1, double _p2); 

    template <typename Xt, typename Yt> class Point {
    public:
        Point(Xt x = 0, Yt y = 0) : m_x(x), m_y(y) {};
        Point(const Point & pt) { m_x = pt.m_x; m_y = pt.m_y; }

        Point & operator = (const Point & pt) { m_x = pt.m_x; m_y = pt.m_y; return *this; };
        bool operator ==(const Point & pt) const { return ((pt.m_x == m_x) && (pt.m_y == m_y)); };

        Point & operator -(const Point & pt) { m_x -= pt.m_x; m_y -= pt.m_y; return *this; }
        Point & operator +(const Point & pt) { m_x += pt.m_x; m_y += pt.m_y; return *this; }
        Point & operator +=(const Point & pt) { m_x += pt.m_x; m_y += pt.m_y; return *this; }
        Point & operator *(double scale) { m_x = (Xt)(m_x*scale); m_y = (Yt)(m_y*scale); return *this; }
        Point & operator /(double divider) { m_x = (Xt)(m_x / divider); m_y = (Yt)(m_y / divider); return *this; }

        Point  transposed() const { return { m_y, m_x }; };

        Point &  scaleY(double scale) {
            m_y = (Yt)(m_y*scale); return *this;
        };

        Point &  scaleX(double scale) {
            m_x = (Xt)(m_x*scale); return *this;
        };

        void setX(Xt x) { m_x = x; };
        void setY(Yt y) { m_y = y; };

        Xt & rx() { return m_x; };
        Yt & ry() { return m_y; };

        Xt  x() const { return m_x; };
        Yt  y() const { return m_y; };

        double  length() const { return std::sqrt(std::pow(m_x, 2) + std::pow(m_y, 2)); };
        double  lengthTo(const Point & pt) const { return std::sqrt(std::pow(m_x - pt.m_x, 2) + std::pow(m_y - pt.m_y, 2)); }
        int     manhattanLength() const { return std::abs(m_x) + std::abs(m_y); }

        bool isNull() const { return ((m_x == 0) && (m_y == 0)); };

    protected:

        Xt m_x = 0;
        Yt m_y = 0;
    };


    template <typename Xt, typename Yt> class Line {
    public:
        Line() = default;
        Line(Xt x1, Yt y1, Xt x2, Yt y2) : m_p1(x1, y1), m_p2(x2, y2) {};
        Line(const Point<Xt, Yt> & p1, const Point<Xt, Yt> & p2) : m_p1(p1), m_p2(p2) {};
        Line(const Line & l) : m_p1(l.m_p1), m_p2(l.m_p2) {};

        Line & operator = (const Line & pt) { m_p1 = pt.m_p1; m_p2 = pt.m_p2; return *this; };
        bool operator ==(const Line & pt) const { return ((pt.m_p1 == m_p1) && (pt.m_p2 == m_p2)); };

        Point<Xt, Yt>      p1() const { return m_p1; };
        Point<Xt, Yt>      p2() const { return m_p2; };

        Point<Xt, Yt>      center() const { return (p1() + p2()) / 2; };

        Xt dx() const { return m_p2.x() - m_p1.x(); }
        Yt dy() const { return m_p2.y() - m_p1.y(); }

        double  length() const { return std::hypot(dx(), dy()); }
        double  angle() const {

            const double dx = m_p2.x() - m_p1.x();
            const double dy = m_p2.y() - m_p1.y();

            const double theta = std::atan2(-dy, dx) * 360.0 / M_2PI;

            const double    theta_normalized = theta < 0 ? theta + 360 : theta;

            if (isSame(theta_normalized, 360.0))
                return 0.0;
            else
                return theta_normalized;
        }

        void    setAngle(double angle) {
            const double angleR = angle * M_2PI / 360.0;
            const double l = length();

            const double dx = std::cos(angleR) * l;
            const double dy = -std::sin(angleR) * l;

            m_p2.rx() = (int)(m_p1.x() + dx);
            m_p2.ry() = (int)(m_p1.y() + dy);
        }

        Point<Xt, Yt>      pointAtX(Xt x, FollowMode follow = DoNotFollow) {
            if ((follow == DoNotFollow) && (x > std::max(m_p1.x(), m_p2.x())))
                return Point<Xt, Yt>();

            if ((follow == DoNotFollow) && (x < std::min(m_p1.x(), m_p2.x())))
                return Point<Xt, Yt>();


            Yt yP = (Yt)((double)((x - m_p1.x()) * (m_p2.y() - m_p1.y())) / (double)(m_p2.x() - m_p1.x()) + m_p1.y());

            return Point<Xt, Yt>(x, yP);
        }

        Point<Xt, Yt>      pointAtY(Yt y, FollowMode follow = DoNotFollow) {
            if ((follow == DoNotFollow) && (y > std::max(m_p1.y(), m_p2.y())))
                return Point<Xt, Yt>();

            if ((follow == DoNotFollow) && (y < std::min(m_p1.y(), m_p2.y())))
                return Point<Xt, Yt>();


            Xt xP = (Xt)((double)((y - m_p1.y()) * (m_p2.x() - m_p1.x())) / (double)(m_p2.y() - m_p1.y()) + m_p1.x());

            return Point<Xt, Yt>(xP, y);
        }

        Xt        x1() const { return m_p1.x(); };
        Yt        y1() const { return m_p1.y(); };
        Xt        x2() const { return m_p2.x(); };
        Yt        y2() const { return m_p2.y(); };

        void       setP1(const Point<Xt, Yt> & p1) { m_p1 = p1; };
        void       setP2(const Point<Xt, Yt> & p2) { m_p2 = p2; };

        bool       isNull() const { return (m_p1.isNull() && m_p2.isNull()); };
        void       setLine(Xt x1, Yt y1, Xt x2, Yt y2) { m_p1 = { x1,y1 }; m_p2 = { x2,y2 }; };
        void       setPoints(const Point<Xt, Yt> & p1, const Point<Xt, Yt> & p2) { m_p1 = p1; m_p2 = p2; };

        void	   translate(const Point<Xt, Yt> & offset) { m_p1 += offset; m_p2 += offset; };
        void	   translate(Xt dx, Yt dy) { m_p1 += {dx, dy}; m_p2 += {dx, dy}; };

    protected:

        Point<Xt,Yt>   m_p1;
        Point<Xt,Yt>   m_p2;
    };

    template <typename Xt, typename Yt> class Rect {
    public:
        Rect() = default;
        Rect(Xt x1, Yt y1, Xt x2, Yt y2) {
            m_p1 = { std::min(x1,x2), std::max(y1,y2) };
            m_p2 = { std::max(x1,x2), std::min(y1,y2) };
        }

        Rect(const Point<Xt, Yt> & p1, const Point<Xt, Yt> & p2) {
            m_p1 = { std::min(p1.x(),p2.x()), std::max(p1.y(),p2.y()) };
            m_p2 = { std::max(p1.x(),p2.x()), std::min(p1.y(),p2.y()) };
        }

        Rect(const Rect<Xt, Yt> & l) : m_p1(l.m_p1), m_p2(l.m_p2) {};

        Rect<Xt, Yt> & operator = (const Rect<Xt, Yt> & pt) { m_p1 = pt.m_p1; m_p2 = pt.m_p2; return *this; };
        bool operator ==(const Rect & pt) const { return ((pt.m_p1 == m_p1) && (pt.m_p2 == m_p2)); };


        Xt width() const  { std::abs(m_p1.x() - m_p2.x()); }
        Yt height() const { std::abs(m_p1.y() - m_p2.y()); }


        Point<Xt, Yt> center() const {
            return Point((m_p2.x() - m_p1.x()) / 2, (m_p1.y() - m_p2.y()) / 2);
        }

        Point<Xt, Yt> topLeft() const {
            return m_p1;
        }

        Point<Xt, Yt> topRight() const {
            return { m_p2.x(), m_p1.y() };
        }

        Point<Xt, Yt> bottomLeft() const {
            return { m_p1.x(), m_p2.y() };
        }

        Point<Xt, Yt> bottomRight() const {
            return m_p2;
        }

    protected:

        Point<Xt, Yt>   m_p1;
        Point<Xt, Yt>   m_p2;
    };

    template <typename Xt, typename Yt> class Polyline {
    public:
        Polyline(PolygonMode m = Default):m_mode(m) {};
        Polyline(const std::vector<Point<Xt, Yt> > & points) : m_points(points) { invalidateBoundingRect(); };
        Polyline(const Polyline<Xt, Yt> & p) {
            m_points = p.m_points; m_mode = p.m_mode; m_maxx = p.m_maxx; m_maxy = p.m_maxy; m_minx = p.m_minx; m_miny = p.m_miny;
        };

        Polyline<Xt, Yt> & operator =(const Polyline<Xt, Yt> & p) { m_points = p.m_points; m_mode = p.m_mode; m_maxx = p.m_maxx; m_maxy = p.m_maxy; m_minx = p.m_minx; m_miny = p.m_miny; return *this; };
        bool      operator==(const Polyline<Xt, Yt> & p) { return (m_points == p.m_points); };
         
        Polyline<Xt, Yt> & operator<<(const Point<Xt, Yt> & p) {
            m_points.push_back(p);
            
            if (p.x() < m_minx) m_minx = p.x();
            if (p.y() < m_miny) m_miny = p.y();
            if (p.x() > m_maxx) m_maxx = p.x();
            if (p.y() > m_maxy) m_maxy = p.y();

            if (m_mode == AutoSortX) sortByX();
            return *this;
        }

        Line<Xt, Yt>        firstSegment() const {
            return segment(0);
        }

        Line<Xt, Yt>        lastSegment() const {
            return segment(count() - 1);
        }

        int         count() const {
            return m_points.size() > 1 ? m_points.size() - 1 : 0;
        }

        Polyline<Xt, Yt> &  scaleY(double scale) {
            for (auto & pt : m_points) pt.scaleY(scale);

            invalidateBoundingRect();
            return *this;
        }

        void       forEach(std::function<bool(Point<Xt, Yt> & pt)> f) {
            for (auto & pt : m_points) {
                if (!f(pt)) break;
            }
            
            invalidateBoundingRect(); 
        }

        void       forEach(std::function<bool(const Point<Xt, Yt> & pt)> f) const {
            for (const auto & pt : m_points) if (!f(pt)) return;
        }

        Line<Xt, Yt>        segmentAtX(Xt x) const {
            bool bFirst = true;
            Point<Xt, Yt>   prevPoint;
            for (const Point<Xt, Yt> & pt : m_points) {
                if (bFirst) {
                    if (pt.x() > x)
                        return firstSegment();

                    prevPoint = pt;
                    bFirst = false;
                    continue;
                }

                if (pt.x() > x) {
                    return Line<Xt, Yt>(prevPoint, pt);
                }

                prevPoint = pt;
            }

            return lastSegment();
        }

        Rect<Xt, Yt>     boundingRect() const {           
            return { m_minx, m_maxy, m_maxx, m_miny };
        }

        void        setPoints(const std::vector<Point<Xt, Yt> > & points) {
            m_points = points;
            if (m_mode == AutoSortX) sortByX();
            invalidateBoundingRect();
        };
        
        double      length() const {
            double l = 0;
            bool bFirst = true;
            Point<Xt, Yt>   prevPoint;
            for (const Point<Xt, Yt> & pt : m_points) {
                if (bFirst) {
                    prevPoint = pt;
                    bFirst = false;
                    continue;
                }

                l += pt.lengthTo(prevPoint);
                prevPoint = pt;
            }

            return l;
        }

        void        sortByX() {
            std::stable_sort(m_points.begin(), m_points.end(), [](const Point<Xt, Yt> & p1, const Point<Xt, Yt> & p2) { return p1.x() < p2.x(); });
        }

        Line<Xt, Yt>        segment(int index) const {
            if (index >= count())
                return Line<Xt, Yt>();

            return Line<Xt, Yt>(m_points[index], m_points[index + 1]);
        }

        const std::vector<Point<Xt, Yt> > & points() const { return m_points; };
              
        void  clear() { 
            m_points.clear(); 
            invalidateBoundingRect();
        };

    protected:

        void    invalidateBoundingRect() {
            m_minx = 0;
            m_maxx = 0;
            m_miny = 0;
            m_maxy = 0;

            bool bFirst = true;

            for (const Point<Xt, Yt> & pt : m_points) {
                if (bFirst) {
                    m_minx = pt.x();
                    m_maxx = pt.x(); //-V656
                    m_miny = pt.y();
                    m_maxy = pt.y(); //-V656
                    bFirst = false;
                    continue;
                }

                if (pt.x() < m_minx) m_minx = pt.x();
                if (pt.y() < m_miny) m_miny = pt.y();
                if (pt.x() > m_maxx) m_maxx = pt.x();
                if (pt.y() > m_maxy) m_maxy = pt.y();
            }

        }

    protected:

        std::vector<Point<Xt, Yt>>   m_points;

        Xt m_minx = 0, m_maxx = 0;
        Yt m_miny = 0, m_maxy = 0;
       
        PolygonMode          m_mode = Default;    
    };


    class Spline {
        public:
                       
            Spline();
            Spline(const Polyline<double, double> & polyline);
            Spline(const Spline & other) { m_spline = other.m_spline; }           
            Spline & operator = (const Spline & other) { m_spline = other.m_spline; return *this; }
            ~Spline() = default;

            void    setPolyline(const Polyline<double, double> & polyline);

            double  value(double x) const;
            double  solve(double y) const;

        protected:
            
            void    calculate(const Polyline<double, double> & polyline);

        protected:
            
            tk::spline      m_spline;
    };
    
    template <typename Xt, typename Yt> void              dump(const Polyline<Xt, Yt> & p, const std::string & header = std::string(), const std::string & format = "{:03.3f} -> {:03.3f}" ) {
        if (header.size()) {
            spdlog::info("{}", header);
        }

        for (auto const & pt : p.points()) {
            spdlog::info(format.c_str(), pt.x(), pt.y());
        }

    }

    template <typename Xt, typename Yt> double            barSquare(const Polyline<Xt, Yt> & p, Yt baseY = 0) {
        double sq = 0;
       
        bool bFirst = true;
        Point<Xt, Yt>   prevPoint;
        for (const Point<Xt, Yt> & pt : p.points()) {
            if (bFirst) {
                prevPoint = pt;
                bFirst = false;
                continue;
            }

            sq += (prevPoint.y() - baseY)*(pt.x() - prevPoint.x());

            prevPoint = pt;
        }

        return sq;
    }



    template <typename Xt, typename Yt> double            square(const Polyline<Xt, Yt> & p, Yt baseY = 0) {
        double sq = 0;
        Yt baseYPrecalc = baseY * 2;
        bool bFirst = true;
        Point<Xt, Yt>   prevPoint;
        for (const Point<Xt, Yt> & pt : p.points()) {
            if (bFirst) {
                prevPoint = pt;
                bFirst = false;
                continue;
            }

            sq += (prevPoint.y() + pt.y() - baseYPrecalc)*(pt.x() - prevPoint.x()) / 2;

            prevPoint = pt;
        }

        return sq;
    }

 
    template <typename Xt, typename Yt> Polyline<Xt, Yt> & sum( Polyline<Xt, Yt> & dst, const Polyline<Xt, Yt> & src, std::function<Yt(Yt v)> filter = nullptr) {
        Rect<Xt, Yt>     dstBoundingRect = dst.boundingRect();
        Rect<Xt, Yt>     srcBoundingRect = src.boundingRect();

        Xt  dstMinX = dstBoundingRect.bottomLeft().x();
        Xt  dstMaxX = dstBoundingRect.bottomRight().x();

        Xt  srcMinX = srcBoundingRect.bottomLeft().x();
        Xt  srcMaxX = srcBoundingRect.bottomRight().x();

        dst.forEach([&src, srcMinX, srcMaxX, &filter](Graph::Point<Xt, Yt> & pt) {
            if ((pt.x() < srcMinX) || (pt.x() > srcMaxX))
                return true;


            Yt srcY = src.segmentAtX(pt.x()).pointAtX(pt.x()).y();            
            srcY = filter ? filter(srcY) : srcY;

            pt.ry() += srcY;

            return true;
        });

        src.forEach([&dst, dstMinX, dstMaxX, &filter](const Graph::Point<Xt, Yt> & pt) {
            if ((pt.x() < dstMinX) || (pt.x() > dstMaxX)) {
                Yt srcY = pt.y();
                srcY = filter ? filter(srcY) : srcY;

                dst << Graph::Point<Xt,Yt>(pt.x(), srcY);
            }

            return true;
        });

        return dst;
    }
};



