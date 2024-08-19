#include <iostream>
#include <functional>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "GridMap.h"
#include "Sweeping.h"

struct CircumcircleTest {
    std::vector<tora::sim::fortune::Point> sites;
    tora::sim::fortune::Circle cc{};

    void render(sf::RenderWindow& window) {
        auto circle = sf::CircleShape(cc.radius);
        circle.setPosition(sf::Vector2f(cc.origin.x - cc.radius, cc.origin.y - cc.radius));
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineColor(sf::Color::White);
        circle.setOutlineThickness(2);
        window.draw(circle);

        for (int i = 0; i < 3; i++) {
            auto v = sf::RectangleShape(sf::Vector2f(5, 5));
            v.setPosition(sf::Vector2f(sites[i].x, sites[i].y));
            window.draw(v);
        }
    }

    static CircumcircleTest create() {
        CircumcircleTest ct;

        auto random = tora::Random();
        for (int i = 0; i < 3; i++) {
            double x = random.getRandomBetween<double>(100, 700);
            double y = random.getRandomBetween<double>(100, 700);
            ct.sites.push_back(tora::sim::fortune::Point(x, y));
        }
        ct.cc = tora::sim::fortune::circumcircle(ct.sites[0], ct.sites[1], ct.sites[2]);

        return ct;
    }
};

struct BreakpointTest {
    std::vector<tora::sim::fortune::Point> sites;
    tora::sim::fortune::Point breakpoint{};
    tora::sim::fortune::Circle cc{};
    double sweepLineY = 0;

    void render(sf::RenderWindow& window) {

        auto newSite = sf::Mouse::getPosition(window);
        sweepLineY = newSite.y;
        breakpoint = tora::sim::fortune::breakpoint(sites[0], sites[1], sweepLineY);

        for (int i = 0; i < 2; i++) {
            auto v = sf::CircleShape(3);
            v.setPosition(sites[i].x - 3, sites[i].y - 3);
            window.draw(v);
        }

        auto x = sf::RectangleShape(sf::Vector2f(6, 6));
        x.setPosition(breakpoint.x - 3, breakpoint.y - 3);
        window.draw(x);

        auto l = sf::RectangleShape(sf::Vector2f(800, 2));
        l.setPosition(0, sweepLineY);
        window.draw(l);

        auto r = abs(sweepLineY - breakpoint.y);
        auto c = sf::CircleShape(r);
        c.setPosition(breakpoint.x - r, breakpoint.y - r);
        c.setFillColor(sf::Color::Transparent);
        c.setOutlineColor(sf::Color::White);
        c.setOutlineThickness(1);
        window.draw(c);
    }
    

    static BreakpointTest create() {
        BreakpointTest t;

        auto random = tora::Random();

        t.sweepLineY = random.getRandomBetween<double>(400, 700);
        for (int i = 0; i < 2; i++) {
            double x = random.getRandomBetween<double>(100, 700);
            double y = random.getRandomBetween<double>(100, t.sweepLineY);
            t.sites.push_back(tora::sim::fortune::Point(x, y));
        }
        // t.breakpoint = tora::sim::fortune::breakpoint(t.sites[0], t.sites[1], t.sweepLineY);

        // std::cout << t.breakpoint.x << ", " << t.breakpoint.y << std::endl;

        return t;
    }
};

struct ParabolaTest {
    tora::sim::fortune::Point site{};

    void render(sf::RenderWindow& window) {

        auto newSite = sf::Mouse::getPosition(window);
        auto intersection = tora::sim::fortune::parabolaIntersect(site, tora::sim::fortune::Point{ (double)newSite.x, (double)newSite.y });

        auto v = sf::CircleShape(3);
        v.setPosition(site.x - 3, site.y - 3);
        window.draw(v);

        auto x = sf::RectangleShape(sf::Vector2f(6, 6));
        x.setPosition(intersection.x - 3, intersection.y - 3);
        window.draw(x);

        auto l = sf::RectangleShape(sf::Vector2f(800, 2));
        l.setPosition(0, newSite.y);
        window.draw(l);

        auto r = abs(newSite.y - intersection.y);
        auto c = sf::CircleShape(r);
        c.setPosition(intersection.x - r, intersection.y - r);
        c.setFillColor(sf::Color::Transparent);
        c.setOutlineColor(sf::Color::White);
        c.setOutlineThickness(1);
        window.draw(c);
    }


