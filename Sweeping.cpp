#include "Sweeping.h"

#include <iostream>
#include <fstream>

namespace tora::sim::fortune {

std::ostream& operator<<(std::ostream& os, const ArcRef& ar) {
    os << ar->id << "(" << std::prev(ar)->site << "," << ar->site << "," << std::next(ar)->site << ")";
    return os;
}

bool State::checkVertexEvent(ArcRef arc, double sweepLineY)
{
    clearVertexEvent(arc);

    if (arc == beachline.begin() || std::next(arc) == beachline.end()) {
        return false;
    }

    auto prev = std::prev(arc);
    auto next = std::next(arc);

    if (prev->site == next->site) {
        return false;
    }

    auto cc = circumcircle(prev->location, arc->location, next->location);
    auto lp = lowestPoint(cc);

    if (lp.y < sweepLineY) {
        return false;
    }

    std::cout << "adding vertex event on arc: " << arc << "; lowest y: " << lp.y << ", sweepline y: " << sweepLineY << ".\n";

    int eventIndex = events.size();
    events.push_back(Event::Vertex(arc->site, lp.y));
    arcEvents[arc->id] = eventIndex;
    eventQueue.insert({ static_cast<int>(round(lp.y)), eventIndex });

    return true;
}

void State::addSiteEvent(const Site& site)
{
    int eventIndex = events.size();
    events.push_back(Event::Site(site.id, site.location.y));
    eventQueue.insert({ static_cast<int>(round(site.location.y)), eventIndex });
}

void State::clearVertexEvent(ArcRef arc)
{
    auto arcEvent = arcEvents.find(arc->id);

    if (arcEvent != arcEvents.end()) {
        std::cout << "remove vertex event with arc: " << arc << ".\n";
        events[arcEvent->second].active = false;
        arcEvents.erase(arcEvent);
    }
}

int State::createSegments(ArcRef a, ArcRef b, Point s)
{
    int sid = segments.size();
    segments.push_back(Segment(s));
    a->s2 = b->s1 = sid;
    segments.back().site1 = a->site;
    segments.back().site2 = b->site;
    return sid;
}

bool State::step()
{
    bool progress = false;

    while (!progress) {

        if (eventQueue.empty()) {
            return false;
        }

        auto it = eventQueue.begin();
        Event ev = events[it->second];
        eventQueue.erase(it);

        if (!ev.active) {
            continue;
        }

        sweepLineY = ev.y;

        if (ev.type == Event::SITE) {

            std::cout << "handling site event, site: " << ev.site << ".\n";

            const auto& newSite = sites[ev.site];
            auto arc = findArcAbove(beachline, newSite.location);

            if (arc == beachline.end()) {
                std::cout << "adding first site " << ev.site << ".\n";
                beachline.push_back(Arc{ .id = nextArcId++, .site = newSite.id, .location = newSite.location });
            }
            else {
                auto intersection = parabolaIntersect(arc->location, newSite.location);
                midPoints.push_back(intersection);

                clearVertexEvent(arc);

                if (arc != beachline.begin() && std::next(arc) != beachline.end()) {
                    std::cout << "splitting arc " << arc << ".\n";
                }
                else {
                    std::cout << "splitting arc " << arc->id << ".\n";
                }
                auto a = beachline.insert(std::next(arc), Arc{ .id = nextArcId++, .site = arc->site, .location = arc->location });
                auto b = beachline.insert(std::next(a), Arc{ .id = nextArcId++, .site = newSite.id, .location = newSite.location });
                auto c = beachline.insert(std::next(b), Arc{ .id = nextArcId++, .site = arc->site, .location = arc->location });

                a->s1 = arc->s1;
                createSegments(a, b, intersection);
                createSegments(b, c, intersection);
                c->s2 = arc->s2;

                beachline.erase(arc);
               
                checkVertexEvent(a, sweepLineY);
                checkVertexEvent(c, sweepLineY);
            }

            progress = true;
        }
        else {
            std::cout << "handling vertex event, site: " << ev.site << ".\n";

            ArcRef arc = beachline.end();
            for (ArcRef it = std::next(beachline.begin()); it != beachline.end(); it++) {
                if (it->site == ev.site && it != beachline.end() && std::next(it) != beachline.end()) {
                    auto bp1 = breakpoint(std::prev(it)->location, it->location, sweepLineY);
                    auto bp2 = breakpoint(it->location, std::next(it)->location, sweepLineY);
                    if (distSqr(bp1, bp2) < 1e-3) {
                        arc = it;
                        break;
                    }
                }
            }

            if (arc == beachline.end()) {
                std::cout << "did not find arc for vertex event.\n";
                continue;
            }
            else {
                std::cout << "collapsing arc " << arc << ".\n";
            }

            auto prev = std::prev(arc);
            auto next = std::next(arc);

            // Check if this circle event is still valid
            if (prev == beachline.begin() || std::next(next) == beachline.end()) {
                continue;  // Skip this event, it's no longer valid
            }

            auto cc = circumcircle(arc->location, prev->location, next->location);

            voronoiVertices.push_back(cc.origin);

            std::cout << "collapsing arc " << arc << " and adding voronoi vertex at(" << cc.origin.x << ", " << cc.origin.y << ").\n";

            createSegments(prev, next, cc.origin);

            if (arc->s1 >= 0) {
                std::cout << "connect arc " << arc << " with new vertex on the left end.\n";
                segments[arc->s1].finish(cc.origin);
            }
            if (arc->s2 >= 0) {
                std::cout << "connect arc " << arc << " with new vertex on the right end.\n";
                segments[arc->s2].finish(cc.origin);
            }
            
            clearVertexEvent(arc);
            beachline.erase(arc);

            checkVertexEvent(prev, sweepLineY);
            checkVertexEvent(next, sweepLineY);

            progress = true;
        }
    }

    std::cout << "current beachline: ";
    for (auto& arc : beachline) {
        std::cout << arc.id << "(" << arc.site << ") ";
    }
    std::cout << "\n" << std::endl;

    return progress;
}

void State::run()
{
    while (step());
}

State::State(const std::vector<Site>& sites) : sites { sites } {
    for (const auto& site : this->sites) {
        addSiteEvent(site);
    }
}

void State::save(const std::string& filename) {

    std::cout << "State: saving to " << filename << std::endl;

    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    for (const auto& p : sites) {
        outFile << p.location.x << " " << p.location.y << "\n";
    }

    outFile.close();
}

std::optional<State> State::load(const std::string& filename) {

    std::cout << "State: loading from " << filename << std::endl;

    std::vector<Site> sites;
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for reading." << std::endl;
        return {};
    }

