// #include <iostream>
// #include <vector>
// #include <lib/argparser.h>

// static const size_t kMaxArgLen = 128;

// struct Options {
//     bool sum = false;
//     bool mult = false;
// };

// bool IsEven(const int& value) {
//     return value % 2 == 0;
// }

// bool Check(const char* const& str) {
//     return std::strlen(str) < 5;
// }

// bool IsValidEmail(const char* const& value) {
//     return (std::strchr(value, '@') != nullptr) && (std::strlen(value) > 3);
// }

// bool IsAlphaOnly(const char* const& value) {
//     if (std::strlen(value) == 0) return false;

//     for (const char* c = value; *c != '\0'; ++c) {
//         if (!std::isalpha(*c)) return false;
//     }
//     return true;
// }

// bool IsNotEmpty(const char* const& value) {
//     return std::strlen(value) != 0;
// }

// bool IsValidPort(const int& value) {
//     return value > 0 && value <= 65535;
// }

// bool IsPositiveFloat(const float& value) {
//     return value > 0.0f;
// }

// bool IsPositive(const int& value) {
//     return value > 0;
// }

int main() {
//     // Options options;
//     // int value;
//     // std::vector<int> values;

//     // nargparse::ArgumentParser parser = nargparse::CreateParser("labwork3");

//     // nargparse::AddFlag(parser, "-s", "--sum", &options.sum, "Summarise");
//     // nargparse::AddFlag(parser, "-m", "--mult", &options.mult, "Multiply");
//     // nargparse::AddArgument(parser, &value, "values", nargparse::kNargsZeroOrMore, IsEven, "only even numbers");
//     // nargparse::AddHelp(parser);

//     // if(!nargparse::Parse(parser, argc, argv)) {
//     //     nargparse::FreeParser(parser);
//     //     nargparse::PrintHelp(parser);
//     //     return 1;
//     // }


//     // int count = nargparse::GetRepeatedCount(parser, "values");
//     // for (int i = 0; i < nargparse::GetRepeatedCount(parser, "values"); ++i) {
//     //     if (nargparse::GetRepeated(parser, "values", i, &value)) {
//     //         values.push_back(value);
//     //     }
//     // }

//     // nargparse::FreeParser(parser);

//     // int result = 0;
//     // if(options.sum) {
//     //     for(int i = 0; i < values.size(); ++i) {
//     //         result += values[i];
//     //     }
//     // } else if(options.mult) {
//     //     result = 1;
//     //     for(int i = 0; i < values.size(); ++i) {
//     //         result *= values[i];
//     //     }
//     // }

//     // std::cout << "Result: " << result << std::endl;

//     nargparse::ArgumentParser parser = nargparse::CreateParser("NIVON", 128);

//     int value = 0;

//     nargparse::AddHelp(parser);

//     const char* argv[] = {"program", "--help"};
//     if (nargparse::Parse(parser, 2, argv)) {
//         std::cout << "SUCCESS\n------\n";
        
//     } else {
//         std::cout << "NOOOOOO\n";
//         nargparse::AddHelp(parser);
//         nargparse::PrintHelp(parser);
//     }

//     nargparse::FreeParser(parser);

    return 0;
}