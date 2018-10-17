#include <vector>
#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <algorithm>
#include <string>
#include <map>

using namespace std;

enum {
	OrbType_Health,
	OrbType_Attack,
	OrbType_Defense
};

enum {
	GearType_Helmet,
	GearType_Armor,
	GearType_Gloves,
	GearType_Boots,
	GearType_Pendant,
	GearType_Talisman,
	GearType_Ring,
	GearType_MainHand,
	GearType_OffHand
};

enum {
	Stats_H,
	Stats_A,
	Stats_D,
	Stats_M
};

enum {
	Links_O1,
	Links_O2,
	Links_D1,
	Links_D2
};

class Gear {
public:
	void boostGear(int k);

	string gearName;
	int gearType;
	int potential;
	int boost;
	int stats[4];
	string links[4];
	int total;
	char gearClass;
	bool used;
};

string cumulativeResult[20];
float cumulativeTotal;
int cumulativeTotals[4];

class DataSource {
public:
	map<string, Gear *> gearMap;
	vector<float> hSlots;
	vector<float> aSlots;
	vector<float> dSlots;
	vector<float> mSlots;
	vector<float> tSlots;
	vector<Gear *> knownGear;
	vector<Gear *> equippedGear;
	vector<Gear *> gearPool;
	vector<Gear *> hSortedPool;
	vector<Gear *> aSortedPool;
	vector<Gear *> dSortedPool;
	vector<Gear *> mSortedPool;
	vector<Gear *> hCollection;
	vector<Gear *> aCollection;
	vector<Gear *> dCollection;
	vector<Gear *> mCollection;
	vector<Gear *> toBeIncluded;
	vector<Gear *> result;
	float totals[4];

	void readData();
	void storeGear(string &input);
	void storeCollections(string &input);
	void storeLine(string &input);
	void storeBoost(string &input);
	void matchSlots();
	void generatePool();
	void equipGear(string &input);
	void removeFromPool(Gear *g);
	void editPool(Gear *oldGear, Gear *newGear);
	void calculateTotals();
	void improveCollections();
	void swap(Gear *g1, Gear *g2);
};

void DataSource::readData() {
	string input;
	ifstream inputFile("input.txt");
	if (inputFile.is_open()) {
		while (getline(inputFile, input)) {
			cout << input << endl;
			storeLine(input);
		}
		inputFile.close();
	}
}

void DataSource::storeLine(std::string &input) {
	if (!input.empty()) {

		if (input[0] == 'g') {
			storeGear(input);
		}
		else if (input[0] == 'c') {
			storeCollections(input);
		}
		else if (input[0] == 'e') {
			equipGear(input);
		}
	}
}

// Store value from : c h1 h2 h3 h4 h5 a1 a2 a3 a4 a5 d1 d2 d3 d4 d5 m1 m2 m3 m4 m5
//                      1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20
void DataSource::storeCollections(string &input) {
	istringstream iss(input);
	vector<string> keys;
	string temp;
	while (iss >> temp) {
		keys.push_back(temp);
	}
	if (!keys.empty()) {
		for (int i = 1; i < 6; ++i) hSlots.push_back(stof(keys[i])/100.0);
		for (int i = 6; i < 11; ++i) aSlots.push_back(stof(keys[i])/100.0);
		for (int i = 11; i < 16; ++i) dSlots.push_back(stof(keys[i])/100.0);
		for (int i = 16; i < 21; ++i) mSlots.push_back(stof(keys[i])/100.0);
		sort(hSlots.begin(), hSlots.end());
		sort(aSlots.begin(), aSlots.end());
		sort(dSlots.begin(), dSlots.end());
		sort(mSlots.begin(), mSlots.end());
	}
	tSlots.insert( tSlots.end(), aSlots.begin(), aSlots.end() );
	tSlots.insert( tSlots.end(), dSlots.begin(), dSlots.end() );
	tSlots.insert( tSlots.end(), mSlots.begin(), mSlots.end() );
	tSlots.insert( tSlots.end(), hSlots.begin(), hSlots.end() );
}

