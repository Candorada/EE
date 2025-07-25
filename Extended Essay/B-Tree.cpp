#include <iostream>
#include <fstream>  // For file handling 
#include <cstdint>  // for uint8_t
#include <chrono>
#include <time.h>       /* time */
const int pageSize = 1<<12;
const int headerSize = 9;
const int maxSize = (pageSize-headerSize)/sizeof(int)/2 - 1; //div 2 -1 because we need to store the pointers aswell and theres 1 pointers per item + 1 pointer
const int minSize = maxSize/2;

//header explained 
//first 4 bytes are the page number
//next 4 bytes are the number of items in the page ----- stores the root page location if the header page
//next 1 byte is the page type l for leaf r for root b for branch and  g for garbage, h for header page (located at page 0)

//root page
//first 4 bytes are page number 0
//next 4 bytes are the location of the root page
//next 1 byte is the page type
//next 8 or so bytes are the number of items in the database
class File{
    private:
    
    std::fstream inputstream;
    std::string fileName;
    int pages;
    unsigned long long int totalItems = 0;
    int rootPage = 1;
    bool createPage(int pageNumber){
        std::fstream &file = this->inputstream;
        int dHeaderSize = ::headerSize; //dynamic header size (used for special cases)
        char pageType;
        int items = 0;
        if(pageNumber == 0){
            pageType = 'h';
        }else if (pageNumber == 1){
            pageType = 'r';
        }else if (pageNumber == 2){
            pageType = 'g';
        }else{
            pageType = 'l';
        }
        file.seekp(pageSize*pageNumber);
        file.write(reinterpret_cast<char*>(&pageNumber), 4);
        if(pageType == 'h'){
            int rootPage = 1;
            file.write(reinterpret_cast<char*>(&rootPage), 4); //root page location if header page
        }else{
            file.write(reinterpret_cast<char*>(&items), 4);
        }
        file.write(reinterpret_cast<char*>(&pageType), sizeof(char));
        if (pageType == 'h') {
            int totalItems = 0;
            file.write(reinterpret_cast<char*>(&totalItems), sizeof(unsigned long long int));
            dHeaderSize = dHeaderSize+sizeof(unsigned long long int);
        }
        uint8_t zero_byte = 0x00;
        for (int i = 0; i < pageSize-dHeaderSize; i++) {
            file.write(reinterpret_cast<char*>(&zero_byte), 1);
        }
        this->pages++;
        return true;
    }
    //retrns page Number
    int appendPage(){
        int pageNumber = getPageCount();
        createPage(pageNumber);
        return pageNumber;
    }
    public:
    class Page{
        private:
        int pageNumber;
        int items;
        int data[maxSize];
        int pointers[maxSize+1];
        char pageType;
        File *db;
        Page *parent;
        public:
        Page(int pageNumber, File &db, Page *parent){
            this->parent = parent;
            this->db = &db;
            this->pageNumber = pageNumber;
            char *buffer = db.getPage(pageNumber);
            this->items = *(int*)(buffer+4);
            this->pageType = *(char*)(buffer+8); //+8 beacuse the first 8 bytes are the page number and item cout
            int i;
            for (i = 0; i < maxSize; i++) { //not using items since the rest of *buffer is then garbage
                int key = *(int*)(buffer+headerSize+2*i*sizeof(int)+sizeof(int));
                int pointer = *(int*)(buffer+headerSize+2*i*sizeof(int));
                data[i] = key;
                pointers[i] = pointer;
            }
            int pointer = *(int*)(buffer+headerSize+2*i*sizeof(int));
            pointers[i] = pointer;
            delete buffer;
        }
        void save(){
            std::fstream &file = db->getInputStream();
            file.seekp(pageSize*pageNumber+4);
            char buffer[pageSize-4];
            memcpy(buffer, &items,4);
            memcpy(buffer+4, &pageType,sizeof(char));
            for(int i = 0; i < maxSize; i++){
                memcpy(buffer+headerSize-4+2*i*sizeof(int), &pointers[i],sizeof(int));
                memcpy(buffer+headerSize-4+2*i*sizeof(int)+sizeof(int), &data[i],sizeof(int));
            }
            memcpy(buffer+headerSize-4+2*maxSize*sizeof(int), &pointers[maxSize],sizeof(int));
            file.write(buffer, pageSize-4);
        }
        void close(){
            Page *parent = this->parent;
            while(parent != nullptr){
                parent->save();
                Page *current = parent;
                delete parent;
                parent = current->parent;
                delete current; //should be fine since the parent is retained
                current = nullptr; //ill keep it here, but its useless since current leaves scope litterally instantly
            }
            this->save();

        }
        int getItemCount(){
            return this->items;
        }
        int getKeyAt(int index){
            return this->data[index];
        }
        int getPageNumber(){
            return this->pageNumber;
        }
        Page* getParent(){
            return this->parent;
        }
        void setParent(Page* parent){
            this->parent->close();
            delete this->parent;
            this->parent = parent;
        }
        void setPageType(char pageType){
            this->pageType = pageType;
        }
        void addItem(int key,int leftPointer){
            if(items >=maxSize){
                int splitPageNumber = this->db->appendPage();//->-> hehehehe
                Page* parent = nullptr;
                int* lItems = new int[maxSize];
                int* rItems = new int[maxSize];
                int* lPointers = new int[maxSize+1];
                int* rPointers = new int[maxSize+1];    
                int middle;
                int li=0;//left index 
                int ri=0;//right index
                int ti=0; //total index
                bool found = false;
                for(int item : data){
                    if(li+ri < (maxSize+1)/2){ //maxSize+1 because we are also inserting the extra key and pointer
                        if(item > key && !found){
                            lItems[li] = key;
                            lPointers[li] = leftPointer;
                            li++;
                            found = true;
                        }
                        lItems[li] = item;
                        lPointers[li] = pointers[ti];
                        li++;
                    }else{
                        if(ri == 0){
                            if(item > key  && !found){
                                middle = key;
                                lPointers[li] = leftPointer;
                            }
                            else{
                                middle = item;
                                lPointers[li] = pointers[ti];
                            }
                            li++;
                        }
                        if(item > key  && !found){
                            rItems[ri] = key;
                            rPointers[ri] = leftPointer;
                            ri++; 
                            found = true;
                        }
                        rItems[ri] = item;
                        rPointers[ri] = pointers[ti];
                        ri++;
                    }
                    ti++;
                }
                if(!found){
                    rItems[ri] = key;
                    rPointers[ri] = leftPointer;
                    ri++; 
                    found = true;
                }
                rPointers[ri] = pointers[ti];
                if(this->parent != nullptr){
                    parent = this->parent;
                    this->parent;
                    this->parent->addItem(middle,splitPageNumber);
                }else{
                    int newRootPage = this->db->appendPage();
                    this->db->setRootPageLocation(newRootPage);
                    parent = new Page(newRootPage, *this->db, nullptr);
                    this->parent = parent;
                    this->parent->addItem(splitPageNumber,middle,this->pageNumber);
                }
                Page* splitLeft = new Page(splitPageNumber, *this->db, nullptr);
                std::copy(lItems,lItems+maxSize,splitLeft->data);
                std::copy(lPointers,lPointers+maxSize+1,splitLeft->pointers);

                std::copy(rItems,rItems+maxSize,this->data);
                std::copy(rPointers,rPointers+maxSize+1,this->pointers);
                splitLeft->items = (maxSize+1)/2;
                this->items = maxSize-this->items;
                splitLeft->save();
            }else{
                int d[maxSize] = {0};
                int p[maxSize+1] = {0};
                int index = 0;
                bool found = false;
                int i;
                for(i=0;i<this->items;i++){
                    int val = data[i];
                    if(val > key && !found){
                        d[index] = key;
                        p[index] = leftPointer;
                        index++;
                        found = true;
                    }
                    d[index] = this->data[i];
                    p[index] = this->pointers[i];
                    index++;
                }
                if(!found){
                    d[index] = key;
                    p[index] = leftPointer;
                    index++;
                }
                this->items+=1;
                p[index] = this->pointers[i];
                std::copy(d,d+maxSize,this->data);
                std::copy(p,p+maxSize+1,this->pointers);
            }
            if(this->parent != nullptr){
                this->parent->save();
            }
            this->save();
        }
        void addItem(int leftPointer,int key,int rightPointer){
            if(this->items != 0) return;
            this->items++;
            this->data[0] = key;
            this->pointers[0] = leftPointer;
            this->pointers[1] = rightPointer;
        }
        //returns the page number of the next page in a B-Tree search, returns the current page if an insert would occure in the current page
        int getNextPageNumber(int key){
            for (int i = 0; i < items; i++) {
                if (key == data[i]){ 
                    if(pointers[i+1] == 0){//pointers[i+1] is the right pointer, pointer[i] is the left pointer 
                       return this->pageNumber; 
                    }else{
                        return pointers[i+1];
                    }
                }else if(key < data[i]){ //0 pointer means that the pointer doesnt exist, since 0 is always the header page, which is invalid
                    if(this->pointers[i]  != 0) return this->pointers[i];
                    return this->pageNumber;
                }
            }
            if(this->pointers[items] != 0){
                return this->pointers[items];
            }else{
                return this->pageNumber;
            }
        }
        void print(){
            std::cout << "page number:"<<this->pageNumber << std::endl;
            std::cout << "items:"<<this->items << std::endl;
            std::cout << "page type:"<<this->pageType << std::endl;
            for(int i = 0; i < this->items; i++){
                std::cout << "pointers["<<i<<"]:"<<this->pointers[i] << std::endl;
                std::cout << "data["<<i<<"]:"<<this->data[i] << std::endl;
            }
            std::cout << "pointers["<<this->items<<"]:"<<this->pointers[this->items] << std::endl;
        }
    };
    File(std::string name){
        inputstream = std::fstream(name, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
        this->fileName = name;
        this->pages = getPageCount();
        std::cout << "pages:"<<pages << std::endl;
        if (pages == 0){
            createPage(0);
            createPage(1);
            createPage(2);
        }
        if (!inputstream) {
        std::cerr << "Failed to open file.\n";
        }
        inputstream.seekg(headerSize);
        char *buffer = new char[sizeof(unsigned long long int)];
        inputstream.read(buffer, sizeof(unsigned long long int));
        this->totalItems = *(unsigned long long int*)buffer;
        delete buffer;
    }
    //returns the fstream of the file
    std::fstream &getInputStream(){
        return this->inputstream;
    }
    void setRootPageLocation(int pageNumber){
        this->rootPage = pageNumber;
    }
    int getRootPageLocation(){
        return this->rootPage;
    }
    //you need to delete the buffer after you are done with it
    char *getPage(int pageNumber){
        inputstream.seekg(pageNumber*pageSize);
        char *buffer = new char[pageSize];
        inputstream.read(buffer, pageSize);
        return buffer;
    }
    Page* findPageOf(int key){
        int pageNumber = this->rootPage;
        Page *current = new Page(pageNumber, *this,nullptr);
        while (current->getPageNumber() != current->getNextPageNumber(key)){
            pageNumber = current->getNextPageNumber(key);
            (*current).save(); //should do nothing for now;
            current = new Page(pageNumber, *this,current);
        }
        (*current).save();
        return current;
    }
    void insert(int key){
        Page *page = findPageOf(key);
        //if(page->getItemCount() < maxSize){
              page->addItem(key,0);
              //(*page).print();
              page->save();
              delete page;
              this->totalItems++;
        //}
        //(*page).print();
    }
    void writePage(int pageNumber, char *buffer){
        inputstream.seekp(pageNumber*pageSize);
        inputstream.write(buffer, pageSize);
    }
    friend std::ostream& operator<<(std::ostream& c, File &f){
        std::fstream &fileStream = f.getInputStream();
        double fileSize = f.getFileSize();
        fileStream.seekg(0); // Byte 0 of the file, yes only byte increments can be moved at a time
        char *buffer = new char[(int)fileSize];
        fileStream.read(buffer, fileSize);
        for (int i = 0; i < fileSize; i++) {
            std::cout << buffer[i];
        }
        delete buffer;
        return c;
    }
    double getFileSize(){
        this->inputstream.seekg(0, std::ios::end);
        double fileSize = (double)this->inputstream.tellg();
        return fileSize;
    }  
    void close(){
        this->inputstream.seekp(headerSize); //because the page is 0 so i only need to search by header Size
        inputstream.write(reinterpret_cast<char*>(&totalItems), sizeof(unsigned long long int));
        this->inputstream.close();
    }
    unsigned long long int getTotalItems(){
        return this->totalItems;
    }
    int getPageCount(){
        return getFileSize()/pageSize;
    }
    //flushes the database and resets the total items to 0
    void flush(){
        this->totalItems = 0;
        this->inputstream.close();
        std::ofstream file(this->fileName, std::ios::trunc);file.close(); //deletes the entire file
        *this = File(this->fileName);
    }
};

int main(){
    srand(time(NULL));
    File db("example.db");
    db.flush();
    int num = db.getTotalItems();
    int n = 509; //works up to 509 pages of data
    for (int i = 0; i < maxSize*n+n; i++) {
        db.insert(num);
        num = db.getTotalItems();
    }
    int r = db.getRootPageLocation();
    File::Page* root = new File::Page(r,db,nullptr);
    std::cout << db.findPageOf(510)->getPageNumber() << std::endl;
    root->print();
    std::cout << "item count: "<< num << std::endl;
    // double fileSize = db.getFileSize();
    // std::cout << "maxSize:"<<maxSize << std::endl; // maxsize
    // std::cout << "minSize:"<<minSize << std::endl; // minsize
    // std::cout << "file size:"<<fileSize << std::endl; // filesize
    // int num = db.getTotalItems();
    // std::cout << db;
    // std::cout << "\nSuccessfully overwrote first "<<db.getFileSize()<<" bytes of example.txt with space." << std::endl;
    // std::cout << num;
    db.close();  
    return 0;
}


//todo

// implement inserting. searching should be done.