#include <iostream>
#include <fstream>  // For file handling 
#include <cstdint>  // for uint8_t
#include <chrono>
#include <time.h>       /* time */
#include <random>

double fileSize = 0;
const int nodeSize = 12;
    //node explained
    //first 4 bytes are the value
    //next 4 bytes are the left pointer
    //next 4 bytes are the right pointer
const int headerSize = 4;
    //header explained
    //first 4 bytes are the number of items in the database
void clearFile(std::string name, std::fstream* file){
    file->close();
    std::ofstream f(name, std::ios::trunc);f.close(); //deletes the entire file
    *file = std::fstream(name, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
}
double getFileSize(std::fstream* db){
    db->seekg(0, std::ios::end);
    double fileSizeT = (double)db->tellg();
    fileSize = fileSizeT;
    return fileSizeT;
}
class Node{
    public:
    Node* parent;
    int value;
    int adress;
    int leftAdress;
    int rightAdress;
    std::fstream* db;
    Node(std::fstream* db): Node((int)(fileSize),db,nullptr){}
    Node(int adress, std::fstream* db): Node(adress, db, nullptr){}
    Node(int adress, Node* parent): Node(adress, parent->db, parent){}
    Node(int adress, std::fstream* db, Node* parent){
        this->db = db;
        this->adress = adress;
        if(this->adress >= fileSize || this->adress == 0){
            this->adress = fileSize;
            db->seekp(this->adress);
            char dummydata[] = {0x0,0x0,0x0,0x0};
            db->write(dummydata, sizeof(int));
            db->write(dummydata, sizeof(int));
            db->write(dummydata, sizeof(int));
            this->value = -1;
            this->leftAdress = 0;
            this->rightAdress = 0;
            fileSize += nodeSize;
        }else{
            this->adress = adress;
            db->seekg(this->adress);
            db->read(reinterpret_cast<char*>(&this->value), sizeof(int));
            db->read(reinterpret_cast<char*>(&leftAdress), sizeof(int));
            db->read(reinterpret_cast<char*>(&rightAdress), sizeof(int));
            // if(this->leftAdress != 0){
            //     this->left = new Node(this->leftAdress, this->db, this);
            // }
            // if(this->rightAdress != 0){
            //     this->right = new Node(this->rightAdress, this->db, this);
            // }//not doing this because memory :(
        }
        this->parent = parent;
    }
    void save(){
        db->seekp(this->adress);
        db->write(reinterpret_cast<char*>(&this->value), sizeof(int));
        db->write(reinterpret_cast<char*>(&leftAdress), sizeof(int));
        db->write(reinterpret_cast<char*>(&rightAdress), sizeof(int));
    }
    Node* getLeft(){
        if(leftAdress == 0) {return nullptr;}
        return new Node(leftAdress, this);
    }
    Node* getRight(){
        if(rightAdress == 0) return nullptr;
        return new Node(rightAdress, this);
    }
    Node* search(int key){
        Node* current = this;
        while(current->value != key){
            Node* next = current->value > key ? current->getLeft() : current->getRight();
            if(next == nullptr) return current;
            if(current->adress != headerSize){
                current->db = nullptr;
                delete current;
            }
            current = next;
        }
        return current;
    }
    Node* appendNode(int key){ //returns false it the node already exists
        Node* parent = search(key);
        if(parent->value == key) return parent;
        Node* newNode = new Node(this->db);
        newNode->value = key;
        newNode->parent = parent;
        if(parent->value > key){
            parent->leftAdress = newNode->adress;
        }else{
            parent->rightAdress = newNode->adress;
        }
        newNode->save();
        parent->save();
        return newNode;
    }
    void printInFix(){
        //prints the children of the current node in Order
        Node* left = this->getLeft();
        if(left != nullptr){
            left->printInFix();
        }
        std::cout << this->value << ", ";
        Node* right = this->getRight();
        if(right != nullptr){
            right->printInFix();
        }
    }
    ~Node() {
        this->db = nullptr;
    }
};
void saveFile(int& items, std::fstream* file){
    file->seekp(0);
    file->write(reinterpret_cast<char*>(&items),4); //example header
    if(fileSize == 0){
        fileSize += 4;
    }
}
void insertBalanced(Node* root, int low, int high,int& items,std::ofstream& file2) { // too untrustworthy
    if (low > high) return;
    int mid = low + (high - low) / 2;
    Node* newNode;
    if(items%100 == 0){
        auto start = std::chrono::high_resolution_clock::now();
        newNode = root->appendNode(items*(items%2==0?1:-1));
        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
        file2 << (items+1) <<","<< duration << std::endl;
        std::cout << (items+1) <<","<< duration << std::endl;
    }else{
        newNode = root->appendNode(items*(items%2==0?1:-1));
    }
    if(newNode->adress != headerSize){
        newNode->db = nullptr;
        delete newNode;
        items+=1;
    }

    insertBalanced(root, low, mid - 1,items,file2);
    insertBalanced(root, mid + 1, high,items,file2);
}



//random insert main function 
int main(){
    std::fstream file("example2.db", std::ios::in | std::ios::out | std::ios::binary);
    std::ofstream file2("data.csv", std::ios::app); //storing the data
    int items = 0;
    if(file.is_open()) std::cout <<"file opened"<<std::endl;
    std::cout << "File Size: "<< getFileSize(&file) << std::endl;
    //clearFile("example2.db", &file);
    if(getFileSize(&file) <= 0){
        std::cout << "file is empty"<<std::endl;
        saveFile(items, &file);
    }else{
        std::cout << "file is not empty"<<std::endl;
        file.seekg(0);
        file.read(reinterpret_cast<char*>(&items),sizeof(int));
    }
    bool hasRoot = getFileSize(&file) > headerSize;
    Node* root = new Node(4,&file); // opens or creates the Root node
    if (!hasRoot){
        root->value = 0;
        items+=1;
    }
    //insertBalanced(root,items-5000000,items,items,file2);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(INT_MIN,INT_MAX);
    for (int a = 0; a < 100; a++)
    {
        int newItems = items*0.2;
        for(int i=0;i<newItems;i++){ //insert 1 million randm numbers
            Node* newNode = root->appendNode(dis(gen));  //dis(gen)
            //fastest way to insert numbers with a average case
            //best case takes too long to calculate the numbers for
            //worst case takes too long to inser the numbers for
            //newNode->db = nullptr;
            delete newNode;
            if(newNode->adress != headerSize){
                items+=1;
            }
        }
        auto start = std::chrono::high_resolution_clock::now();
        Node* newNode = root->appendNode(dis(gen)); 
        auto end = std::chrono::high_resolution_clock::now();
        delete newNode;
        if(newNode->adress != headerSize){
            items+=1;
        }
        double duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
        file2 << items <<","<< duration << std::endl;
        std::cout << items <<","<< duration << std::endl;
    }
    

    saveFile(items, &file);
    //root->printInFix();
    std::cout << std::endl;
    file.close();
    file2.close();
    std::cout << "items: "<<items << std::endl;
}


// the main function bellow was used to generate the linear insert data  


// int main(){ 
//     std::fstream file("example2.db", std::ios::in | std::ios::out | std::ios::binary);
//     std::ofstream file2("data.csv", std::ios::app); //storing the data
//     int items = 0;
//     if(file.is_open()) std::cout <<"file opened"<<std::endl;
//     std::cout << "File Size: "<< getFileSize(&file) << std::endl;
//     //clearFile("example2.db", &file);
//     if(getFileSize(&file) <= 0){
//         std::cout << "file is empty"<<std::endl;
//         saveFile(items, &file);
//     }else{
//         std::cout << "file is not empty"<<std::endl;
//         file.seekg(0);
//         file.read(reinterpret_cast<char*>(&items),sizeof(int));
//     }
//     bool hasRoot = getFileSize(&file) > headerSize;
//     Node* root = new Node(4,&file); // opens or creates the Root node
//     if (!hasRoot){
//         root->value = 0;
//         items+=1;
//     }
//     //insertBalanced(root,items-1000,items,items);
//     std::random_device rd;
//     std::mt19937 gen(rd());
//     std::uniform_int_distribution<> dis(INT_MIN,INT_MAX);
//     for (int a = 0; a < 100; a++)
//     {
//         int newItems = 100;
//         for(int i=0;i<newItems;i++){ //insert 1 million randm numbers
//             Node* newNode = root->appendNode(items);  //dis(gen)
//             //fastest way to insert numbers with a average case
//             //best case takes too long to calculate the numbers for
//             //worst case takes too long to inser the numbers for
//             //newNode->db = nullptr;
//             delete newNode;
//             if(newNode->adress != headerSize){
//                 items+=1;
//             }
//         }
//         auto start = std::chrono::high_resolution_clock::now();
//         Node* newNode = root->appendNode(items); 
//         auto end = std::chrono::high_resolution_clock::now();
//         delete newNode;
//         if(newNode->adress != headerSize){
//             items+=1;
//         }
//         double duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
//         file2 << items <<","<< duration << std::endl;
//         std::cout << items <<","<< duration << std::endl;
//     }
    

//     saveFile(items, &file);
//     //root->printInFix();
//     std::cout << std::endl;
//     file.close();
//     file2.close();
//     std::cout << "items: "<<items << std::endl;
// }