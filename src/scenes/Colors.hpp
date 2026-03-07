#pragma once

#include <glm/glm.hpp>

namespace scenes::Colors {
    inline constexpr glm::vec4 BtnBgHovered    = {0.35f, 0.35f, 0.75f, 1.00f};
    inline constexpr glm::vec4 BtnBgNormal     = {0.15f, 0.15f, 0.30f, 1.00f};
    inline constexpr glm::vec4 BtnTextHovered  = {1.00f, 0.15f, 0.30f, 1.00f};
    inline constexpr glm::vec4 BtnTextNormal   = {0.80f, 0.80f, 0.80f, 1.00f};
    inline constexpr glm::vec4 BtnBorder       = {0.50f, 0.50f, 1.00f, 1.00f};

    inline constexpr glm::vec4 PlayerLeft      = {0.30f, 0.60f, 1.00f, 1.00f};
    inline constexpr glm::vec4 PlayerRight     = {1.00f, 0.40f, 0.40f, 1.00f};

    inline constexpr glm::vec4 BallColor       = {1.0f, 0.95f, 0.3f, 1.0f};

    inline constexpr glm::vec4 MainWhite       = {1.00f, 1.00f, 1.00f, 1.00f};
    inline constexpr glm::vec4 MainSubtle      = {0.65f, 0.65f, 0.65f, 1.00f};

    inline constexpr glm::vec4 CreditsNormal   = MainWhite;
    inline constexpr glm::vec4 CreditsSubtle   = MainSubtle;
    inline constexpr glm::vec4 CreditsAccent   = {0.50f, 0.75f, 1.00f, 1.00f};
    inline constexpr glm::vec4 CreditsHint     = {0.50f, 0.50f, 0.50f, 1.00f};
    inline constexpr glm::vec4 CreditsPanel    = {0.05f, 0.05f, 0.12f, 1.00f};
    inline constexpr glm::vec4 CreditsBorder   = {0.40f, 0.40f, 0.80f, 0.80f};
}