    static ParabolaTest create() {
        ParabolaTest t;

        auto random = tora::Random();

        double x = random.getRandomBetween<double>(100, 700);
        double y = random.getRandomBetween<double>(50, 400);

        t.site.x = x;
        t.site.y = y;

        return t;
    }
};

struct BeachlineTest {
    std::list<tora::sim::fortune::Arc> beachline;

    void render(sf::RenderWindow& window) {

        auto newSite = sf::Mouse::getPosition(window);
        auto breakpoints = tora::sim::fortune::breakpoints(beachline, newSite.y);
        auto arcAbove = findArcAbove(beachline, tora::sim::fortune::Point(newSite.x, newSite.y));

        for (const auto& arc : beachline) {
            auto v = sf::CircleShape(5);
            v.setPosition(arc.location.x - 5, arc.location.y - 5);
            if (arc.site == arcAbove->site) {
                v.setFillColor(sf::Color::Green);
            }
            else {
                v.setFillColor(sf::Color::White);
            }
            window.draw(v);
        }

        for (int i = 0; i < breakpoints.size(); i++) {
            auto v = sf::CircleShape(2);
            v.setPosition(breakpoints[i].x - 2, breakpoints[i].y - 2);
            window.draw(v);
        }

        auto l = sf::RectangleShape(sf::Vector2f(800, 2));
        l.setPosition(0, newSite.y);
        window.draw(l);
    }


    static BeachlineTest create(int numSites) {
        BeachlineTest t;

        auto random = tora::Random();

        double approxStep = (700 - 100) / numSites;

        double x = 75 + approxStep * random.getRandomBetween<double>(0.5, 1.5);
        double y = 250 + approxStep * (random.getRandomBetween<double>(0, 2.0) - 1.0);
        for (int i = 0; i < numSites; i++) {
            t.beachline.push_back(tora::sim::fortune::Arc{
                .id = -1,
                .site = i,
                .location = tora::sim::fortune::Point(x, y),
            });
            x += approxStep * random.getRandomBetween<double>(0.5, 1.5);
            y += approxStep * (random.getRandomBetween<double>(0, 2.0) - 1.0);
        }

        return t;
    }
};

struct GenerateSitesTest {
    std::vector<tora::sim::fortune::Site> sites;

    void render(sf::RenderWindow& window) {

        auto sweepLineY = sf::Mouse::getPosition(window).y;

        for (int i = 0; i < sites.size(); i++) {
            auto v = sf::CircleShape(5);
            v.setPosition(sites[i].location.x - 5, sites[i].location.y - 5);
            v.setFillColor(sf::Color::White);
            window.draw(v);
        }

        auto l = sf::RectangleShape(sf::Vector2f(800, 2));
        l.setPosition(0, sweepLineY);
        window.draw(l);
    }


    static GenerateSitesTest create(int numSites) {
        GenerateSitesTest t;

        auto random = tora::Random();

        for (int i = 0; i < numSites; i++) {
            double x = random.getRandomBetween<double>(100, 700);
            double y = random.getRandomBetween<double>(100, 700);
            t.sites.push_back(tora::sim::fortune::Site{
                .id = i,
                .location = tora::sim::fortune::Point(x, y),
            });
        }

        std::sort(t.sites.begin(), t.sites.end(), [](const auto& a, const auto& b) {
            return a.location.y < b.location.y;
            });

        for (int i = 0; i < t.sites.size(); i++) {
            t.sites[i].id = i;
        }

        return t;
    }
};

struct SweepingTest {
    tora::sim::fortune::State algorithm;

    SweepingTest(const std::vector<tora::sim::fortune::Site>& sites) : algorithm{ sites } {}

    void step() {
        if (!algorithm.step()) {
            std::cout << "Algorithm finished.\n";
        }
    }

