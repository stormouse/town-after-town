#pragma once

#include "VectorMath.h"
#include "Random.h"
#include "delaunator.hpp"
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

namespace tora::sim {

	struct GridKey {
		int gridX;
		int gridY;

		explicit GridKey(int x, int y) : gridX{ x }, gridY{ y } {}

		bool operator==(const GridKey& other) const {
			return gridX == other.gridX && gridY == other.gridY;
		}

		bool operator!=(const GridKey& other) const {
			return !(*this == other);
		}

		inline bool isInside(int centerX, int centerY, int radius) const {
			return centerX - radius <= gridX && gridX <= centerX + radius &&
				centerY - radius <= gridY && gridY <= centerY + radius;
		}
	};

	struct VertexKey {
		GridKey g;
		int index;

		explicit VertexKey(int gx, int gy, int i) : g{ gx, gy }, index{ i } {}

		bool operator==(const VertexKey& other) const {
			return g == other.g && index == other.index;
		}

		bool operator!=(const VertexKey& other) const {
			return !(*this == other);
		}
	};

	struct EdgeKey {
		VertexKey a;
		VertexKey b;

		explicit EdgeKey(VertexKey a, VertexKey b) : a{ a }, b{ b } {}

		bool operator==(const EdgeKey& other) const {
			return a == other.a && b == other.b;
		}

		bool operator!=(const EdgeKey& other) const {
			return !(*this == other);
		}
	};

} // namespace tora


namespace std {
	template <>
	struct hash<tora::sim::GridKey> {
		size_t operator()(const tora::sim::GridKey& k) const {
			return std::hash<int>{}(k.gridX) ^ (std::hash<int>{}(k.gridY) << 8);
		}
	};

	template <>
	struct hash<tora::sim::VertexKey> {
		size_t operator()(const tora::sim::VertexKey& k) const {
			return (std::hash<tora::sim::GridKey>{}(k.g) << 5) ^ (std::hash<int>{}(k.index));
		}
	};

	template <>
	struct hash<tora::sim::EdgeKey> {
		size_t operator()(const tora::sim::EdgeKey& k) const {
			return std::hash<tora::sim::VertexKey>{}(k.a) ^ (std::hash<tora::sim::VertexKey>{}(k.b) << 32);
		}
	};
} // namespace std

namespace tora::sim {

	class TriangulationGrid {
	public:
		TriangulationGrid(int xIndex, int yIndex) : xIndex_{ xIndex }, yIndex_{ yIndex } {}

		void addVertex(Vec2f vert) {
			vertices_.emplace_back(vert);
		}

		void addEdge(EdgeKey edge) {
			edges_.emplace_back(edge);
		}

		void breakEdgesTowards(int gx, int gy, int radius) {
			edges_.erase(
				std::remove_if(edges_.begin(), edges_.end(), [&](const EdgeKey& e) {
					return e.b.g.isInside(gx, gy, radius);
				}),
				edges_.end());
		}

		void breakFarEdges(int threshold) {
			edges_.erase(
				std::remove_if(edges_.begin(), edges_.end(), [&](const EdgeKey& e) {
					return abs(xIndex_ - e.b.g.gridX) > threshold ||
						abs(yIndex_ - e.b.g.gridY) > threshold;
					}),
				edges_.end());
		}

		const std::vector<Vec2f>& vertices() const {
			return vertices_;
		}

		const std::vector<EdgeKey>& edges() const {
			return edges_;
		}

		bool hasNearbyVertex(Vec2f vert, float minDistance) const {
			float m2 = minDistance * minDistance;
			for (const auto& v : vertices_) {
				if (distanceSqr(v, vert) < m2) {
					return true;
				}
			}
			return false;
		}

		GridKey getKey() const {
			return GridKey{ xIndex_, yIndex_ };
		}

		VertexKey getVertexKey(int index) const {
			return VertexKey{ xIndex_, yIndex_, index };
		}

	private:
		int xIndex_;
		int yIndex_;
		std::vector<Vec2f> vertices_;
		std::vector<EdgeKey> edges_;
	};