// Store value from : g GearName GearType BasePotential Health Attack Defense Magic Link1ID Val O/D Link2ID Val O/D Link3ID Val O/D GearClass
//                         1         2         3           4     5       6      7     8      9  10   11     12  13    14   15   16  17
void DataSource::storeGear(std::string &input) {
	istringstream iss(input);
	vector<string> keys;
	string temp;
	while (iss >> temp) {
		keys.push_back(temp);
	}
	if (!keys.empty()) {
		Gear *newGear = new Gear;
		gearMap[keys[1]] =  newGear;
		newGear->gearName = keys[1];
		for (int i = 0; i < 4; ++i) newGear->links[i] = "";
		if (keys[2] == "Helmet") newGear->gearType = GearType_Helmet;
		else if (keys[2] == "Armor") newGear->gearType = GearType_Armor;
		else if (keys[2] == "Boots") newGear->gearType = GearType_Boots;
		else if (keys[2] == "Pendant") newGear->gearType = GearType_Pendant;
		else if (keys[2] == "Talisman") newGear->gearType = GearType_Talisman;
		else if (keys[2] == "Ring") newGear->gearType = GearType_Ring;
		else if (keys[2] == "MainHand") newGear->gearType = GearType_MainHand;
		else if (keys[2] == "OffHand") newGear->gearType = GearType_OffHand;
		newGear->potential = stoi(keys[3]);
		newGear->stats[Stats_H] = stoi(keys[4]);	
		newGear->stats[Stats_A] = stoi(keys[5]);
		newGear->stats[Stats_D] = stoi(keys[6]);
		newGear->stats[Stats_M] = stoi(keys[7]);
		int dCount = 0;
		int oCount = 0;
		for (int i = 0; i < 3; ++i) {
			if (keys[i*3+10][0] == 'D') {
				newGear->links[dCount+2] = keys[i*3+8]; 
				++dCount;
			}
			else if (keys[i*3+10][0]) newGear->links[oCount] = keys[i*3+8]; ++oCount;
		}
		newGear->gearClass = keys[17][0];
		newGear->used = false;
		knownGear.push_back(newGear);
	}
}

void Gear::boostGear(int k) {
	if (boost < 20) {
		
	}
}

void DataSource::storeBoost(std::string &input) {
	/*
	istringstream iss(input);
	vector<string> keys;
	string temp;
	while (iss >> temp) {
		keys.push_back(temp);
	}
	if (!keys.empty()) {
		int boost = stoi(keys[2]);
		Gear *g = gearMap[keys[1]];
		for (int i = 0; i < boost; ++i) Gear::boostGear;
	}
	*/
}

void DataSource::equipGear(string &input) {
	string temp = input.substr(2);
	for (Gear *g : knownGear) {
		if (g->gearName == temp) {
			g->used = true;
			equippedGear.push_back(g);
			break;
		}
	}
}

void DataSource::generatePool() {
	for (Gear *g1 : equippedGear) {
		for (Gear *g2 : knownGear) {
			if ((g2->gearName == g1->links[Links_O1] || g2->gearName == g1->links[Links_O2] || g2->gearName == g1->links[Links_D1]) && !g2->used) {
				gearPool.push_back(g2);
			}
		}
	}
	vector<Gear *> poolSnapshot =  gearPool;
	for (Gear *g1 : poolSnapshot) {
		for (Gear *g2 : knownGear) {
			if ((g2->gearName == g1->links[Links_O1] || g2->gearName == g1->links[Links_O2] || g2->gearName == g1->links[Links_D1]) && !g2->used) {
				gearPool.push_back(g2);
			}
		}
	}
	sort( gearPool.begin(), gearPool.end() );
	gearPool.erase( unique( gearPool.begin(), gearPool.end() ), gearPool.end() );
	hSortedPool = gearPool;
	aSortedPool = gearPool;
	dSortedPool = gearPool;
	mSortedPool = gearPool;
	sort(hSortedPool.begin(), hSortedPool.end(), [](const Gear* a, const Gear* b) -> bool { return a->stats[Stats_H] > b->stats[Stats_H]; });
	sort(aSortedPool.begin(), aSortedPool.end(), [](const Gear* a, const Gear* b) -> bool { return a->stats[Stats_A] > b->stats[Stats_A]; });
	for (Gear * g : aSortedPool) cout << g->gearName << endl;
	cout << endl;
	sort(dSortedPool.begin(), dSortedPool.end(), [](const Gear* a, const Gear* b) -> bool { return a->stats[Stats_D] > b->stats[Stats_D]; });
	sort(mSortedPool.begin(), mSortedPool.end(), [](const Gear* a, const Gear* b) -> bool { return a->stats[Stats_M] > b->stats[Stats_M]; });
}

void DataSource::swap(Gear *g1, Gear *g2) {
	Gear *temp = g1;
	g1 = g2;
	g2 = temp;
}

