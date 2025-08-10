#include <iostream>
#include <fstream>  // For file handling 
#include <cstdint>  // for uint8_t
#include <chrono>
#include <time.h>       /* time */
#include <random>
const int headerSize = 13;
const int expectedMaxItems = 4;
const int pageSize = headerSize+expectedMaxItems*sizeof(int)+expectedMaxItems*2*sizeof(int)+4; //1<<12;
const int maxSize = (pageSize-headerSize)/sizeof(int)/2 -    1; //div 2 -1 because we need to store the pointers aswell and theres 1 pointers per item + 1 pointer
const int minSize = maxSize/2;
int maxDepth = 0;
//header explained 
//first 4 bytes are the page number
//next 4 bytes are the number of items in the page ----- stores the root page location if the header page
//next 1 byte is the page type l for leaf r for root b for branch and  g for garbage, h for header page (located at page 0)
//next 4 bytes is the parent page number

//root page
//first 4 bytes are page number 0 //identifier
//next 4 bytes are the root page location
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
        int parentPage = 0;
        file.write(reinterpret_cast<char*>(&parentPage), sizeof(int));
        if (pageType == 'h') {
            int totalItems = 0;
            file.write(reinterpret_cast<char*>(&totalItems), sizeof(unsigned long long int));
            dHeaderSize = dHeaderSize+sizeof(unsigned long long int);
        }
        uint8_t zero_byte = 0x00;
        file.seekp(pageNumber*pageSize +dHeaderSize);
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
        char pageType;
        File *db;
        Page *parent;
        public:
        int parentPage = 0;
        int data[maxSize];
        int pointers[maxSize+1];
        Page(int pageNumber, File &db, Page *parent){
            this->parent = parent;
            this->db = &db;
            this->pageNumber = pageNumber;
            char *buffer = db.getPage(pageNumber);
            this->items = *(int*)(buffer+4);
            this->pageType = *(char*)(buffer+8); //+8 beacuse the first 8 bytes are the page number and item cout
            this->parentPage = *(int*)(buffer+9);
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
            memcpy(buffer+5, &parentPage,sizeof(int));
            for(int i = 0; i < maxSize; i++){
                memcpy(buffer+headerSize-4+2*i*sizeof(int), &pointers[i],sizeof(int));
                memcpy(buffer+headerSize-4+2*i*sizeof(int)+sizeof(int), &data[i],sizeof(int));
            }
            memcpy(buffer+headerSize-4+2*maxSize*sizeof(int), &pointers[maxSize],sizeof(int));
            file.write(buffer, pageSize-4);
        }
        void close(){
            Page *parent = this->getParent();
            while(parent != nullptr){
                parent->save();
                Page *current = parent;
                delete parent;
                parent = current->getParent();
                delete current; //should be fine since the parent is retained
                current = nullptr; //ill keep it here, but its useless since current leaves scope litterally instantly
            }
            this->save();
        }
        ~Page() {
            this->db = nullptr;
            this->parent = nullptr;
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
            if(this->parentPage != 0 && this->parent == nullptr){
                this->parent = new Page(this->parentPage, *this->db, nullptr);
            }
            return this->parent;
        }
        void setParent(Page* parent){
            if(this->parent != nullptr) this->parent->close();
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
                if(this->parentPage != 0){
                    parent = this->parent;
                    this->getParent()->addItem(middle,splitPageNumber);
                }else{
                    int newRootPage = this->db->appendPage();
                    this->db->setRootPageLocation(newRootPage);
                    parent = new Page(newRootPage, *this->db, nullptr);
                    this->parent = parent;
                    this->parentPage = this->parent->pageNumber;
                    this->parent->addItem(splitPageNumber,middle,this->pageNumber);
                }
                Page* splitLeft = new Page(splitPageNumber, *this->db, nullptr);
                std::copy(lItems,lItems+maxSize,splitLeft->data);
                std::copy(lPointers,lPointers+maxSize+1,splitLeft->pointers);

                std::copy(rItems,rItems+maxSize,this->data);
                std::copy(rPointers,rPointers+maxSize+1,this->pointers);
                delete rItems;
                delete rPointers;
                delete lItems;
                delete lPointers;
                splitLeft->items = (maxSize+1)/2;
                this->items = maxSize-ri+1;
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
        inputstream.seekg(4);
        char *buffer = new char[sizeof(int)];
        inputstream.read(buffer, sizeof(int));
        this->rootPage = *(int*)buffer;
        delete buffer;
        inputstream.seekg(headerSize);
        buffer = new char[sizeof(unsigned long long int)];
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
        int depth = -1;
        while (current->getPageNumber() != current->getNextPageNumber(key)){
            pageNumber = current->getNextPageNumber(key);
            //(*current).save(); //should do nothing for now;
            //current->close();
            Page* prev = current;
            current = new Page(pageNumber, *this,nullptr);
            delete prev;
            depth++;
        }
        //(*current).save();
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
        this->inputstream.seekp(4);
        this->inputstream.write(reinterpret_cast<char*>(&rootPage), sizeof(int));
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
    void printAll(int pageNumber){
        Page* page = new Page(pageNumber, *this,nullptr);
        int itemCount = page->getItemCount();
        for(int i =0;i<itemCount;i++){
            if(page->pointers[i] >0){
                printAll(page->pointers[i]);   
            }
            std::cout << page->data[i] << ",";
        }
        if(page->pointers[itemCount] >0){
            printAll(page->pointers[itemCount]);   
        }
    }
};


