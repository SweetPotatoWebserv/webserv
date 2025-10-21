#include <cctype>
#include <iostream>
#include <string>

std::string toUpper(const std::string& str) {
    std::string result;

    for (const unsigned char character : str) {
        result += static_cast<char>(std::toupper(character));
    }

    return result;
}

int main(int argc, char** argv) {
    std::string upper;

    if (argc == 1) {
        std::cout << "* LOUD AND UNBEARABLE FEEDBACK NOISE *" << '\n';
        return 0;
    }
    for (int i = 1; i < argc; i++) {
        upper += toUpper(argv[i]);
        if (i != argc - 1) upper += " ";
    }
    std::cout << upper << '\n';
    return 0;
}