    void render(sf::RenderWindow& window, sf::Font& font) {
        auto sweepLineY = algorithm.sweepLineY;
        auto mouse = sf::Mouse::getPosition(window);
        auto mousePoint = tora::sim::fortune::Point(mouse.x, mouse.y);
        auto siteAboveMouseId = -1;
        auto siteCloseToMouseId = -1;

        for (const auto& site : algorithm.sites) {
            if (tora::geometry::distSqr(site.location, mousePoint) < 9) {
                siteCloseToMouseId = site.id;
            }
        }

        //// Debug: draw breakpoints
        //if (!algorithm.beachline.empty()) {
        //    siteAboveMouseId = findArcAbove(algorithm.beachline, mousePoint)->site;
        //    auto breakpoints = tora::sim::fortune::breakpoints(algorithm.beachline, mouse.y);
        //    for (const auto& bp : breakpoints) {
        //        auto v = sf::CircleShape(2);
        //        v.setFillColor(sf::Color::White);
        //        v.setPosition(bp.x - 2, bp.y - 2);
        //        window.draw(v);
        //    }
        //}

        for (const auto& segment : algorithm.segments) {
            if (segment.finished) {
                sf::Vertex line[2];
                sf::Color lineColor = sf::Color::White;
                if (siteCloseToMouseId != -1 && (siteCloseToMouseId == segment.site1 || siteCloseToMouseId == segment.site2)) {
                    lineColor = sf::Color::Green;
                }
                line[0].position = sf::Vector2f(segment.a.x, segment.a.y);
                line[0].color = lineColor;
                line[1].position = sf::Vector2f(segment.b.x, segment.b.y);
                line[1].color = lineColor;
                window.draw(line, 2, sf::Lines);
            }
        }

        for (const auto& site : algorithm.sites) {
            auto v = sf::CircleShape(3);
            v.setPosition(site.location.x - 3, site.location.y - 3);
            if (site.id == siteAboveMouseId) {
                v.setFillColor(sf::Color::Red);
            }
            else {
                v.setFillColor(sf::Color::White);
            }
            window.draw(v);

            //// Debug: draw site ID
            //auto idText = sf::Text();
            //idText.setFont(font);
            //idText.setFillColor(sf::Color::Magenta);
            //idText.setCharacterSize(20);
            //idText.setString(sf::String(std::to_string(site.id)));
            //idText.setPosition(sf::Vector2f(site.location.x, site.location.y));
            //window.draw(idText);
        }

        for (const auto& vert : algorithm.voronoiVertices) {
            auto v = sf::CircleShape(2);
            v.setPosition(vert.x - 2, vert.y - 2);
            v.setFillColor(sf::Color::Green);
            window.draw(v);
        }

        //// Debug: draw midpoints (where site hits arc above)
        //for (const auto& vert : algorithm.midPoints) {
        //    auto v = sf::CircleShape(2);
        //    v.setPosition(vert.x - 2, vert.y - 2);
        //    v.setFillColor(sf::Color::Red);
        //    window.draw(v);
        //}

        for (const auto& ev : algorithm.events) {
            if (ev.y <= sweepLineY) continue;
            if (ev.type == tora::sim::fortune::Event::VERTEX && ev.active) {
                auto evl = sf::RectangleShape(sf::Vector2f(800, 2));
                evl.setPosition(0, ev.y);
                evl.setFillColor(ev.active ? sf::Color(180, 120, 255) : sf::Color(200, 200, 200));
                window.draw(evl);
            }
        }

        auto l = sf::RectangleShape(sf::Vector2f(800, 2));
        l.setPosition(0, sweepLineY);
        window.draw(l);

        auto dl = sf::RectangleShape(sf::Vector2f(800, 2));
        dl.setFillColor(sf::Color::Green);
        dl.setPosition(0, mouse.y);
        window.draw(dl);

    }

    static SweepingTest create(int numSites) {
        
        std::vector<tora::sim::fortune::Site> sites;
        auto random = tora::Random();

        for (int i = 0; i < numSites; i++) {
            double x = random.getRandomBetween<double>(150, 650);
            double y = random.getRandomBetween<double>(150, 650);
            sites.push_back(tora::sim::fortune::Site{
                .id = i,
                .location = tora::sim::fortune::Point(x, y),
                });
        }

        // draw boundary circle
        const double pi = 3.14159265;
        int numSides = 12;
        double step = 2 * pi / numSides;
        double radius = 650;
        sf::Vector2 origin(400, 400);
        double theta = 0;
        for (int i = 0; i < numSides; i++, theta += step) {
            double x = cos(theta) * radius + origin.x;
            double y = sin(theta) * radius + origin.y;
            sites.push_back(tora::sim::fortune::Site{
                .id = 0,
                .location = tora::sim::fortune::Point(x, y)
            });
        }

        std::sort(sites.begin(), sites.end(), [](const auto& a, const auto& b) {
            return a.location.y < b.location.y;
            });

        for (int i = 0; i < sites.size(); i++) {
            sites[i].id = i;
        }

        return SweepingTest{ sites };
    }

    static SweepingTest reset(const SweepingTest& old) {
        return SweepingTest{ old.algorithm.sites };
    }
};