//main used to generate random data insert
int main(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(INT_MIN,INT_MAX);
    File db("example.db");
    std::ofstream file2("data.csv", std::ios::app); //storing the data
    db.flush();
    int num = db.getTotalItems();
    for (int a = 0; a < 1324; a++)
    {
        int newItems = num*0.01;
        for (int i = 0; i < newItems; i++) {
            db.insert(dis(gen)); //num
            num +=1;
        }
        auto start = std::chrono::high_resolution_clock::now();
        db.insert(dis(gen)); //num
        auto end = std::chrono::high_resolution_clock::now();
        num +=1;
        double duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
        file2 << num <<","<< duration << std::endl;
        std::cout << num <<","<< duration << std::endl;
    }
    
    int r = db.getRootPageLocation();
    File::Page* root = new File::Page(r,db,nullptr);
    std::cout << "pages:" << db.getPageCount() << std::endl;
    std::cout << "maxSize:"<<maxSize << std::endl; // maxsize
    std::cout << "Printing All" << std::endl;
    //db.printAll(r);
    std::cout << std::endl;
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

//main function used to generate linear data insert.
// int main(){
//     srand(time(NULL));
//     File db("example.db");
//     std::ofstream file2("data.csv", std::ios::app); //storing the data
//     db.flush();
//     int num = db.getTotalItems();
//     for (int a = 0; a < 1324; a++)
//     {
//         int newItems = num*0.01;
//         for (int i = 0; i < newItems; i++) {
//             db.insert(num);
//             num +=1;
//         }
//         auto start = std::chrono::high_resolution_clock::now();
//         db.insert(num);
//         auto end = std::chrono::high_resolution_clock::now();
//         num +=1;
//         double duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
//         file2 << num <<","<< duration << std::endl;
//         std::cout << num <<","<< duration << std::endl;
//     }
    
//     int r = db.getRootPageLocation();
//     File::Page* root = new File::Page(r,db,nullptr);
//     std::cout << "pages:" << db.getPageCount() << std::endl;
//     std::cout << "maxSize:"<<maxSize << std::endl; // maxsize
//     std::cout << "Printing All" << std::endl;
//     //db.printAll(r);
//     std::cout << std::endl;
//     std::cout << "item count: "<< num << std::endl;
//     // double fileSize = db.getFileSize();
//     // std::cout << "maxSize:"<<maxSize << std::endl; // maxsize
//     // std::cout << "minSize:"<<minSize << std::endl; // minsize
//     // std::cout << "file size:"<<fileSize << std::endl; // filesize
//     // int num = db.getTotalItems();
//     // std::cout << db;
//     // std::cout << "\nSuccessfully overwrote first "<<db.getFileSize()<<" bytes of example.txt with space." << std::endl;
//     // std::cout << num;
//     db.close();  
//     return 0;
// }