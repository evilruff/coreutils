// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include "points.h"

#include <cmath>
#include <cstdlib>

bool Graph::isSame(double _p1, double _p2) {
      return ((std::fabs(_p1 - _p2) * 1000000000000.) <= (std::min(std::fabs(_p1), std::fabs(_p2))));
}


Graph::Spline::Spline(const Graph::Polyline<double, double> & polyline) {
    calculate(polyline);
}

void    Graph::Spline::setPolyline(const Polyline<double, double> & polyline) {
    calculate(polyline);
}

Graph::Spline::Spline() {
    m_spline = tk::spline();
}

double  Graph::Spline::value(double x) const {
    return m_spline(x);
}

double  Graph::Spline::solve(double y) const {
    std::vector<double> x = m_spline.solve(y, false);
    if (x.size())
        return x[0];

    return 0.0;
}

void    Graph::Spline::calculate(const Polyline<double, double> & polyline) {
    std::vector<double> x;
    std::vector<double> y;

    polyline.forEach([&x, &y](const Graph::Point<double, double> & pt) {
        x.push_back(pt.x());
        y.push_back(pt.y());
        return true;
    });

    m_spline = tk::spline(x, y);
}


