#ifndef CAMERA_H
#define CAMERA_H

#include <algorithm>

#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Maths.h"
#include "Timer.h"

namespace mau
{
	struct Camera final
	{
	public:
		Vector3 origin{};
		float fovAngle{ 90.f };

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix cameraToWorld{  };

		float movementSpeed{ 3.f };
		float rotationSpeed{ 10.f };

		[[nodiscard]] bool IsDirty() const noexcept { return m_IsDirty; }

		Camera() = default;

		Camera(Vector3 const&  _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{ }

		Matrix CalculateCameraToWorld()
		{
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			cameraToWorld = { right, up, forward, origin };

			return cameraToWorld;
		}

		void Update(Timer* pTimer)
		{
			m_IsDirty = false;
			float const deltaTime{ pTimer->GetElapsed() };

			Vector3 movementDir{ };

			//Keyboard Input
			uint8_t const* pKeyboardState{ SDL_GetKeyboardState(nullptr) };
			if(pKeyboardState[SDL_SCANCODE_W])
			{
				movementDir += forward;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				movementDir -= forward;
			}

			if (pKeyboardState[SDL_SCANCODE_A])
			{
				movementDir -= right;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				movementDir += right;
			}

			if (movementDir != Vector3::Zero)
			{
				movementDir.Normalize();
				origin += (movementDir * movementSpeed * deltaTime);
				m_IsDirty = true;
			}

			//Mouse Input
			int mouseX{};
			int mouseY{};
			uint32_t const mouseState{ SDL_GetRelativeMouseState(&mouseX, &mouseY) };

			if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				if (UpdateCameraDirection(static_cast<float>(mouseX), static_cast<float>(mouseY), deltaTime))
					m_IsDirty = true;
			}
		}

	private:
		bool m_IsDirty{ false };

		bool UpdateCameraDirection(float deltaX, float deltaY, float deltaTime)
		{
			if (deltaX == 0.f && deltaY == 0.f)
			{
				return false;
			}

			totalYaw += deltaX * rotationSpeed * deltaTime;
			totalPitch = std::clamp(totalPitch + deltaY * rotationSpeed * deltaTime, -89.f, 89.f);

			auto const m{ Matrix::CreateRotation(TO_RADIANS * totalPitch, TO_RADIANS * totalYaw, 0.f) };

			forward = m.TransformVector(Vector3::UnitZ);
			forward.Normalize();
			return true;
		}
	};
}

#endif
