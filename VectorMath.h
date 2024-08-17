#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

namespace tora {

	template <class T>
	using Vec2 = sf::Vector2<T>;

	template <class T>
	using Vec3 = sf::Vector3<T>;

	using Vec2f = sf::Vector2f;
	using Vec2i = sf::Vector2i;

	template <class TVec>
	inline float sqrNorm(TVec&& v) {
		return v.x * v.x + v.y * v.y;
	}

	template <class TVec>
	inline float norm(TVec&& v) {
		return sqrtf(sqrNorm(v));
	}

	template <class TVec1, class TVec2>
	inline float distanceSqr(TVec1&& a, TVec2&& b) {
		return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
	}

	template <class TVec1, class TVec2>
	inline float distance(TVec1&& a, TVec2&& b) {
		return sqrtf(distanceSqr(a, b));
	}

} // namespace tora