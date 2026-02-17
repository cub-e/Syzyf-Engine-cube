#include <cmath>

namespace Easing {
  inline float inOutBack(const float progress) {
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;

    if (progress < 0.5f) {
      return ((2.0f * progress) * (2.0f * progress) * ((c2 + 1.0f) * 2.0f * progress - c2)) * 0.5f;
    } else {
      return ((2.0f * progress - 2.0f) * (2.0f * progress - 2.0f) * ((c2 + 1.0f) * (progress * 2.0f - 2.0f) + c2) + 2.0f) * 0.5f;
    }
  }

  inline float inOutSine(const float progress) {
    return -(cos(M_PI * progress) - 1.0f) * 0.5f;
  }
}