    int id = 0;
    double first, second;
    while (inFile >> first >> second) {
        sites.push_back(Site{ .id = id++, .location = Point(first, second)});
    }

    inFile.close();
    return State{ sites };
}


// note this function destroys the list in argument
bool toPolygon(std::list<Segment>& segments, Polygon& outPolygon) {
    outPolygon.vertices.clear();

    int n = segments.size();
    auto seg = segments.begin();
    Point p = seg->b;
    seg = segments.erase(seg);

    outPolygon.vertices.push_back(p);
    for (int i = 0; i < n - 1; i++) {
        for (auto it = segments.begin(); it != segments.end(); it++) {
            if (geometry::pointEq(it->a, p, 0.1)) {
                p = it->b;
                segments.erase(it);
                break;
            }
            else if (geometry::pointEq(it->b, p, 0.1)) {
                p = it->a;
                segments.erase(it);
                break;
            }
        }
        outPolygon.vertices.push_back(p);
    }

    if (!segments.empty()) {
        //std::cerr << "error: " << segments.size() << " segments not connected.\n";
        //std::cerr << "connected points: ";
        //for (int i = 0; i < outPolygon.vertices.size(); i++) {
        //    std::cerr << "(" << outPolygon.vertices[i].x << ", " << outPolygon.vertices[i].y << ") ";
        //}
        //std::cerr << "\nunconnected points: ";
        //for (const auto& s : segments) {
        //    std::cerr << "(" << s.a.x << ", " << s.a.y << ")-" << "(" << s.b.x << ", " << s.b.y << ") ";
        //}
        //std::cerr << std::endl;
        //outPolygon.vertices.clear();

        return false;
    }

    if (geometry::windingDirection(outPolygon) < 0) {
        std::reverse(outPolygon.vertices.begin(), outPolygon.vertices.end());
    }

    return true;
}

std::vector<geometry::Polygon> State::getPolygons() const
{
    std::unordered_map<int, std::list<Segment>> siteSegments;
    for (const auto& segment : segments) {
        if (segment.finished) {
            siteSegments[segment.site1].push_back(segment);
            siteSegments[segment.site2].push_back(segment);
        }
    }

    std::vector<geometry::Polygon> result;
    for (auto& [_, segments] : siteSegments) {
        Polygon p;
        if (segments.size() > 2 && toPolygon(segments, p)) {
            result.push_back(p);
        }
    }

    return result;
}

} // namespace tora::sim::fortune