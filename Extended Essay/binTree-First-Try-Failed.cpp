#include <iostream>
#include <fstream>  // For file handling 
#include <cstdint>  // for uint8_t
#include <chrono>
#include <time.h>       /* time */
const int nodeSize = 12;
    //node explained
    //first 4 bytes are the value
    //next 4 bytes are the left pointer
    //next 4 bytes are the right pointer
const int headerSize = 4;
    //header explained
    //first 4 bytes are the number of items in the database
    class File{
    private:
    std::fstream inputstream;
    std::string fileName;
    unsigned long long int totalItems = 0;
    int rootNodeLocation = headerSize;
    public:
    class Node{
        public:
        int value;
        int adress;
        File* db;
        Node* left;
        Node* right;
        char* data;
        Node(int byte, File* db){
            this->db = db;
            this->left = nullptr;
            this->right = nullptr;
            std::fstream* file = db->getInputStream();
            this->adress = byte;
            file->seekg(byte);
            this->data = new char[nodeSize];
            file->read(this->data, nodeSize); //breaks the code :)
            this->value = *(int*)(this->data);
        }
        Node(File* db){
            this->db = db;
            std::fstream* file = db->getInputStream();
            this->adress = db->getFileSize();
            this->left = nullptr;
            this->right = nullptr;
            file->seekp(this->adress); //goes to the end of the file
            file->write(reinterpret_cast<char*>(&this->value), sizeof(int));
            int adresses = 0;
            file->write(reinterpret_cast<char*>(0), sizeof(int));
            file->write(reinterpret_cast<char*>(0), sizeof(int));
            file->flush();
            file->seekg(adress);
            this->data = new char[nodeSize];
            file->read(this->data, nodeSize);
            if(!file->good()){
                std::cerr << "file is not good"<<std::endl;
            }else{
                std::cout << "file is good"<<std::endl;
                std::cout << "data: ";
                for(int i = 0; i < nodeSize; i++){
                    std::cout << "["<<this->data[i]<< "]";
                }
                std::cout << std::endl;
            }
            db->totalItems++;       
        }
        Node* getLeft(){
            if(this->left == nullptr){
                int adress = *(int*)(data+4);
                if(adress == 0){
                    return nullptr;
                }
                return new Node(adress,this->db);
            }
            return this->left;
        }
        Node* getRight(){
            std::cout << "getting right"<<std::endl;
            if(this->right == nullptr){
                std::cout << "data: ";
                for(int i = 0; i < nodeSize; i++){
                    std::cout << this->data[i];
                }
                std::cout << std::endl;
                int adress = *(int*)(this->data+8);
                if(adress == 0){
                    return nullptr;
                }
                return new Node(adress,this->db);
            }
            return this->right;
        }
        bool isLeaf(){
            return this->left == nullptr && this->right == nullptr;
        }
        void save(){
            std::fstream* inputstream = this->db->getInputStream();
            inputstream->seekp(this->adress);
            inputstream->write(reinterpret_cast<char*>(&(this->value)), sizeof(int));
            if(this->getLeft() != nullptr){
                inputstream->write(reinterpret_cast<char*>(&(this->getLeft()->adress)), sizeof(int));
            }else{
                inputstream->write(reinterpret_cast<char*>(0), sizeof(int));
            }
            if(this->getRight() != nullptr){
                inputstream->write(reinterpret_cast<char*>(&(this->getRight()->adress)), sizeof(int));
            }else{
                inputstream->write(reinterpret_cast<char*>(0), sizeof(int));
            }
        }
        void close(){
            this->save();
            delete[] this->data;
            if(this->left != nullptr){
                this->left->close();
                delete this->left;

            }
            if(this->right != nullptr){
                this->right->close();
                delete this->right;
            }
        }
    
    };
    Node* root = nullptr;
    std::fstream* getInputStream(){
        return &(this->inputstream);
    }
    double getFileSize(){
        this->inputstream.seekg(0, std::ios::end);
        double fileSize = (double)this->inputstream.tellg();
        return fileSize;
    }  
    int readHeader(){
        
        if(getFileSize() < headerSize){
            std::cout << "wrighting new header"<<std::endl; 
            int totalItems = 0;
            inputstream.seekp(0);
            inputstream.write(reinterpret_cast<char*>(&totalItems), sizeof(int));
            return 0;
        }else{
            inputstream.seekg(0);
            char *buffer = new char[headerSize];
            inputstream.read(buffer, headerSize);
            int totalItems = *(int*)buffer;
            delete[] buffer;
            return totalItems;
        }
    }
    File(std::string name){
        this->inputstream = std::fstream(name, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
        this->fileName = name;
        this->totalItems = readHeader();
        this->root = nullptr;
        if(totalItems > 0){
            this->root = new Node(4,this);
        }
    }
    void save(){
        inputstream.seekp(0);
        inputstream.write(reinterpret_cast<char*>(&totalItems), sizeof(int));
    }
    void flush(){
        this->totalItems = 0;
        this->inputstream.close();
        std::ofstream file(this->fileName, std::ios::trunc);file.close(); //deletes the entire file
        this->inputstream = std::fstream(this->fileName, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
        *this = File(this->fileName);
    }
    
    Node* search(int key){
        Node* current = root;
        while(current != nullptr){
            std::cout << "current: "<<current->value << std::endl;
            std::cout << "left: "<<current->getLeft() << std::endl;
            std::cout << "right: "<<current->getRight() << std::endl;
            if(current->value == key){//figure out why it went to 0
                return current;
            }else if(current->value > key){
                Node* prev = current;
                current = current->getLeft();
                if(current == nullptr) return prev;
                prev->close();
                delete prev;
                prev = nullptr;

            }else{
                Node* prev = current;
                current = current->getRight();
                if(current == nullptr) return prev;
                prev->close();
                delete prev;
                prev = nullptr;
            }
        }
        return current;
    }

    void insert(int key){
        std::cout << "inserting: "<<key << std::endl;
        if(root == nullptr){
            this->root = new Node(this);
            this->root->value = key;
            this->root->save();
            return;
        }
        std::cout << "searching: "<<key << std::endl;
        Node* pos = search(key);
        if(pos == nullptr) return;
        Node* newNode = new Node(this);  //i appended this stuff, idk
        newNode->value = key;
        if(pos->value == key)return;
        if(pos->value > key){
            pos->left = newNode;
        }else{
            pos->right = newNode;
        }
        newNode->save();
        pos->save();
        //std::cout << "pos:"<<pos << std::endl;
    }
    void printInFix(Node* current){
        if(current == root){
            std::cout << "Database: "<<std::endl;
        }
        if(current == nullptr){return;}
        printInFix(current->getLeft());
        std::cout << current->value << std::endl;
        printInFix(current->getRight());
    }
    void printInFix(){
        printInFix(root);
    }
};
int main(){
    std::string name = "example2.db";
    File db(name);
    db.flush();
    db.insert(10);
    // db.insert(20);
    // std::cout << "everything up till here worked"<< std::endl;
    // db.insert(30);
    std::cout << "File Size: "<<db.getFileSize() << std::endl;
    std::cout << "root value: "<< db.root->value << std::endl;
    db.printInFix();

}



//honestly start from scratch fuck this shit