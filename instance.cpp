#include "instance.hpp"

#include <fstream>
#include <iostream>

using namespace std;

//---------------------------------------------------------
Instance::Instance() {

    
    bin_size = -1;

}

//---------------------------------------------------------
void Instance::loadFromFile(string filename) {

    ifstream file(filename);

    if(file) {

        // first line: bin size and n objects
        int n_objects = 0;
        file >> bin_size;
        file >> n_objects;

        objects.resize(n_objects);

        // following lines: object size and number of objects
        for(int obj_ind = 0; obj_ind < n_objects; obj_ind ++) {
            file >> objects[obj_ind].size;
            file >> objects[obj_ind].nb;
        }

        file.close();
    }

}

//---------------------------------------------------------
unsigned int Instance::n_obj() {
    return static_cast<unsigned int>(objects.size());
}


//---------------------------------------------------------
void Instance::display() {

    cout << endl;
	cout << "Bin packing instance:" << endl << endl;
	cout << "Bin size : " << bin_size << endl;
	cout << "Number of object types: " << objects.size() << endl;
	cout << "Objects : " << endl << endl;

	for(unsigned int ind = 0; ind < objects.size(); ++ind) {
		cout << "Object " << ind << " : size = " << objects[ind].size << " ; Nb = " << objects[ind].nb << endl;
    }
	
    cout << endl << endl;

}