	class TriangulationGridMap {
	public:
		void buildGrid(int gx, int gy) {
			auto g = std::make_unique<TriangulationGrid>(gx, gy);

			// Generate K vertices
			for (int i = 0; i < kVerticesPerGrid; i++) {
				for (int t = 0; t < kPutVertexMaxRetries; t++) {
					// TODO: Tweak random so we don't get close-to-border verts
					auto vx = kGridSize * random_.getRandomBetween(0.2f, 0.8f);
					auto vy = kGridSize * random_.getRandomBetween(0.2f, 0.8f);
					auto v = Vec2f{ gx * kGridSize + vx, gy * kGridSize + vy };

					if (g->hasNearbyVertex(v, kMinDistanceBetweenVertices)) {
						continue;
					}

					g->addVertex(v);
					break;
				}
			}

			// Triangulation
			auto vertexKeys = std::vector<VertexKey>{};
			auto vertexCoords = std::vector<double>{};
			int index = 0;
			for (const auto& v : g->vertices()) {
				vertexCoords.push_back(v.x);
				vertexCoords.push_back(v.y);
				vertexKeys.push_back(g->getVertexKey(index++));
			}
			for (auto* ng : getNeighborGrids(gx, gy, kMaxImpactRadiusHeuristic)) {
				if (!ng) continue;
				ng->breakEdgesTowards(gx, gy, kMaxImpactRadiusHeuristic);
				index = 0;
				for (const auto& v : ng->vertices()) {
					vertexCoords.push_back(v.x);
					vertexCoords.push_back(v.y);
					vertexKeys.push_back(ng->getVertexKey(index++));
				}
			}

			double w = (kMaxImpactRadiusHeuristic * 2 + kArtificialHullExtension) * kGridSize;
			double stepLength = w / kArtificialHullSteps;
			double sx = (gx + 0.5) * kGridSize - 0.5 * w;
			double sy = (gy + 0.5) * kGridSize - 0.5 * w;
			for (int i = 0, xx = sx; i <= kArtificialHullSteps; i++, xx += stepLength) {
				vertexCoords.push_back(xx);
				vertexCoords.push_back(sy);
				vertexCoords.push_back(xx);
				vertexCoords.push_back(sy + w);
			}
			for (int i = 0, yy = sy; i <= kArtificialHullSteps; i++, yy += stepLength) {
				vertexCoords.push_back(sx);
				vertexCoords.push_back(yy);
				vertexCoords.push_back(sx + w);
				vertexCoords.push_back(yy);
			}

			auto dt = delaunator::Delaunator{ vertexCoords };

			// Finalize and assign edges to grids.
			
			grids_.emplace(GridKey{ gx, gy }, std::move(g));
			
			for (int i = 0; i < dt.triangles.size(); i += 3) {
				if (dt.triangles[i] >= vertexKeys.size() || dt.triangles[i + 1] >= vertexKeys.size() || dt.triangles[i + 2] >= vertexKeys.size()) {
					// artificial triangle, skip
					continue;
				}
				for (int j = 0, k = 1; j < 3; j++, k = (k + 1) % 3) {
					// FIXME: duplicate edges
					const auto& a = vertexKeys[dt.triangles[i + j]];
					const auto& b = vertexKeys[dt.triangles[i + k]];
					if (distanceSqr(Vec2f(vertexCoords[dt.triangles[i + j]], vertexCoords[dt.triangles[i + j] + 1]),
									Vec2f(vertexCoords[dt.triangles[i + k]], vertexCoords[dt.triangles[i + k] + 1])) 
						> kMaxEdgeLength * kMaxEdgeLength) {
						continue;
					}
					if (a.g == b.g) {
						grids_[a.g]->addEdge(EdgeKey{ a, b });
					}
					else {
						grids_[a.g]->addEdge(EdgeKey{ a, b });
						grids_[b.g]->addEdge(EdgeKey{ b, a });
					}
				}
			}

			// Clean up edgey edges.
			//for (auto* ng : getNeighborGrids(gx, gy, kCleanUpRadiusHeuristic)) {
			//	if (ng) {
			//		ng->breakFarEdges(1);
			//	}
			//}
		}

		std::vector<TriangulationGrid*> getNeighborGrids(int x, int y, int r) {
			auto result = std::vector<TriangulationGrid*>{};
			const int r2 = (r * 2 + 1) * (r * 2 + 1);
			if (r2 <= 100) result.reserve(r2);
			
			int sx = x - r;
			int ex = x + r + 1;
			int sy = y - r;
			int ey = y + r + 1;

			for (int tx = sx; tx < ex; tx++) {
				for (int ty = sy; ty < ey; ty++) {
					if (tx == x && ty == y)
						continue;

					auto it = grids_.find(GridKey{ tx, ty });
					if (it != grids_.end()) {
						result.push_back(it->second.get());
					}
				}
			}

			return result;
		}

		void render(sf::RenderWindow& window) {
			for (const auto& [key, grid] : grids_) {
				for (const auto& e : grid->edges()) {
					if (e.a.g.gridY > e.b.g.gridY || e.a.g.gridY == e.b.g.gridY && e.a.g.gridX > e.b.g.gridX) {
						// Between grids, each grid stores half-edges.
						// Establish a consistent rule to draw only one of them.
						continue;
					}
					const auto& v1 = grids_[e.a.g]->vertices()[e.a.index];
					const auto& v2 = grids_[e.b.g]->vertices()[e.b.index];
					sf::Vertex line[] = {
						sf::Vertex(v1),
						sf::Vertex(v2),
					};
					window.draw(line, 2, sf::Lines);
				}
			}
		}

	private:
		std::unordered_map<GridKey, std::unique_ptr<TriangulationGrid>> grids_;
		Random random_;
		static const int kVerticesPerGrid = 3;
		static const int kPutVertexMaxRetries = 10;
		static const int kGridSize = 80;
		static const int kMinDistanceBetweenVertices = 10;
		static const int kMaxEdgeLength = kGridSize * 1.414;
		static const int kMaxImpactRadiusHeuristic = 2;
		// static const int kCleanUpRadiusHeuristic = 3;
		static const int kArtificialHullSteps = 9;
		static const int kArtificialHullExtension = 4;
	};

} // namespace tora::sim
