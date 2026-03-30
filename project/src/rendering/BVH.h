#ifndef BVH_H
#define BVH_H

#include <cassert>
#include <cstdint>
#include <limits>
#include <vector>
#include "Vector3.h"

namespace mau
{
	struct BVHNode final
	{
		Vector3 aabbMin{};
		Vector3 aabbMax{};

		uint32_t leftFirst{ 0 };
		uint32_t triangleCount{ 0 };

		bool IsLeaf() const
		{ 
			return triangleCount > 0; 
		}

		void BuildBVH(std::vector<BVHNode>& bvh, std::vector<int>& indices, std::vector<Vector3>const& vertices, std::vector<Vector3>& normals, std::vector<Vector3>& transformedNormals)
		{
			static constexpr uint32_t rootNodeIdx{ 0 };

			bvh.reserve(normals.size() * 2);

			bvh[rootNodeIdx].triangleCount = static_cast<uint32_t>(normals.size());

			//Assign all to root node
			UpdateNodeBounds(bvh, indices, vertices, rootNodeIdx);
			//Recursively subdivide
			SubDivide(bvh, indices, vertices, rootNodeIdx, normals, transformedNormals);
		}

		void UpdateNodeBounds(std::vector<BVHNode>& bvh, std::vector<int>const& indices, std::vector<Vector3>const& vertices, uint32_t nodeIdx)
		{
			assert(bvh[nodeIdx].IsLeaf());

			BVHNode& node{ bvh[nodeIdx] };
			node.aabbMin = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() };
			node.aabbMax = { std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest() };
			
			uint32_t const first{ node.leftFirst };
			for (uint32_t i{ 0 }; i < node.triangleCount; ++i)
			{
				//every 3 indices == a triangle
				auto const offset{ (first + i) * 3 };

				node.aabbMin = Vector3::Min(node.aabbMin, vertices[indices[offset]]);
				node.aabbMin = Vector3::Min(node.aabbMin, vertices[indices[offset + 1]]);
				node.aabbMin = Vector3::Min(node.aabbMin, vertices[indices[offset + 2]]);
				node.aabbMax = Vector3::Max(node.aabbMax, vertices[indices[offset]]);
				node.aabbMax = Vector3::Max(node.aabbMax, vertices[indices[offset + 1]]);
				node.aabbMax = Vector3::Max(node.aabbMax, vertices[indices[offset + 2]]);
			}
		}

		void SubDivide(std::vector<BVHNode>& bvh, std::vector<int>& indices, std::vector<Vector3>const& vertices, uint32_t nodeIdx, std::vector<Vector3>& normals, std::vector<Vector3>& transformedNormals)
		{
			if (bvh[nodeIdx].triangleCount <= 2)
			{
				return;
			}

			//Splitting plane axis
			Vector3 const extent{ bvh[nodeIdx].aabbMax - bvh[nodeIdx].aabbMin };
			int axis{ 0 };
			if (extent.y > extent.x)
			{
				axis = 1;
			}
			if (extent.z > extent[axis])
			{
				axis = 2;
			}

			float const splitPos{ bvh[nodeIdx].aabbMin[axis] + extent[axis] * .5f };

			uint32_t i{ bvh[nodeIdx].leftFirst };
			uint32_t j{ i + bvh[nodeIdx].triangleCount - 1 };

			//in place splitting of groups
			while(i <= j)
			{
				auto const idx{ i * 3 };
				Vector3 const center{ (vertices[indices[idx]] + vertices[indices[idx + 1]] + vertices[indices[idx + 2]]) / 3 };

				if (center[axis] < splitPos)
				{
					i++;
				}
				else
				{
					auto const idx2{ (j * 3) };
					std::swap(indices[idx], indices[idx2]);
					std::swap(indices[idx + 1], indices[idx2 + 1]);
					std::swap(indices[idx + 2], indices[idx2 + 2]);

					std::swap(normals[i], normals[j]);
					std::swap(transformedNormals[i], transformedNormals[j]);

					--j;
				}
			}

			//Abort split if one of the sides is empty
			uint32_t const leftCount{ i - bvh[nodeIdx].leftFirst };
			if (leftCount == 0 || leftCount == bvh[nodeIdx].triangleCount)
			{
				return;
			}

			//Create child nodes (vector is pre-reserved so no reallocation)
			auto const leftChildIdx = static_cast<uint32_t>(bvh.size());
			bvh.emplace_back(BVHNode{});
			auto const rightChildIdx = static_cast<uint32_t>(bvh.size());
			bvh.emplace_back(BVHNode{});

			bvh[leftChildIdx].leftFirst = bvh[nodeIdx].leftFirst;
			bvh[leftChildIdx].triangleCount = leftCount;
			bvh[rightChildIdx].leftFirst = i;
			bvh[rightChildIdx].triangleCount = bvh[nodeIdx].triangleCount - leftCount;

			bvh[nodeIdx].leftFirst = leftChildIdx;
			bvh[nodeIdx].triangleCount = 0;

			UpdateNodeBounds(bvh, indices, vertices, leftChildIdx);
			UpdateNodeBounds(bvh, indices, vertices, rightChildIdx);

			SubDivide(bvh, indices, vertices, leftChildIdx, normals, transformedNormals);
			SubDivide(bvh, indices, vertices, rightChildIdx, normals, transformedNormals);
		}
	};
}

#endif