void DataSource::matchSlots() {
	for (int i = 0; i < 5; ++i) {
		Gear *g = aSortedPool[0];
		aCollection.push_back(g);
		g->used = true;
		if (gearMap[g->links[Links_O1]] && !gearMap[g->links[Links_O1]]->used) toBeIncluded.push_back(gearMap[g->links[Links_O1]]);
		if (gearMap[g->links[Links_O2]] && !gearMap[g->links[Links_O2]]->used) toBeIncluded.push_back(gearMap[g->links[Links_O2]]);
		removeFromPool(g);
	}
	for (int i = 0; i < 5; ++i) {
		Gear *g = dSortedPool[0];
		dCollection.push_back(g);
		g->used = true;
		if (gearMap[g->links[Links_D1]] && !gearMap[g->links[Links_D1]]->used) toBeIncluded.push_back(gearMap[g->links[Links_D1]]);
		if (gearMap[g->links[Links_D2]] && !gearMap[g->links[Links_D2]]->used) toBeIncluded.push_back(gearMap[g->links[Links_D2]]);
		removeFromPool(g);
	}
	for (int i = 0; i < 5; ++i) {
		Gear *g = mSortedPool[0];
		mCollection.push_back(g);
		g->used = true;
		if (gearMap[g->links[Links_O1]] && !gearMap[g->links[Links_O1]]->used) toBeIncluded.push_back(gearMap[g->links[Links_O1]]);
		if (gearMap[g->links[Links_O2]] && !gearMap[g->links[Links_O2]]->used) toBeIncluded.push_back(gearMap[g->links[Links_O2]]);
		removeFromPool(g);
	}
	for (int i = 0; i < 5; ++i) {
		Gear *g = hSortedPool[0];
		hCollection.push_back(g);
		g->used = true;
		removeFromPool(g);
	}

	sort( toBeIncluded.begin(), toBeIncluded.end() );
	toBeIncluded.erase( unique( toBeIncluded.begin(), toBeIncluded.end() ), toBeIncluded.end() );

	for (int i = 0; (i < toBeIncluded.size()) && (i < 5); ++i) {
		Gear *g = toBeIncluded[i];
		if (!g->used) {
			Gear *temp = hCollection[4-i];
			hCollection[4-i] = g;
			temp->used = false;
			hCollection[4-i]->used = true;
			editPool(g, temp);
		}
	}

	result.insert( result.end(), aCollection.begin(), aCollection.end() );
	result.insert( result.end(), dCollection.begin(), dCollection.end() );
	result.insert( result.end(), mCollection.begin(), mCollection.end() );
	result.insert( result.end(), hCollection.begin(), hCollection.end() );
	result.insert( result.end(), gearPool.begin(), gearPool.end() );

	calculateTotals();	
	improveCollections();
}

void DataSource::calculateTotals() {
	totals[Stats_H] = 0;
	totals[Stats_A] = 0;
	totals[Stats_D] = 0;
	totals[Stats_M] = 0;
	for (int i = 0; i < 20; ++i) {
		float temp = 0;
		float linkMulti = 1;
		Gear *g = result[i];
		if (i < 5) {
			temp = g->stats[Stats_A] * tSlots[i];
			if (g->gearClass = 'W') {
				if (gearMap[g->links[Links_O1]] && gearMap[g->links[Links_O1]]->used) linkMulti += .22;
				if (gearMap[g->links[Links_O2]] && gearMap[g->links[Links_O2]]->used) linkMulti += .22;
			}
			temp *= linkMulti;
			totals[Stats_A] += temp;
		} else if (i < 10) {
			temp = result[i]->stats[Stats_D] * tSlots[i];
			if (gearMap[g->links[Links_D1]] && gearMap[result[i]->links[Links_D1]]->used) linkMulti += .22;
			if (gearMap[g->links[Links_D2]] && gearMap[result[i]->links[Links_D2]]->used) linkMulti += .22;
			temp *= linkMulti;
			totals[Stats_D] += temp;
		} else if (i < 15) {
			temp = result[i]->stats[Stats_M] * tSlots[i];
			if (g->gearClass = 'M') {
				if (gearMap[g->links[Links_O1]] && gearMap[g->links[Links_O1]]->used) linkMulti += .22;
				if (gearMap[g->links[Links_O2]] && gearMap[g->links[Links_O2]]->used) linkMulti += .22;
			}
			temp *= linkMulti;
			totals[Stats_M] += temp;
		} else if (i < 20) {
			temp = result[i]->stats[Stats_H] * tSlots[i];
			totals[Stats_H] += temp;
		}
	}
}

void DataSource::editPool(Gear *oldGear, Gear *newGear) {
	if (find(gearPool.begin(), gearPool.end(), oldGear) != gearPool.end()) {
		*(find(gearPool.begin(), gearPool.end(), oldGear)) = newGear;
	}
}

