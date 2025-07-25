#include <iostream> //for cout
#include <string> //for tostring
#include <cmath> //for pow sqrt etc
using namespace std; //fuses std scope into the current scope
using std::cout;// imports cout into the current scope
typedef std::string str; // use str instead of std::string for strings
using str = std::string; // typedef but using using :)
//:: is the scope resolution operator
// so ::variable is the global scope variable called variable
//allows for count << " "; instead of std::cout << " ";

enum nameOfEnum {value1 = 20, value2 = 30, value3 = 20,uniqueValue = 500}; // can only use intigers as values
nameOfEnum var2 = uniqueValue; //using enums
//function templates

int max(int a, int b){ // without template
    return a>b?a:b;
}
template<typename T> //defines T as a template Type
T max (T a, T b){  // allows max(int,int) and max(float,float) etc but not max(int,float)
    return a>b?a:b;
}
template<typename T, typename U>
auto max(T a, U b){ // allows max(int,float) and max(float,int) etc 
    return a>b?a:b;
}

//classes / structs

//struct members and base classes/structs are public by default.
//class members and base classes/structs are private by default.
struct person{
    std::string name;
    int age;
    int birthDay; //1->357 days
    std::string gender;
    person(){
        name = "default";
        age = 0;
        birthDay = 0;
        gender = "default";
        height = 0;
        weight = 0;
    }
    int getHeight(){
        return height;
    }
    int getWeight(){
        return weight;
    }
    void setHeight(int height){
        this->height = height;
    }
    void setWeight(int weight){
        this->weight = weight;// -> is a member access operator
    }
    private: //indent doesnt matter here, so make shure not to declare anythign after private:
        int height;
        int weight;
    
}; //yes you need to ; at the end of structs, its kinda dumb

class dog{
    public: //public members
        std::string name;
        person *owner;
        int age;
        void eat(){
            std::cout << "eating\n";
        }
        void sleep(){
            std::cout << "sleeping\n";
        }
        void bark(){
            std::cout << "barking\n";
        }
        void ageUp(){
            dog::age++;
        }
        dog(std::string n, int a, person *p ){
            name = n;
            age = a;    
            owner = p;
        }
};

class chiwawa : public dog{
    bool isOwnedByQueen;
    chiwawa(std::string n, int a, person *p, bool isQ): dog("chiwawa "+n,a,p){ // initialized dog, but prefferably have default constructor for dog
        isOwnedByQueen = isQ;
    }
};
// person *joe = new person; // creates memory adress for new person
//person joe = new person; //would not work
void swap(int &a, int &b);
namespace mySpaceCool {
    bool bool1 = false;
    int func(int num) {
        return num;
    }
}

int main() { //function gets called by the executor
    using namespace mySpaceCool; //using a namespace
    bool bool1 = true; //variable declaration
    std::cout << (bool1? "true" : "false"); //prioritises 

    int aaaaqaaa = mySpaceCool::func(10); //calling a function


    int num = 10; // how to declare a variable
    std::cout << "Hello My Friend"; // how to print a variable
    std::cout << num; // streaming a number into the console
    std::cout << '\n'; // printing new line
    printf("line #2 %i\n",num); // another method of printing variables & stuff

    //data types
    bool boolVar = true; //1 byte
    char charVar = 'a'; //1 byte
    short shortVar = 10;  //2 bytes
    int intVar = 10; //4 bytes
    long longVar = 10; //4 bytes
    float floatVar = 10.0; //4 bytes
    double doubleVar = 10.0; //8 bytes
    std::string stringVar = "Hello World"; // .length() bytes
    char string[] = "hello world"; // length of array bytes

    //Const
    const int constInt = 10; //cannot be changed
    // can also be in function declarations like func(const int num);


    //data type conversion --litterally the same as in Java

    double pi = 3.14159265358979323846;
    int piInt = (int)pi; //explicit casting
    int piInt2 = pi; //implicit casting
    std::string piStr = std::to_string(pi); //explicit tostring


    std::string in = "";
    std::cin >> in; //input



    // if statements
    int a = 10;
    int b = 20;
    bool expression = a > b;
    bool expression2 = a + b > 0;
    //comparisons
    // >,<,>=,<=,==,!=

    if(expression){
        // do something
    }else if (expression2)
    {
        // else if
    }else{
        // do something else
    }
    //inline if
    int c = a>b?a:b; //inline max function kinda
    //bitwhise operators
    // or = |
    // and = &
    // xor = ^
    // not = !
    // left shift = <<
    // right shift = >>
    // logical and = &&
    // logical or = ||
    int month = 10;
    std::string monthStr = "";
    switch (month){
        case 0:
            monthStr = "January";
            break;
        case 1:
            monthStr = "February";
            break;
        case 2:
            monthStr = "March";
            break;
        case 3:
            monthStr = "April";
            break;
        case 4:
            monthStr = "May";
            break;
        case 5:
            monthStr = "June";
            break;
        case 6:
            monthStr = "July";
            break;
        case 7:
            monthStr = "August";
            break;
        case 8:
            monthStr = "September";
            break;
        case 9:
            monthStr = "October";
            break;
        case 10:
            monthStr = "November";
            break;
        case 11:
            monthStr = "December";
            break;
        default:
            monthStr = "Invalid Month";
            break;
    }



    std::string name = "Gustav";
    std::cout << name[0] << name[1] <<'\n'; // name[0]+name[1] seas them as a byte
    int array[] = {1,2,3,4,5,6,7};
    int array2[5];
    for(int i = 0; i < 5; i++){
        array2[i] = array[i];
    }
    int i = 0;
    for (int val : array2){ // foreach loop
        array2[i] = val*2;
        i++;
    }
    std::cout << sizeof(array2) << " bytes of data in array2\n"; // returns the bytes of an array
    //filling array
    std::string toBeFilled[5];
    //fill(startingPointer,endingPointer,value)
    fill(toBeFilled,toBeFilled+sizeof(toBeFilled)/sizeof(std::string),"filling a string");
    for(std::string str:toBeFilled){
        std::cout << str << '\n';
    }

    //pointers &variable
    //arr == &arr[0] //because array gets implicitly converted to a pointer of the first element
    //arr + 5 == &arr[0]+5 == &arr[5] //because the pointer addition adds the size of the type
    
    // pointers
    // & address of operator
    int var = 100;
    std::cout << "the adress of var is " << &var << '\n';
    int foo = 10;
    int bar = 20;
    swap(foo,bar); // will swap the values of at those memory adresses
    std::cout << "foo is now " << foo << " and bar is now " << bar << '\n';
    
    // * dereference operator
    int n = 500;
    int *pn = &n; // pnum is the memory adress of num
    std::cout << "the adress of num is " << pn << '\n';
    std::cout << pn + 5 << '\n'; // pointer moves 5 ints forward
    std::cout << *pn + "\n"; // gets the value of the memory adress pn
    
    //nullptr
    nullptr; //is the memory adress of null


    //dynamic memory allocation
    int *ptr = new int; //creates a memory adress of size 4 bytes

    *ptr = 10; //sets the value of the memory adress ptr to 10
    std::cout << *ptr << '\n'; //prints the value of the memory adress ptr
    delete ptr;
    std::cout << *ptr << '\n'; //prints out basically garbage

    return 0; 
}
void swap(int &a, int &b){ // pass by referance
    int temp = a;
    a = b;
    b = temp;
}