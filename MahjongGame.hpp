#include "Tile.hpp"
#include <json.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <glm/gtx/string_cast.hpp>

using json = nlohmann::json;
using namespace std;

class MahjongGame {

public:
	vector<Tile> tiles;
	bool isInitialized;
	vector<int> suitVectors[36];

	MahjongGame(string path) {
		isInitialized = true;
		// open Input File Stream (ifstream) associated to structure json
		ifstream f(path);
		// parse structure as json
		json data = json::parse(f);
		// close stream, not needed anymore after parsing
		f.close();
		// convert json information into list of tiles
		// import array of suit indices
		vector<int> suits;
		for (auto& el : data["suits"].items())
		{
			suits.push_back(el.value());
		}
		// randomize array of indices
		// based on https://stackoverflow.com/a/6926473
		auto rd = random_device{};
		auto rng = default_random_engine{ rd() };
		shuffle(suits.begin(), suits.end(), rng);
		// work with array of tiles
		for (auto& el : data["tiles"].items()) {
			json tile = el.value();
			int tileIdx = tile["tileIdx"];
			vector<int> under;
			vector<int> over;
			vector<int> left;
			vector<int> right;
			// import bottom vector
			for (auto& uel : tile["under"].items()) {
				under.push_back(uel.value());
			}
			// import top vector
			for (auto& oel : tile["over"].items()) {
				over.push_back(oel.value());
			}
			// import left vector
			for (auto& lel : tile["left"].items()) {
				left.push_back(lel.value());
			}
			// import right vector
			for (auto& rel : tile["right"].items()) {
				right.push_back(rel.value());
			}
			// import position vector
			vector<double> coords;
			for (auto& coord : tile["position"].items()) {
				coords.push_back(coord.value());
			}
			glm::vec3 position = glm::vec3(coords[0], coords[1], coords[2]);
			Tile newTile = Tile(tileIdx, suits[tileIdx], over, under, left, right, position);
			tiles.push_back(newTile);
			suitVectors[newTile.getSuitVectorIndex()].push_back(tileIdx);
		}
	}

	//returns true if the two tiles whose tile_indexes are passed as parameters can be removed from the game together
	bool canRemoveTiles(int idx0, int idx1) {
		Tile tile0 = tiles[idx0];
		Tile tile1 = tiles[idx1];
		int suitIdx0 = tile0.suitIdx;
		int suitIdx1 = tile1.suitIdx;
		bool result = (((suitIdx0 < 40 && suitIdx1 == suitIdx0) ||
			(suitIdx0 >= 40 && suitIdx0 < 44 && suitIdx1 >= 40 && suitIdx1 < 44) ||
			(suitIdx0 >= 44 && suitIdx0 < 48 && suitIdx1 >= 44 && suitIdx1 < 48)) && tile0.isOpen() && tile1.isOpen() && idx0!=idx1);
		
		//DEBUG:
		std::cout << "\nResult of check on 2 tiles: " << result << "\n";
		
		return result;
	}

	void removeTiles(int idx0, int idx1) {
		// access tiles
		Tile tile0 = tiles[idx0];
		Tile tile1 = tiles[idx1];
		int suitIdx0 = tile0.suitIdx;
		int suitIdx1 = tile1.suitIdx;
		// check on suit idx
		if (((suitIdx0 < 40 && suitIdx1 == suitIdx0) ||
			(suitIdx0 >= 40 && suitIdx0 < 44 && suitIdx1 >= 40 && suitIdx1 < 44) ||
			(suitIdx0 >= 44 && suitIdx0 < 48 && suitIdx1 >= 44 && suitIdx1 < 48)) && tile0.isOpen() && tile1.isOpen() && idx0 != idx1) {
			// for each tile, remove its index from neighbours
			for (Tile tile : {tile0, tile1}) {
				// remove current tile index from neighbors
				for (int leftIdx : tile.left) {
					tiles[leftIdx].right.erase(remove(tiles[leftIdx].right.begin(), tiles[leftIdx].right.end(), tile.tileIdx), tiles[leftIdx].right.end());
				}
				for (int rightIdx : tile.right) {
					tiles[rightIdx].left.erase(remove(tiles[rightIdx].left.begin(), tiles[rightIdx].left.end(), tile.tileIdx), tiles[rightIdx].left.end());
				}
				for (int underIdx : tile.under) {
					tiles[underIdx].over.erase(remove(tiles[underIdx].over.begin(), tiles[underIdx].over.end(), tile.tileIdx), tiles[underIdx].over.end());
				}
				//// remove all neighbors from current tile
				//tile.left.clear();
				//tile.right.clear();
				//tile.under.clear();
				// set current tile as removed
				tiles[tile.tileIdx].isRemoved = true;
				// remove current tile from related suit vector
				int svi = tile.getSuitVectorIndex();
				suitVectors[svi].erase(remove(suitVectors[svi].begin(), suitVectors[svi].end(), tile.tileIdx), suitVectors[svi].end());
			}
		}
	}

	void printSuitVectors() {
		for (int i = 0; i < 36; i++) {
			cout << "Vector " << i << ": [";
			for (int index : suitVectors[i]) {
				cout << index << ", ";
			}
			cout << "]\n";
		}
	}

	bool isGameOver() {
		if (isWon()) return false;
		for (vector<int> suitVector : suitVectors) {
			int count = 0;
			for (int index : suitVector) {
				if (tiles[index].isOpen()) count++;
			}
			if (count > 1) return false;
		}
		return true;
	}

	bool isWon() {
		for (Tile tile : tiles) {
			if (!tile.isRemoved) return false;
		}
		return true;
	}

};