/*
void DataSource::improveCollections() {
	int i = result.size()-1;
	for (int i = result.size()-1; i >= 0; --i) {
		for (Gear *g : gearPool) {
			if (!g->used) {
				calculateTotals();
				float prev = totals[Stats_A]*1.5+totals[Stats_D]+totals[Stats_M]*.5;
				Gear *temp = result[i];
				result[i] = g;
				calculateTotals();
				float curr = totals[Stats_A]*1.5+totals[Stats_D]+totals[Stats_M]*.5;
				if (curr <= prev) {
					result[i] = temp;
					calculateTotals();
				}
				else {
					result[i]->used = true;
					temp->used = false;
					editPool(g, temp);
					improveCollections();
				}
			}
		}
	}
}
*/

void DataSource::improveCollections() {
	cout << "improveCollections call" << endl;
	for (Gear * g: result) cout << g->gearName << endl;
	for (int i = 19; i >= 0; --i) {
		Gear *origin = result[i];
		for (int j = result.size(); j >= 0; --j) {
			Gear *target = result[j];
			if (origin != target) {
				calculateTotals();
				float oldVal = totals[Stats_A] * 1.5 + totals[Stats_D] + totals[Stats_M] * .5;
				swap(origin, target);
				calculateTotals();
				float newVal = totals[Stats_A] * 1.5 + totals[Stats_D] + totals[Stats_M] * .5;
				if (newVal > oldVal) {
					cout << "Prev: " << oldVal << endl;
					cout << "Curr: " << newVal << endl;
					origin->used = !origin->used;
					target->used = !target->used;
					improveCollections();
					return;
				}
				else {
					swap(origin, target);
					calculateTotals();
				}
			}
		}
	}
}

void DataSource::removeFromPool(Gear *g) {
	hSortedPool.erase(find(hSortedPool.begin(), hSortedPool.end(), g));
	aSortedPool.erase(find(aSortedPool.begin(), aSortedPool.end(), g));
	dSortedPool.erase(find(dSortedPool.begin(), dSortedPool.end(), g));
	mSortedPool.erase(find(mSortedPool.begin(), mSortedPool.end(), g));
	gearPool.erase(find(gearPool.begin(), gearPool.end(), g));
}

int main ()
{
	for (int i = 0; i < 10; ++i) {
		DataSource *src = new DataSource;
		src->readData();
		src->generatePool();
		src->matchSlots();
		float currentTotal = src->totals[Stats_A]*1.5+src->totals[Stats_D]+src->totals[Stats_M]*.5;
		if (currentTotal > cumulativeTotal) {
			cumulativeTotal = currentTotal;
			cumulativeTotals[Stats_H] = src->totals[Stats_H];
			cumulativeTotals[Stats_A] = src->totals[Stats_A];
			cumulativeTotals[Stats_D] = src->totals[Stats_D];
			cumulativeTotals[Stats_M] = src->totals[Stats_M];
		}
		for (i = 0; i < 20; ++i) cumulativeResult[i] = src->result[i]->gearName;
		for (Gear *g : src->knownGear) delete g;
		delete src;
	}
	ofstream outputFile;
	outputFile.open("output.txt");
	outputFile << "From highest percent slot to lowest" << endl;
	for (int i = 0; i < 20; ++i) {
		if (i < 5) outputFile << "Attack:  " << "Gear: " << cumulativeResult[i] << endl;
		else if (i < 10) outputFile << "Defense: " << "Gear: " << cumulativeResult[i] << endl;
		else if (i < 15) outputFile << "Magic:   " << "Gear: " << cumulativeResult[i] << endl;
		else if (i < 20) outputFile << "Health:  " << "Gear: " << cumulativeResult[i] << endl;
	}
	for (int i = 0; i < 20; ++i) {
		if (i < 5) cout << "Attack:  " << "Gear: " << cumulativeResult[i] << endl;
		else if (i < 10) cout << "Defense: " << "Gear: " << cumulativeResult[i] << endl;
		else if (i < 15) cout << "Magic:   " << "Gear: " << cumulativeResult[i] << endl;
		else if (i < 20) cout << "Health:  " << "Gear: " << cumulativeResult[i] << endl;
	}
	outputFile << "Bonus Per Gear attack: " << cumulativeTotals[Stats_A] << "  defense: " << cumulativeTotals[Stats_D] << "  magic: " << cumulativeTotals[Stats_M] << "  health: " << cumulativeTotals[Stats_H] << endl;
	outputFile.close();
	cout << "Bonus Per Gear attack: " << cumulativeTotals[Stats_A] << "  defense: " << cumulativeTotals[Stats_D] << "  magic: " << cumulativeTotals[Stats_M] << "  health: " << cumulativeTotals[Stats_H] << endl;
	cout << cumulativeTotal << endl;
}