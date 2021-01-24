#ifndef _GRID_WORLD_H
#define _GRID_WORLD_H
#include <vector>
#include <iostream>
using std::vector;

class GridWorld {
private:
    struct Info {               // stores information for each id, doubly linked list nodes
        int personID;           // id (also the index - accessible on person vector)
        bool isAlive;           // living status
        int row, col;           // district location
        Info *next, *prev;
    };
    
    struct District {           // stores information for each district
        int row, col;           // location
        int populationDist;     // population
        Info *front, *back;     // pointers to the front and back of each list of district members
    };
    
    int idNumber = 0;
    Info *front, *back;         // pointers for the death queue
    District **grid;            // 2d grid array
    int rows, cols;             // nxn dimensions of the world
    int populationW;            // world population
    vector<Info*> person;       // vector of pointers to information on each person
    
public:
    /**
     * constructor:  initializes a "world" with nrows and
     *    ncols (nrows*ncols districtcs) in which all
     *    districtricts are empty (a wasteland!).
     */
    GridWorld(unsigned nrows, unsigned ncols)   {
        this->rows = nrows;
        this->cols = ncols;
        this->populationW = 0;  // 0 default world population
        
        grid = new District*[ncols];
        
        for (int i = 0; i < nrows; i++) {
            District *grids = new District[nrows];
            grid[i] = grids;
        }
        
        for (int i = 0; i < ncols; i++) {
            for (int j = 0; j < nrows; j++) {
                grid[i][j].populationDist=0;    // set default population to 0 in all districts
                grid[i][j].front = nullptr;
                grid[i][j].back = nullptr;
            }
        }
        front = back = nullptr;
    }
    
    // destructor - deletes the grid
    ~GridWorld(){
        for (int i = 0; i < this->rows; i++) {
            delete[] grid[i];
        }
        delete[] grid;
    }
    
    // creates a doubly linked list out of people structs appending: used for birth() and move()
    void append(Info *node) {
        // adding to an empty list
        if (grid[node->row][node->col].populationDist == 0) {
            grid[node->row][node->col].front = node;       // front and back point to the singular node
            grid[node->row][node->col].back = node;
            return;
        }
        // add to the back of an existing list
        grid[node->row][node->col].back->next = node;       // back node's next points to the new node
        node->prev = grid[node->row][node->col].back;       // new node's prev pointer is the back of the current list
        node->next = nullptr;                               // set the back to null
        grid[node->row][node->col].back = node;             // new node is now the back of the list
    }
    
    // removes members from a district
    void remove(Info *node) {
        Info *temp, *prev;
        int row = node->row;    // save district info r,c for the member
        int col = node->col;
        
        // removing a node from a singular list
        if (grid[row][col].populationDist == 1) {
            grid[row][col].front = nullptr;         // list becomes empty
            grid[row][col].back = nullptr;
            node->next = nullptr;                   // null the pointers for the removed member
            node->prev = nullptr;
            return;
        }
        // removing the front of the list
        else if (node == grid[row][col].front) {
            front = node->next;         // advance the front of of the list
            front->prev = nullptr;      // previous field of the new front is null
            node->prev = nullptr;       // null out the prev and next pointers for the removed node
            node->next = nullptr;
            return;
        }
        // removing the end of the list
        else if (node == grid[row][col].back) {
            temp = node->prev;               // save the previous to last member
            temp->next = nullptr;            // set its next field to null
            grid[row][col].back = temp;      // set the new back of the list
            node->next = nullptr;            // null out the prev and next nodes
            node->prev = nullptr;
            return;
        }
        // removing middle members
        prev = node->prev;
        prev->next = node->next;            // adjust the previous and next nodes to point to eachother
        temp = node->next;
        temp->prev = prev;
    }
    
    // removes a member from its current district and appends the id to the death queue to be reused for birth
    void addDeath(Info *victim) {
        Info *temp, *temp2;
        
        int row = victim->row;      // save district info r,c for the member being killed
        int col = victim->col;
        
        // killing the oldest (front) member in the district, that is not a singular member
        if (victim == grid[row][col].front && victim != grid[row][col].back) {
            grid[row][col].front = victim->next;        // advance the front of the list
            grid[row][col].front->prev = nullptr;       // set the previous field to null
        }
        // killing the only member
        else if (grid[row][col].back  == grid[row][col].front) {
            grid[row][col].back = nullptr;
            grid[row][col].front = nullptr;
        }
        // killing last member
        else if (victim == grid[row][col].back) {
            temp = grid[row][col].back->prev;       // save the second to last member
            temp->next = nullptr;                   // update its next pointer to null
            grid[row][col].back = temp;             // set the second to last member to be the last
        }
        // killing a middle member
        else if (victim->next != nullptr && victim->prev != nullptr) {
            temp = victim->prev;                     // save previous and next members
            temp2 = victim->next;
            temp->next = temp2;                      // adjust the previous and next pointers after removing the member
            temp2->prev = temp;
        }
        // populating the death queue
        // empty death queue
        if (front == nullptr) {
            front = victim;             // front and back point to the member
            back = victim;
            victim->prev = nullptr;     // adjust pointers to null
            victim->next = nullptr;
            return;
        }
        // non-empty death queue
        victim->next = nullptr;         // set the next pointer to null
        victim->prev = back;            // have the member's previous point to the current list back
        back->next = victim;            // the back of the current list will now point to the appended member
        back = victim;                  // killed member is now at the back of the death queue
    }
    
