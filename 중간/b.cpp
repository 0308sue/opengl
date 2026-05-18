#include <iostream>

using namespace std;
int main()
{
    cout << "Hello, OpenGL!" << endl;
    int a = 5;
    int *p = &a;
    cout << *p << endl;

    int list[5] = {1, 2, 3, 4, 5};
    int *ptr = list; // 배열 이름은 배열의 첫 번째 요소의 주소를 가리킴
    cout << *(ptr+2) << endl; //3
    return 0;
}