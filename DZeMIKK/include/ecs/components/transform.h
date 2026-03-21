#ifndef DZEMIKK_TRANSFORM_H
#define DZEMIKK_TRANSFORM_H

#pragma once

#include "../component.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace dzemikk {
    class Transform : public Component {
        public:
            // --- Constructors & Destructor
            Transform(glm::vec3 position = glm::vec3(0.0f), glm::vec3 rotation = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f));
            Transform(const Transform& other);
            Transform& operator=(const Transform& other);

            // --- Setters
            void setPosition(const glm::vec3 position);
            void setRotation(const glm::quat rotation);
            void setScale(const glm::vec3 scale);

            void setEulerAngles(glm::vec3 rotation);

            // --- Getters
            const glm::vec3 getPosition() const;
            const glm::quat getRotation() const;
            const glm::vec3 getScale() const;

            const glm::vec3 getEulerAngles() const;

            // --- Modifiers
            void translate(const glm::vec3& delta);
            void rotate(const glm::quat& q);
            void rotateEuler(const glm::vec3& degrees);
            void rotateAround(float degrees, const glm::vec3& axis);
            void scale(const glm::vec3& scale);
            void scale(float uniform);

            // --- Direction vectors
            glm::vec3 forward() const;
            glm::vec3 right() const;
            glm::vec3 up() const;

            // --- Matrix
            const glm::mat4& getMatrix();
        private:
            glm::vec3 _position = glm::vec3(0.0f);
            glm::quat _rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            glm::vec3 _scale = glm::vec3(1.0f);

            glm::mat4 _cachedMatrix = glm::mat4(1.0f);
            bool _dirty = true;
    };
}

#endif // DZEMIKK_TRANSFORM_H
