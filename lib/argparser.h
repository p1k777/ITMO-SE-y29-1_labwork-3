#pragma once

#include <iostream>

namespace nargparse {
    struct Container {
        size_t size = 0;
        size_t capacity = 0;
        void** data = nullptr;
    };

    void ContRealloc(Container&);
    void ContPushBack(Container&, void*);

    struct ArgumentParser {
        enum class ArgsN {
            kOptional = 0,
            kRequired = 1,
            kZeroOrMore = 2,
            kOneOrMore = 3,
        };

        char* name = nullptr;
        size_t max_arg_len = 0;
        char* help = nullptr;

        // всё для флагов (односвязный список)
        struct Flag {
            bool* value_ptr = nullptr;
            char* info = nullptr;
            char* short_flag = nullptr;
            char* long_flag = nullptr;
        };
        Container flags{0, 0, nullptr};

        // енум класс для типов аргументов (дин массив)
        enum class ValueType {
            kNone = 0,
            kInt = 1,
            kFloat = 2,
            kString = 3,
        };

        union Validator {
            bool (*int_validator)(int const&) = nullptr;
            bool (*float_validator)(float const&);
            bool (*string_validator)(const char* const&);
        };

        // всё для позиционных аргументов (дин массив)
        struct PosArg {
            Container values;
            void* connector = nullptr;
            ValueType value_type = ValueType::kNone;
            char* info = nullptr;
            ArgsN values_count = ArgsN::kRequired;
            Validator validator;
            char* criteria = nullptr;
        };

        Container pos_args;

        // всё для именованных аргументов (дин массив)
        struct NamedArg {
            Container values;
            void* connector = nullptr;
            ValueType value_type = ValueType::kNone;
            char* short_flag = nullptr;
            char* long_flag = nullptr;
            char* info = nullptr;
            ArgsN values_count = ArgsN::kRequired;
            Validator validator;
            char* criteria = nullptr;
        };
        Container named_args;
    };

    const ArgumentParser::ArgsN kNargsOptional = ArgumentParser::ArgsN::kOptional;
    const ArgumentParser::ArgsN kNargsRequired = ArgumentParser::ArgsN::kRequired;
    const ArgumentParser::ArgsN kNargsZeroOrMore = ArgumentParser::ArgsN::kZeroOrMore;
    const ArgumentParser::ArgsN kNargsOneOrMore = ArgumentParser::ArgsN::kOneOrMore;

    ArgumentParser CreateParser(const char* name, size_t max_arg_len=0);

    void AddFlag(ArgumentParser& parser,
                 const char* short_flag,
                 const char* long_flag,
                 bool* value_ptr,
                 const char* info,
                 bool default_val=false);

    void AddArgument(ArgumentParser& parser,
                     int* value_ptr,
                     const char* info,
                     ArgumentParser::ArgsN values_count=nargparse::ArgumentParser::ArgsN::kRequired,
                     bool (*validator)(int const&)=nullptr,
                     const char* criteria=nullptr);
    void AddArgument(ArgumentParser& parser,
                     const char* short_flag,
                     const char* long_flag,
                     int* value_ptr,
                     const char* info, 
                     ArgumentParser::ArgsN values_count=nargparse::ArgumentParser::ArgsN::kRequired,
                     bool (*validator)(int const&)=nullptr,
                     const char* criteria=nullptr);
    
    void AddArgument(ArgumentParser& parser,
                     float* value_ptr,
                     const char* info,
                     ArgumentParser::ArgsN values_count=ArgumentParser::ArgsN::kRequired,
                     bool (*validator)(float const&)=nullptr,
                     const char* criteria=nullptr);
    void AddArgument(ArgumentParser& parser,
                     const char* short_flag,
                     const char* long_flag,
                     float* value_ptr,
                     const char* info, 
                     ArgumentParser::ArgsN values_count=ArgumentParser::ArgsN::kRequired,
                     bool (*validator)(float const&)=nullptr,
                     const char* criteria=nullptr);

    void AddArgument(ArgumentParser& parser,
                     char (*value_ptr)[],
                     const char* info,
                     ArgumentParser::ArgsN values_count=ArgumentParser::ArgsN::kRequired,
                     bool (*validator)(const char* const&)=nullptr,
                     const char* criteria=nullptr);
    void AddArgument(ArgumentParser& parser,
                     const char* short_flag,
                     const char* long_flag,
                     char (*value_ptr)[],
                     const char* info, 
                     ArgumentParser::ArgsN values_count=ArgumentParser::ArgsN::kRequired,
                     bool (*validator)(const char* const&)=nullptr,
                     const char* criteria=nullptr);

    bool Parse(ArgumentParser& parser, size_t argc, const char* argv[]);
    
    size_t GetRepeatedCount(const ArgumentParser& parser, const char* info);
    bool GetRepeated(ArgumentParser& parser, const char* info, size_t idx, void* target);

    void AddHelp(ArgumentParser& parser);
    void PrintHelp(ArgumentParser& parser);

    void FreeParser(ArgumentParser& parser);
}