struct DivisionTest {
    tora::sim::fortune::State algorithm;
    std::vector<tora::geometry::Polygon> polygons;

    DivisionTest(const std::vector<tora::sim::fortune::Site>& sites) : algorithm{ sites } {
        algorithm.run();
        auto voronoiPolygons = algorithm.getPolygons();
        for (const auto& p : voronoiPolygons) {
            polygons.push_back(tora::geometry::offset(p, -8.0));
        }
    }

    void renderPolygon(sf::RenderWindow& window, const auto& polygon, sf::Color lineColor) {
        for (int i = 0; i < polygon.vertices.size(); i++) {
            auto v1 = polygon.vertices[i];
            auto v2 = polygon.vertices[(i + 1) % polygon.vertices.size()];
            sf::Vertex line[2];
            line[0].position = sf::Vector2f(v1.x, v1.y);
            line[0].color = lineColor;
            line[1].position = sf::Vector2f(v2.x, v2.y);
            line[1].color = lineColor;
            window.draw(line, 2, sf::Lines);
        }
    }

    void render(sf::RenderWindow& window) {
        auto mouse = sf::Mouse::getPosition(window);
        auto mousePoint = tora::geometry::Point(mouse.x, mouse.y);
        auto siteCloseToMouseId = -1;

        for (const auto& site : algorithm.sites) {
            auto v = sf::CircleShape(3);
            v.setPosition(site.location.x - 3, site.location.y - 3);
            v.setFillColor(sf::Color::White);
            window.draw(v);
        }

        for (const auto& polygon : polygons) {
            if (polygon.contains(mousePoint)) {
                // renderPolygon(window, polygon, sf::Color::Green);
            }
            else {
                renderPolygon(window, polygon, sf::Color::White);
            }
        }

        for (const auto& polygon : polygons) {
            if (polygon.contains(mousePoint)) {
                renderPolygon(window, polygon, sf::Color::Green);
            }
            else {
                // renderPolygon(window, polygon, sf::Color::White);
            }
        }
    }

    static DivisionTest create(int numSites) {

        std::vector<tora::sim::fortune::Site> sites;
        auto random = tora::Random();

        for (int i = 0; i < numSites; i++) {
            double x = random.getRandomBetween<double>(150, 650);
            double y = random.getRandomBetween<double>(150, 650);
            sites.push_back(tora::sim::fortune::Site{
                .id = i,
                .location = tora::sim::fortune::Point(x, y),
                });
        }

        // draw boundary circle
        const double pi = 3.14159265;
        int numSides = 12;
        double step = 2 * pi / numSides;
        double radius = 650;
        sf::Vector2 origin(400, 400);
        double theta = 0;
        for (int i = 0; i < numSides; i++, theta += step) {
            double x = cos(theta) * radius + origin.x;
            double y = sin(theta) * radius + origin.y;
            sites.push_back(tora::sim::fortune::Site{
                .id = 0,
                .location = tora::sim::fortune::Point(x, y)
                });
        }

        std::sort(sites.begin(), sites.end(), [](const auto& a, const auto& b) {
            return a.location.y < b.location.y;
            });

        for (int i = 0; i < sites.size(); i++) {
            sites[i].id = i;
        }

        return DivisionTest{ sites };
    }
};


struct PolygonShrinkTest {
    tora::geometry::Polygon original;
    tora::geometry::Polygon transformed;
    int numSegmentsToRender = 0;

    void renderPolygon(sf::RenderWindow& window, const auto& polygon, sf::Color lineColor, int limit, bool showVerts) {
        for (int i = 0; i < std::min(limit, (int)polygon.vertices.size()); i++) {
            auto v1 = polygon.vertices[i];
            auto v2 = polygon.vertices[(i + 1) % polygon.vertices.size()];

            sf::Vertex line[2];
            line[0].position = sf::Vector2f(v1.x, v1.y);
            line[0].color = lineColor;
            line[1].position = sf::Vector2f(v2.x, v2.y);
            line[1].color = lineColor;
            window.draw(line, 2, sf::Lines);

            if (showVerts) {
                auto vert1 = sf::CircleShape(2);
                vert1.setPosition(v1.x - 2, v1.y - 2);
                vert1.setFillColor(lineColor);
                window.draw(vert1);

                auto vert2 = sf::CircleShape(2);
                vert2.setPosition(v2.x - 2, v2.y - 2);
                vert2.setFillColor(lineColor);
                window.draw(vert2);
            }
        }
    }

