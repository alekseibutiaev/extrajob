#include <iostream>

class test {
	public:
	int a = 0;
};

int main(int ac, char* av[])
{
	test t;
	t.a = 50;
	std::cout << t.a << std::endl;
  return 0;
}