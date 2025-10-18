#pragma once

#include <spdlog/fmt/bundled/format.h>

#include <glm/fwd.hpp>

template<>
struct fmt::formatter<glm::vec3> : fmt::formatter<std::string> {
	template <typename Context>
    auto format(const glm::vec3& input, Context& ctx) {
		return fmt::format_to(
			ctx.out(),
			"({:.3f}, {:.3f}, {:.3f})",
			input.x, input.y, input.z
		);
	}
};

template<>
struct fmt::formatter<glm::vec4> {
	template <typename Context>
    auto format(const glm::vec4& input, Context& ctx) {
		return fmt::format_to(
			ctx.out(),
			"({:.3f}, {:.3f}, {:.3f}, {:.3f})",
			input.x, input.y, input.z, input.w
		);
	}
};