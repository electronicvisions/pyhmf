#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
extern "C" {
#include <fcntl.h>
}

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "logging.cpp"


char const* filename = "logging.out";

std::string const teststring1 = "This is a newline-terminated teststring.\n";
std::string const teststring2 = "Newline too!\n";
std::string const teststring3 = "This is just another teststring.";

void test() {
	int fd = open(filename, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR); // boost changes umask???
	if (fd == -1)
		throw std::runtime_error("Could not open logging.out");

	BOOST_TEST_MESSAGE("Testing redirection of stdout to some other file handle (" << filename << ")...");

	redirectStdout(fd); // FUNCTION TO TEST :)
	close(fd);

	printf("%s", teststring1.c_str());
	std::cout << teststring2;
	printf("%s", teststring3.c_str());

	redirectStdout(fd); // and switching back

	printf("... and we're back to console!\n");
}

void checkfile(std::string const expect) {
	std::ifstream file(filename);
	std::stringstream content;
	content << file.rdbuf();
	file.close();
	remove(filename);

	BOOST_REQUIRE( content.str() == expect );
}



BOOST_AUTO_TEST_SUITE( test_logging_stuff )

BOOST_AUTO_TEST_CASE( test_redirectStdout )
{
	test();
	checkfile(teststring1 + teststring2 + teststring3);
}

BOOST_AUTO_TEST_SUITE_END()
