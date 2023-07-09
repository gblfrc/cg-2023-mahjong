#include <glm/glm.hpp>
#include <vector>

class Tile {

	public: 
		int tileIdx;
		int suitIdx;
		std::vector<int> over;
		std::vector<int> under;
		std::vector<int> left;
		std::vector<int> right;
		glm::vec3 position;

		Tile(int tileIdx, int suitIdx,
			std::vector<int> over,
			std::vector<int> under,
			std::vector<int> left,
			std::vector<int> right,
			glm::vec3 position
			) {
			this->tileIdx = tileIdx;
			this->suitIdx = suitIdx;
			this->over = over;
			this->under = under;
			this->left = left;
			this->right = right;
			this->position = position;
		};


		bool isOpen() {
			return (over.size() == 0 && (left.size() == 0 ||right.size() == 0));
		}
};
