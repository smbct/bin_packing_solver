#pragma once

#include <vector>
#include <string>

// store an object to place in the bins
//---------------------------------------
class Object {
    
    public:
        Object() { }

    public:
        unsigned int size;
        unsigned int nb;
};

//---------------------------------------
class Instance {

    public:

        // constructor
        Instance();

        // load an instance from a file
        void loadFromFile(std::string filename);

        // return the number of objects
        unsigned int n_obj();

        // display an instance information
        void display();

    public:

        unsigned int bin_size;
        std::vector<Object> objects; 

};