    // removes the front id from the death queue if a new member is being born
    Info* popfront() {
        Info *pop = front;
        front = front->next;        // advance the front
        pop->prev = nullptr;        // set the pointers to null
        pop->next = nullptr;
        
        return pop;
    }
    
    /**
     * function: birth
     * description:  if row/col is valid, a new person is created
     *   with an ID according to rules in handout.  New person is
     *   placed in district (row, col)
     *
     * return:  indicates success/failure
     */
    bool birth(int row, int col, int &id){
        // check that the rows and cols are in the range
        if (row >= this->rows || col >= this->cols || row < 0 || col < 0) {
            return false;
        }
        // check for a dead person in the queue before adding a new ID
        if (front != nullptr) {
            Info *recycled = popfront();    // remove the front person from the queue
            id = recycled->personID;        // communicate id to reference parameter
            recycled->row = row;            // update person info
            recycled->col = col;
            recycled->isAlive = true;
            append(recycled);               // add the person to another district
        }
        // create an entirely new id if the death queue is empty
        else {
            Info *subject = new Info;       // create a new person and update their information
            subject->personID = idNumber;   // update person info
            subject->row = row;
            subject->col = col;
            subject->isAlive = true;
            subject->prev = subject->next = nullptr;
            person.push_back(subject);      // insert the person to the universal list - vector index is also their ID
            append(subject);                // add the person to the district
            idNumber++;                     // increment id number
            id = populationW;               // communicate id to reference parameter
        }
        grid[row][col].populationDist++;    // increment district population
        populationW++;                      // increment world population
        return true;
    }
    
    /**
     * function: death
     * description:  if given person is alive, person is killed and
     *   data structures updated to reflect this change.
     *
     * return:  indicates success/failure
     */
    bool death(int personID){
        // cannot kill a non-existent member
        if (personID+1 > person.size() || personID < 0)
            return false;
        
        // kill living member
        if (person.at(personID)->isAlive) {
            person.at(personID)->isAlive = false;
            grid[person.at(personID)->row][person.at(personID)->col].populationDist--;  // decrement population of their district
            addDeath(person.at(personID));      // add the id to the queue
            person.at(personID)->row = -1;      // adjust their row and col to off-grid indices
            person.at(personID)->col = -1;
            populationW--;                      // decrement world population
            return true;
        }
        
        return false;
    }
    
    /**
     * function: whereis
     * description:  if given person is alive, his/her current residence
     *   is reported via reference parameters row and col.
     *
     * return:  indicates success/failure
     */
    bool whereis(int id, int &row, int &col)const{
        // cannot locate dead member
        if (person.at(id)->isAlive == false) {
            return false;
        }
        // locate any members in the correct range
        if ((id+1) <= person.size() && id >= 0) {
            row = person.at(id)->row;       // communicate row and col info to reference parameter
            col = person.at(id)->col;
            return true;
        }
        return false;
    }
    
    /**
     * function: move
     * description:  if given person is alive, and specified target-row
     *   and column are valid, person is moved to specified district and
     *   data structures updated accordingly.
     *
     * return:  indicates success/failure
     *
     * comment/note:  the specified person becomes the 'newest' member
     *   of target district (least seniority) --  see requirements of members().
     */
    bool move(int id, int targetRow, int targetCol){
        if (person.at(id)->isAlive == false  || id+1 > populationW || targetCol > this->cols
            || targetRow > this->rows || targetCol < 0 || targetRow < 0) {
            return false;
        }
        
        // remove member from the current district, update population
        remove(person.at(id));
        grid[person.at(id)->row][person.at(id)->col].populationDist--;
        person.at(id)->row = targetRow;     // update new district info
        person.at(id)->col = targetCol;
        
        // place member in a new district, update population
        append(person.at(id));
        grid[targetRow][targetCol].populationDist++;
        
        return true;
    }
    
    /**
     * populates a vector with the current members of a specified district in descending order of seniority
     * returns the populated vector
     */
    std::vector<int> * members(int row, int col)const{
        vector<int> *members = new vector<int>;     // allocate memory for the vector
        
        Info *temp = grid[row][col].front;          // traversal pointer to the front of the district list
        while (temp != nullptr) {
            members->push_back(temp->personID);     // push back the members
            temp = temp->next;
        }
        return members;
    }
    
    /**
     * function: population
     * description:  returns the current (living) population of the world.
     */
    int population()const{
        return this->populationW;
    }
    
    /**
     * function: population(int,int)
     * description:  returns the current (living) population of specified
     *   district.  If district does not exist, zero is returned
     */
    int population(int row, int col)const{
        return grid[row][col].populationDist;
    }
    
    /**
     * function: num_rows
     * description:  returns number of rows in world
     */
    int num_rows()const {
        return this->rows;
    }
    
    /**
     * function: num_cols
     * description:  returns number of columns in world
     */
    int num_cols()const {
        return this->cols;
    }
    
};

#endif
