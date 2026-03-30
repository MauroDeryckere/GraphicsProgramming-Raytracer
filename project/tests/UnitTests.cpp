#include <gtest/gtest.h>
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix.h"

namespace mau
{
	TEST(Vector3, DotProduct) {
		EXPECT_EQ(1.0f, Vector3::Dot(Vector3::UnitX, Vector3::UnitX)); // (1) Same direction
		EXPECT_EQ(-1.0f, Vector3::Dot(Vector3::UnitX, -Vector3::UnitX)); // (-1) Opposite direction
		EXPECT_EQ(0.0f, Vector3::Dot(Vector3::UnitX, Vector3::UnitY)); // (0) Perpendicular

		Vector3 v1(1.0f, 2.0f, 3.0f);
		Vector3 v2(4.0f, 5.0f, 6.0f);
		EXPECT_EQ(32.0f, mau::Vector3::Dot(v1, v2));
	}

	TEST(Vector4, DotProduct) {
		EXPECT_EQ(70.f, Vector4::Dot({ 1, 2, 3, 4 }, { 5, 6, 7, 8 }));
		EXPECT_EQ(30.f, Vector4::Dot({ 1, 2, 3, 4 }, { 1, 2, 3, 4 }));
		EXPECT_EQ(-30.f, Vector4::Dot({ 1, 2, 3, 4 }, { -1, -2, -3, -4 }));
		EXPECT_EQ(0.f, Vector4::Dot({ 1, 0, 0, 0 }, { 0, 1, 0, 0 }));
		EXPECT_EQ(0.f, Vector4::Dot({ 0, 1, 0, 0 }, { 0, 0, 1, 0 }));
	}

	TEST(Vector3, CrossProduct) {
		EXPECT_EQ(Vector3::UnitY, Vector3::Cross(Vector3::UnitZ, Vector3::UnitX)); // (0,1,0) UnitY
		EXPECT_EQ(-Vector3::UnitY, Vector3::Cross(Vector3::UnitX, Vector3::UnitZ)); // (0,-1,0) -UnitY

		Vector3 v1(1.0f, 2.0f, 3.0f);
		Vector3 v2(4.0f, 5.0f, 6.0f);
		EXPECT_EQ(mau::Vector3(-3.0f, 6.0f, -3.0f), mau::Vector3::Cross(v1, v2));
	}


	int main(int argc, char** argv) {
		::testing::InitGoogleTest(&argc, argv);
		return RUN_ALL_TESTS();
	}
}