    void render(sf::RenderWindow& window, sf::Font& font) {
        auto mouse = sf::Mouse::getPosition(window);
        auto mousePoint = tora::geometry::Point(mouse.x, mouse.y);

        renderPolygon(window, original, sf::Color::White, original.vertices.size(), false);
        renderPolygon(window, transformed, sf::Color::Green, numSegmentsToRender, true);

        // Debug: draw cursor position
        auto text = sf::Text();
        text.setFont(font);
        text.setFillColor(sf::Color::White);
        text.setCharacterSize(16);
        text.setString(sf::String(std::to_string(mousePoint.x) + "," + std::to_string(mousePoint.y)));
        text.setPosition(sf::Vector2f(mousePoint.x + 15.0, mousePoint.y + 10.0));
        window.draw(text);
    }

    void reset() {
        numSegmentsToRender = 0;
    }

    void step() {
        numSegmentsToRender++;
        int lastIndex = std::min(numSegmentsToRender, (int)transformed.vertices.size()) - 1;
        int nextIndex = (lastIndex + 1) % transformed.vertices.size();
        std::cout << "(" << transformed.vertices[lastIndex].x << ", " << transformed.vertices[lastIndex].y << "), ("
            << transformed.vertices[nextIndex].x << ", " << transformed.vertices[nextIndex].y << ")\n";
    }

    static PolygonShrinkTest create(int verts) {
        PolygonShrinkTest pst;

        // draw boundary circle
        const double pi = 3.14159265;
        int numSides = verts;
        double step = -2 * pi / numSides;
        double radius = 250;
        sf::Vector2 origin(400, 400);
        double theta = 0;
        for (int i = 0; i < numSides; i++, theta += step) {
            double x = cos(theta) * radius + origin.x;
            double y = sin(theta) * radius + origin.y;
            pst.original.vertices.push_back(tora::geometry::Point(x, y));
        }

        pst.transformed = tora::geometry::offset(pst.original, 75.0);

        return pst;
    }
};


int main(int argc, char** argv) {
    sf::RenderWindow window(sf::VideoMode(800, 800), "SFML works!");

    // auto map = tora::sim::TriangulationGridMap{};
    // int actionIndex = 0;

    sf::Font font;
    if (!font.loadFromFile("C://Users/storm/Downloads/Open_Sans/static/OpenSans-Light.ttf")) {
        std::cerr << "error loading OpenSans-Light.ttf" << std::endl;
        return 1;
    }

    //CircumcircleTest ct = CircumcircleTest::create();
    //ParabolaTest pt = ParabolaTest::create();
    //BreakpointTest bt = BreakpointTest::create();
    //BeachlineTest blt = BeachlineTest::create(8);
    //GenerateSitesTest gt = GenerateSitesTest::create(8);
    //SweepingTest st = SweepingTest::create(16);
    //st.algorithm.run();
    DivisionTest dt = DivisionTest::create(16);
    PolygonShrinkTest pst = PolygonShrinkTest::create(7);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                //if (sf::Keyboard::isKeyPressed(sf::Keyboard::G))
                //{
                //    map.buildGrid(actionIndex % 10, actionIndex / 10);
                //    actionIndex++;
                //}

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::G))
                {
                    // ct = CircumcircleTest::create();
                    // pt = ParabolaTest::create();
                    // bt = BreakpointTest::create();
                    // blt = BeachlineTest::create(8);
                    // gt = GenerateSitesTest::create(8);
                    // st = SweepingTest::create(16);
                    // st.algorithm.run();
                    dt = DivisionTest::create(16);

                    // pst = PolygonShrinkTest::create(7);
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
                {
                    // st = SweepingTest::reset(st);
                    // pst.reset();
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::N))
                {
                    // st.step();
                    // pst.step();
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                {
                    // st.algorithm.save("savedstate.txt");
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::L))
                {
                    //auto savedState = tora::sim::fortune::State::load("savedstate.txt");
                    //if (savedState.has_value()) {
                    //    st = SweepingTest{ savedState.value().sites };
                    //}
                }
            }
        }

        window.clear();
        
        // ct.render(window);
        // pt.render(window);
        // bt.render(window);
        // blt.render(window);
        // gt.render(window);
        // st.render(window, font);
        dt.render(window);
        // pst.render(window, font);

        // map.render(window);
        window.display();
    }

    return 0;
}