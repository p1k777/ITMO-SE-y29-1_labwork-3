#include <cstring>
#include <cstdint>
#include "argparser.h"

namespace {
    nargparse::ArgumentParser::Flag* GetFlagPtr(const nargparse::ArgumentParser& parser, const char* flag, bool is_short) {
        if (!flag) {
            return nullptr;
        }

        using Flag = nargparse::ArgumentParser::Flag;
        if (is_short) {
            for (size_t i = 0; i < parser.flags.size; i++) {
                Flag* curr = ((Flag*)(parser.flags.data[i]));
                if (curr->short_flag && !strcmp(curr->short_flag, flag)) {
                    return curr;
                }
            }
        } else {
            for (size_t i = 0; i < parser.flags.size; i++) {
                Flag* curr = ((Flag*)(parser.flags.data[i]));
                if (curr->long_flag && !strcmp(curr->long_flag, flag)) {
                    return curr;
                }
            }
        }

        return nullptr;
    }

    nargparse::ArgumentParser::PosArg* GetPosArgPtr(const nargparse::ArgumentParser& parser, nargparse::ArgumentParser::ValueType value_type) {
        using PosArg = nargparse::ArgumentParser::PosArg;
        for (size_t i = 0; i < parser.pos_args.size; i++) {
            PosArg* curr = (PosArg*)(parser.pos_args.data[i]);
            if (curr && curr->value_type == value_type) {
                return curr;
            }
        }

        return nullptr;
    }
    nargparse::ArgumentParser::NamedArg* GetNamedArgPtr(const nargparse::ArgumentParser& parser, const char* flag, bool is_short) {
        if (!flag) {
            return nullptr;
        }

        using NamedArg = nargparse::ArgumentParser::NamedArg;
        if (is_short) {
            for (size_t i = 0; i < parser.named_args.size; i++) {
                NamedArg* curr = (NamedArg*)parser.named_args.data[i];
                if (curr->short_flag && !std::strcmp(curr->short_flag, flag)) {
                    return curr;
                }
            }
        } else {
            for (size_t i = 0; i < parser.named_args.size; i++) {
                NamedArg* curr = (NamedArg*)parser.named_args.data[i];
                if (curr->long_flag && !std::strcmp(curr->long_flag, flag)) {
                    return curr;
                }
            }
        }

        return nullptr;
    }

    void AddPosArg(nargparse::ArgumentParser& parser,
                   void* value_ptr,
                   const char* info, 
                   nargparse::ArgumentParser::ArgsN values_count,
                   nargparse::ArgumentParser::Validator validator,
                   const char* criteria,
                   nargparse::ArgumentParser::ValueType value_type) {
        
        if (!info) {
            std::cout << "[ERROR]: No info about pos arg\n";
            return;
        }

        using PosArg = nargparse::ArgumentParser::PosArg;
        PosArg* new_arg = new PosArg;

        *new_arg = {
            .values = nargparse::Container{0, 0, nullptr},
            .connector = value_ptr,
            .value_type = value_type,
            .info = nullptr,
            .values_count = values_count,
            .validator = validator,
            .criteria = nullptr,
        };

        new_arg->info = new char[std::strlen(info) + 1];
        std::strcpy(new_arg->info, info);

        if (criteria) {
            size_t len = std::strlen(criteria);
            new_arg->criteria = new char[len + 1];
            std::strcpy(new_arg->criteria, criteria);
        }

        nargparse::ContPushBack(parser.pos_args, new_arg);
    }
    void AddNamedArg(nargparse::ArgumentParser& parser,
                     const char* short_flag,
                     const char* long_flag,
                     void* value_ptr,
                     const char* info, 
                     nargparse::ArgumentParser::ArgsN values_count,
                     nargparse::ArgumentParser::Validator validator,
                     const char* criteria,
                     nargparse::ArgumentParser::ValueType value_type) {

        if (!info) {
            std::cout << "[ERROR]: No info about named arg\n";
            return;
        }
        if (!short_flag && !long_flag) {
            std::cout << "[ERROR]: No flag for named arg\n";
            return;
        }
        if (GetNamedArgPtr(parser, short_flag, true)) {
            std::cout << "[ERROR]: named arg using \"" << short_flag << "\" already exists\n";
            return;
        }
        if (GetNamedArgPtr(parser, long_flag, false)) {
            std::cout << "[ERROR]: named arg using \"" << long_flag << "\" already exists\n";
            return;
        }

        using NamedArg = nargparse::ArgumentParser::NamedArg;
        NamedArg* new_arg = new NamedArg;

        *new_arg = {
            .values = nargparse::Container{0, 0, nullptr},
            .connector = value_ptr,
            .value_type = value_type,
            .short_flag = nullptr,
            .long_flag = nullptr,
            .info = nullptr,
            .values_count = values_count,
            .validator = validator,
            .criteria = nullptr,
        };

        new_arg->info = new char[std::strlen(info) + 1];
        std::strcpy(new_arg->info, info);

        if (short_flag) {
            new_arg->short_flag = new char[std::strlen(short_flag) + 1];
            std::strcpy(new_arg->short_flag, short_flag);
        }

        if (long_flag) {
            new_arg->long_flag = new char[std::strlen(long_flag) + 1];
            std::strcpy(new_arg->long_flag, long_flag);
        }

        if (criteria) {
            new_arg->criteria = new char[std::strlen(criteria) + 1];
            std::strcpy(new_arg->criteria, criteria);
        }

        nargparse::ContPushBack(parser.named_args, (void*)new_arg);
    }

    enum class ArgType {
        kNone = 0,
        kFlag = 1,
        kNamedArgFlag = 2,
        kInt = 3,
        kFloat = 4,
        kString = 5,
        kNamedArgWithVal = 6,
    };

    nargparse::ArgumentParser::ValueType GetValueType(const char* arg, size_t arg_size) {
        using ValueType = nargparse::ArgumentParser::ValueType;
        ValueType result = ValueType::kInt;
        for (size_t i = (arg[0] == '-'); i < arg_size; i++) {
            if (!std::isdigit(arg[i])) {
                // если встречена не точка (или уже 2-я точка) или не буква f (или она не последняя)
                if ((arg[i] != '.' || result == ValueType::kFloat) && (arg[i] != 'f' || i + 1 < arg_size)) { return ValueType::kString; }
                
                result = ValueType::kFloat;
            }
        }

        return result;
    }

    ArgType GetArgType(const nargparse::ArgumentParser& parser, size_t argc, const char* argv[], size_t idx, size_t arg_size) {
        if (!arg_size || arg_size == 1 && !std::isalnum(argv[idx][0])) { return ArgType::kNone; }

        if (arg_size < 2 || std::isalnum(argv[idx][0]) || argv[idx][0] == '.' || argv[idx][0] == '/') {
            if (std::isalpha(argv[idx][0]) || argv[idx][0] == '/') {
                return ArgType::kString;
            }

            // return GetNumberType(argv[idx], arg_size);
        }

        if (argv[idx][0] != '-') { return ArgType::kNone; }

        if (std::isdigit(argv[idx][1])) {
            // return GetNumberType(argv[idx], arg_size);
        }

        if (argv[idx][1] == '-' && arg_size == 2) { return ArgType::kNone; }

        if (argv[idx][1] != '-' && arg_size > 2) {
            for (size_t i = 0; i < arg_size; i++) {
                if (argv[idx][i] == '=') {
                    return ArgType::kNamedArgWithVal;
                }
            }

            return ArgType::kFlag;
        }

        if (idx + 1 >= argc || argv[idx + 1][0] == '-' && (argv[idx + 1][1] == '-' || std::isalpha(argv[idx + 1][1]))) {
            for (size_t i = 0; i < arg_size; i++) {
                if (argv[idx][i] == '=') {
                    return ArgType::kNamedArgWithVal;
                }
            }

            return ArgType::kFlag;
        }

        for (size_t i = 0; i < arg_size; i++) {
            if (argv[idx][i] == '=') {
                return ArgType::kNamedArgWithVal;
            }
        }

        if (GetFlagPtr(parser, argv[idx], argv[idx][1] != '-')) {
            return ArgType::kFlag;
        }

        return ArgType::kNamedArgFlag;
    }
    ArgType GetArgType(const nargparse::ArgumentParser& parser, const char* arg, size_t arg_size) {
        if (!arg || !arg_size) { return ArgType::kNone; }

        ArgType conv[] = {ArgType::kNone, ArgType::kInt, ArgType::kFloat, ArgType::kString};

        if (arg_size < 2) {
            return conv[(int)GetValueType(arg, arg_size)];
        }

        const char* equals_sign = std::strchr(arg, '=');
        if (equals_sign) {
            return ArgType::kNamedArgWithVal;
        }

        if (GetFlagPtr(parser, arg, arg[1] != '-')) {
            return ArgType::kFlag;
        }

        if (GetNamedArgPtr(parser, arg, arg[1] != '-')) {
            return ArgType::kNamedArgFlag;
        }

        // std::cout << (int)GetValueType(arg, arg_size) << '\n';
        return conv[(int)GetValueType(arg, arg_size)];
    }
    
    int ToInt(const char* str) {
        int result = 0;
        for (size_t i = (str[0] == '-'); str[i] != '\0'; i++) {
            result *= 10;
            result -= str[i] - '0';
        }

        return str[0] == '-' ? result : -result;
    }
    float ToFloat(const char* str) {
        float result = 0;
        size_t i = str[0] == '-';
        while (str[i] != '\0' && str[i] != '.' && str[i] != 'f') {
            result *= 10;
            result += str[i] - '0';
            i++;
        }
        if (str[i++] == '\0') { return result; }

        uint32_t div = 10;
        while (str[i] != '\0' && str[i] != 'f') {
            result += (float)(str[i] - '0') / (float)div;
            div *= 10;
            i++;
        }

        return result;
    }
    void StrCpySlice(char* target, const char* src, size_t from, size_t to) {
        for (size_t i = from; i < to; i++) {
            target[i - from] = src[i];
        }
        target[to - from] = '\0';
    }

    bool IsWritable(const nargparse::ArgumentParser::PosArg& arg) {
        return arg.values_count == nargparse::kNargsOneOrMore || arg.values_count == nargparse::kNargsZeroOrMore || \
               !arg.values.size;
    }

    bool ParsePosArg(nargparse::ArgumentParser& parser, const char* raw_value, nargparse::ArgumentParser::ValueType arg_type) {
        using ArgsN = nargparse::ArgumentParser::ArgsN;
        using ValueType = nargparse::ArgumentParser::ValueType;

        using PosArg = nargparse::ArgumentParser::PosArg;
        PosArg* arg = nullptr;

        for (size_t i = 0; i < parser.pos_args.size; i++) {
            arg = (PosArg*)parser.pos_args.data[i];
            if (arg->value_type == arg_type && IsWritable(*arg)) {
                break;
            }
            arg = nullptr;
        }
        
        if (!arg) { return false; }

        if (arg->values.size && (arg->values_count == ArgsN::kRequired || arg->values_count == ArgsN::kOptional)) {
            return false;
        }

        bool was_empty = arg->values.size == 0;
        if (arg_type == ValueType::kInt) {
            int value = ToInt(raw_value);
            if (arg->validator.int_validator && !arg->validator.int_validator(value)) { return false; }

            nargparse::ContPushBack(arg->values, (void*)(new int(value)));

            if (was_empty && arg->connector) {
                *(int*)arg->connector = *(int*)arg->values.data[0];
            }
        } else if (arg_type == ValueType::kFloat) {
            float value = ToFloat(raw_value);
            if (arg->validator.float_validator && !arg->validator.float_validator(value)) { return false; }

            nargparse::ContPushBack(arg->values, (void*)(new float(value)));
            
            if (was_empty && arg->connector) {
                *(float*)arg->connector = *(float*)arg->values.data[0];
            }
        } else if (arg_type == ValueType::kString) {
            size_t arg_size = std::strlen(raw_value);
            char* value = nullptr;
            if (raw_value[0] == '"') {
                if (raw_value[arg_size - 1] != '"' || arg_size - 2 > parser.max_arg_len - 1) { return false; }

                value = new char[arg_size - 1];
                StrCpySlice(value, raw_value, 1, arg_size - 1);
            } else {
                if (arg_size > parser.max_arg_len - 1) { return false; }

                value = new char[arg_size + 1];
                std::strcpy(value, raw_value);
            }

            if (arg->validator.string_validator && !arg->validator.string_validator(value)) {
                delete[] value;
                return false;
            }
            
            nargparse::ContPushBack(arg->values, (void*)(char (*)[])(value));
            
            if (was_empty && arg->connector) {
                std::strcpy(*(char (*)[])arg->connector, *(char (*)[])arg->values.data[0]);
            }
        }

        return true;
    }
    bool ParseNamedArg(nargparse::ArgumentParser& parser, const char* flag, const char* raw_value, nargparse::ArgumentParser::ValueType arg_type) {
        using ArgsN = nargparse::ArgumentParser::ArgsN;
        using ValueType = nargparse::ArgumentParser::ValueType;

        nargparse::ArgumentParser::NamedArg* arg = GetNamedArgPtr(parser, flag, flag[1] != '-');

        if (!arg) { return false; }

        if (arg->values.size && (arg->values_count == ArgsN::kRequired || arg->values_count == ArgsN::kOptional)) {
            return false;
        }

        bool was_empty = arg->values.size == 0;
        if (arg_type == ValueType::kInt) {
            int value = ToInt(raw_value);
            if (arg->validator.int_validator && !arg->validator.int_validator(value)) { return false; }

            nargparse::ContPushBack(arg->values, (void*)(new int(value)));

            if (was_empty && arg->connector) {
                *(int*)arg->connector = *(int*)arg->values.data[0];
            }
        } else if (arg_type == ValueType::kFloat) {
            float value = ToFloat(raw_value);
            if (arg->validator.float_validator && !arg->validator.float_validator(value)) { return false; }

            nargparse::ContPushBack(arg->values, (void*)(new float(value)));

            if (was_empty && arg->connector) {
                *(float*)arg->connector = *(float*)arg->values.data[0];
            }
        } else if (arg_type == ValueType::kString) {
            size_t arg_size = std::strlen(raw_value);
            char* value = nullptr;
            if (raw_value[0] == '"') {
                if (raw_value[arg_size - 1] != '"' || arg_size - 2 > parser.max_arg_len - 1) { return false; }

                value = new char[arg_size - 2];
                StrCpySlice(value, raw_value, 1, arg_size - 1);
            } else {
                if (arg_size > parser.max_arg_len - 1) { return false; }

                value = new char[arg_size + 1];
                std::strcpy(value, raw_value);
            }
            
            if (arg->validator.string_validator && !arg->validator.string_validator(value)) {
                delete[] value;
                return false;
            }

            nargparse::ContPushBack(arg->values, (void*)(char (*)[])(value));
            
            if (was_empty && arg->connector) {
                std::strcpy(*(char (*)[])arg->connector, *(char (*)[])arg->values.data[0]);
            }
        }

        return true;
    }

    nargparse::ArgumentParser::PosArg* GetPosArgPtr(const nargparse::ArgumentParser& parser, const char* info) {
        using PosArg = nargparse::ArgumentParser::PosArg;
        PosArg* curr;
        for (size_t i = 0; i < parser.pos_args.size; i++) {
            curr = (PosArg*)parser.pos_args.data[i];
            if (!std::strcmp(curr->info, info)) {
                return curr;
            }
        }

        return nullptr;
    }
    nargparse::ArgumentParser::NamedArg* GetNamedArgPtr(const nargparse::ArgumentParser& parser, const char* info) {
        using NamedArg = nargparse::ArgumentParser::NamedArg;
        NamedArg* curr;
        for (size_t i = 0; i < parser.named_args.size; i++) {
            curr = (NamedArg*)parser.named_args.data[i];
            if (!std::strcmp(curr->info, info)) {
                return curr;
            }
        }

        return nullptr;
    }

    void FreeContainer(nargparse::Container& cont, nargparse::ArgumentParser::ValueType value_type=nargparse::ArgumentParser::ValueType::kNone) {
        using ValueType = nargparse::ArgumentParser::ValueType;

        if (value_type == ValueType::kInt) {
            for (size_t i = 0; i < cont.size; i++) {
                delete (int*)cont.data[i];
            }
        }  else if (value_type == ValueType::kFloat) {
            for (size_t i = 0; i < cont.size; i++) {
                delete (float*)cont.data[i];
            }
        } else if (value_type == ValueType::kString) {
            for (size_t i = 0; i < cont.size; i++) {
                delete[] (char*)cont.data[i];
            }
        }

        delete[] cont.data;
    }
}

void nargparse::ContRealloc(nargparse::Container& cont) {
    cont.capacity = cont.capacity ? cont.capacity * 2 : 1;
    void** tmp = new void*[cont.capacity];
    for (size_t i = 0; i < cont.size; i++) {
        tmp[i] = cont.data[i];
    }
    delete[] cont.data;
    cont.data = tmp;
}
void nargparse::ContPushBack(nargparse::Container& cont, void* value_ptr) {
    if (cont.size == cont.capacity || !cont.data) {
        nargparse::ContRealloc(cont);
    }
    cont.data[cont.size++] = value_ptr;
}

nargparse::ArgumentParser nargparse::CreateParser(const char* name, size_t max_arg_len) {
    size_t name_size = std::strlen(name);
    size_t help_size = std::strlen(" [args, flags]") + name_size;
    nargparse::ArgumentParser result =  {
        .name = new char[name_size + 1],
        .max_arg_len = max_arg_len,
        .help = new char[help_size + 1],

        .flags = Container{0, 0, nullptr},
        .pos_args = Container{0, 0, nullptr},
        .named_args = Container{0, 0, nullptr},
    };

    std::strcpy(result.name, name);
    result.name[name_size] = '\0';

    std::strcpy(result.help, name);
    std::strcat(result.help, " [args, flags]");
    result.help[help_size] = '\0';

    return result;
}

void nargparse::AddFlag(nargparse::ArgumentParser& parser, const char* short_flag, const char* long_flag, bool* value_ptr, const char* info, bool default_val) {
    if (GetFlagPtr(parser, short_flag, true)) {
        std::cout << "[WARNING]: flag using \"" << short_flag << "\" already exists\n";
    } if (GetFlagPtr(parser, long_flag, false)) {
        std::cout << "[WARNING]: flag using \"" << long_flag << "\" already exists\n";
    }

    nargparse::ArgumentParser::Flag* new_flag = new nargparse::ArgumentParser::Flag;
    *new_flag = {
        .value_ptr = value_ptr,
        .info = new char[std::strlen(info) + 1],
        .short_flag = nullptr,
        .long_flag = nullptr
    };
    std::strcpy(new_flag->info, info);
    
    if (short_flag) {
        new_flag->short_flag = new char[std::strlen(short_flag) + 1];
        std::strcpy(new_flag->short_flag, short_flag);
    }

    if (long_flag) {
        new_flag->long_flag = new char[std::strlen(long_flag) + 1];
        std::strcpy(new_flag->long_flag, long_flag);
    }

    if (value_ptr) {
        *value_ptr = default_val;
    }

    ContPushBack(parser.flags, (void*)new_flag);
}

void nargparse::AddArgument(nargparse::ArgumentParser& parser, int* value_ptr, const char* info, nargparse::ArgumentParser::ArgsN values_count, bool (*validator)(int const&), const char* criteria) {
    AddPosArg(parser,
              value_ptr,
              info,
              values_count,
              nargparse::ArgumentParser::Validator{.int_validator = validator},
              criteria,
              nargparse::ArgumentParser::ValueType::kInt);
}
void nargparse::AddArgument(nargparse::ArgumentParser& parser, const char* short_flag, const char* long_flag, int* value_ptr, const char* info, nargparse::ArgumentParser::ArgsN values_count, bool (*validator)(int const&), const char* criteria) {
    AddNamedArg(parser,
                short_flag,
                long_flag,
                value_ptr,
                info,
                values_count,
                nargparse::ArgumentParser::Validator{.int_validator = validator},
                criteria,
                nargparse::ArgumentParser::ValueType::kInt);
}

void nargparse::AddArgument(nargparse::ArgumentParser& parser, float* value_ptr, const char* info, nargparse::ArgumentParser::ArgsN values_count, bool (*validator)(float const&), const char* criteria) {
    AddPosArg(parser,
              value_ptr,
              info,
              values_count,
              nargparse::ArgumentParser::Validator{.float_validator = validator},
              criteria,
              nargparse::ArgumentParser::ValueType::kFloat);
}
void nargparse::AddArgument(nargparse::ArgumentParser& parser, const char* short_flag, const char* long_flag, float* value_ptr, const char* info, nargparse::ArgumentParser::ArgsN values_count, bool (*validator)(float const&), const char* criteria) {
    AddNamedArg(parser,
                short_flag,
                long_flag,
                value_ptr,
                info,
                values_count,
                nargparse::ArgumentParser::Validator{.float_validator = validator},
                criteria,
                nargparse::ArgumentParser::ValueType::kFloat);
}

void nargparse::AddArgument(nargparse::ArgumentParser& parser, char (*value_ptr)[], const char* info, nargparse::ArgumentParser::ArgsN values_count, bool (*validator)(const char* const&), const char* criteria) {
    AddPosArg(parser,
              value_ptr,
              info,
              values_count,
              nargparse::ArgumentParser::Validator{.string_validator = validator},
              criteria,
              nargparse::ArgumentParser::ValueType::kString);
}
void nargparse::AddArgument(nargparse::ArgumentParser& parser, const char* short_flag, const char* long_flag, char (*value_ptr)[], const char* info, nargparse::ArgumentParser::ArgsN values_count, bool (*validator)(const char* const&), const char* criteria) {
    AddNamedArg(parser,
                short_flag,
                long_flag,
                value_ptr,
                info,
                values_count,
                nargparse::ArgumentParser::Validator{.string_validator = validator},
                criteria,
                nargparse::ArgumentParser::ValueType::kString);
}

bool nargparse::Parse(nargparse::ArgumentParser& parser, size_t argc, const char* argv[]) {
    for (size_t i = 1; i < argc; i++) {
        if (!std::strcmp(argv[i], "-h") || !std::strcmp(argv[i], "--help")) {
            nargparse::PrintHelp(parser);
            return true;
        }

        size_t arg_size = std::strlen(argv[i]);
        ArgType arg_type = GetArgType(parser, argv[i], arg_size);

        // std::cout << argv[i] << ": ArgType(" << (int)arg_type << ")\n";

        if (arg_type == ArgType::kNone) { return false; }

        using ValueType = nargparse::ArgumentParser::ValueType;
        if (arg_type == ArgType::kFlag) {
            nargparse::ArgumentParser::Flag* target = GetFlagPtr(parser, argv[i], std::isalpha(argv[i][1]));
            if (!target) { return false; }
            
            if (target->value_ptr) {
                *target->value_ptr = true;
            }
        } else if (arg_type == ArgType::kInt) {
            if (!ParsePosArg(parser, argv[i], ValueType::kInt) &&
                !ParsePosArg(parser, argv[i], ValueType::kFloat)) {
                
                return false;
            }
        } else if (arg_type == ArgType::kFloat) {
            if (!ParsePosArg(parser, argv[i], ValueType::kFloat)) {
                return false;
            }
        } else if (arg_type == ArgType::kString) {
            if (!ParsePosArg(parser, argv[i], ValueType::kString)) {
                return false;
            }
        } else if (arg_type == ArgType::kNamedArgFlag) {
            i++;

            ValueType next_type = GetValueType(argv[i], std::strlen(argv[i]));

            // std::cout << "    " << argv[i] << ": ArgType(" << (int)next_type << ")\n";

            if (next_type == ValueType::kInt) {
                if (!ParseNamedArg(parser, argv[i - 1], argv[i], ValueType::kInt) &&
                    !ParseNamedArg(parser, argv[i - 1], argv[i], ValueType::kFloat)) {
                    
                    return false;
                }
            } else if (next_type == ValueType::kFloat) {
                if (!ParseNamedArg(parser, argv[i - 1], argv[i], ValueType::kFloat)) {
                    return false;
                }
            } else if (next_type == ValueType::kString) {
                if (!ParseNamedArg(parser, argv[i - 1], argv[i], ValueType::kString)) {
                    return false;
                }
            } else { return false; }
        } else if (arg_type == ArgType::kNamedArgWithVal) {
            size_t flag_size = 0;
            while (argv[i][flag_size] != '=') { flag_size++; }

            size_t val_size = arg_size - flag_size - 1;
            if (!val_size) { return false; }

            char* flag = new char[flag_size + 1];
            StrCpySlice(flag, argv[i], 0, flag_size);
            flag[flag_size] = '\0';

            char* val = new char[val_size + 1];
            StrCpySlice(val, argv[i], flag_size + 1, arg_size);
            val[val_size] = '\0';

            ValueType val_type = GetValueType(val, val_size);

            if (val_type == ValueType::kString) {
                // значит была строка
                if (!ParseNamedArg(parser, flag, val, ValueType::kString)) {
                    delete[] flag;
                    delete[] val;
                    return false;
                }
            } else if (val_type == ValueType::kInt) {
                if (!ParseNamedArg(parser, flag, val, ValueType::kInt) &&
                    !ParseNamedArg(parser, flag, val, ValueType::kFloat)) {
                    delete[] flag;
                    delete[] val;
                    return false;
                }
            } else if (val_type == ValueType::kFloat) {
                if (!ParseNamedArg(parser, flag, val, ValueType::kFloat)) {
                    delete[] flag;
                    delete[] val;
                    return false;
                }
            }
            else {
                delete[] flag;
                delete[] val;
                return false;
            }

            delete[] flag;
            delete[] val;
        } else { return false; }
    }

    using ArgsN = nargparse::ArgumentParser::ArgsN;
    void* arg;

    using PosArg = nargparse::ArgumentParser::PosArg;
    for (size_t i = 0; i < parser.pos_args.size; i++) {
        arg = parser.pos_args.data[i];

        if (!((PosArg*)arg)->values.size) {
            if (((PosArg*)arg)->values_count == ArgsN::kOneOrMore || ((PosArg*)arg)->values_count == ArgsN::kRequired) {
                return false;
            }
        }
    }

    using NamedArg = nargparse::ArgumentParser::NamedArg;
    for (size_t i = 0; i < parser.named_args.size; i++) {
        arg = parser.named_args.data[i];

        if (!((NamedArg*)arg)->values.size) {
            if (((NamedArg*)arg)->values_count == ArgsN::kOneOrMore || ((NamedArg*)arg)->values_count == ArgsN::kRequired) {
                return false;
            }
            // Для опциональных аргументов не выделяем память, если значение не было предоставлено
        }
    }

    return true;
}

size_t nargparse::GetRepeatedCount(const nargparse::ArgumentParser& parser, const char* info) {
    using PosArg = nargparse::ArgumentParser::PosArg;
    void* arg = GetPosArgPtr(parser, info);
    if (arg) {
        return ((PosArg*)arg)->values.size;
    }

    using NamedArg = nargparse::ArgumentParser::NamedArg;
    arg = GetNamedArgPtr(parser, info);
    if (arg) {
        return ((NamedArg*)arg)->values.size;
    }

    std::cout << "[WARNING]: No argument \"" << info << "\" exists\n";

    return 0;
}
bool nargparse::GetRepeated(nargparse::ArgumentParser& parser, const char* info, size_t idx, void* target) {
    using ValueType = nargparse::ArgumentParser::ValueType;
    ValueType value_type;

    using PosArg = nargparse::ArgumentParser::PosArg;
    void* arg = GetPosArgPtr(parser, info);

    if (arg) {
        value_type = ((PosArg*)arg)->value_type;

        if (!target) { return false; }

        if (idx >= ((PosArg*)arg)->values.size) {
            std::cout << "[ERROR]: Index out of range\n";
            return false;
        }

        if (value_type == ValueType::kInt) {
            *(int*)target = *(int*)((PosArg*)arg)->values.data[idx];
        } else if (value_type == ValueType::kFloat) {
            *(float*)target = *(float*)((PosArg*)arg)->values.data[idx];
        } else if (value_type == ValueType::kString) {
            // std::strcpy(*(char (*)[])target, *(char (*)[])((PosArg*)arg)->values.data[idx]);
            *(const char**)target = *(char (*)[])((PosArg*)arg)->values.data[idx];
        } else {
            std::cout << "[ERROR]: Unknown arg type\n";
        }

        return true;
    }

    using NamedArg = nargparse::ArgumentParser::NamedArg;
    arg = GetNamedArgPtr(parser, info);
    if (arg) {
        value_type = ((NamedArg*)arg)->value_type;

        if (!target) { return false; }

        if (idx >= ((NamedArg*)arg)->values.size) {
            std::cout << "[ERROR]: Index out of range\n";
            return false;
        }

        if (value_type == ValueType::kInt) {
            *(int*)target = *(int*)((NamedArg*)arg)->values.data[idx];
        } else if (value_type == ValueType::kFloat) {
            *(float*)target = *(float*)((NamedArg*)arg)->values.data[idx];
        } else if (value_type == ValueType::kString) {
            // std::strcpy(*(char (*)[])target, *(char (*)[])((NamedArg*)arg)->values.data[idx]);
            *(const char**)target = *(char (*)[])((NamedArg*)arg)->values.data[idx];
        } else {
            std::cout << "[ERROR]: Unknown arg type\n";
        }

        return true;
    }

    std::cout << "[ERROR]: No argument \"" << info << "\" exists";

    return false;
}

void nargparse::AddHelp(nargparse::ArgumentParser& parser) {
    size_t help_size = strlen(parser.help);
    void* tmp;

    const char* types[] = {"none", "int", "float", "char[]"};

    nargparse::AddFlag(parser, "-h", "--help", nullptr, "Help");

    using Flag = nargparse::ArgumentParser::Flag;
    for (size_t i = 0; i < parser.flags.size; i++) {
        tmp = parser.flags.data[i];
        help_size += 5 + std::strlen(((Flag*)tmp)->short_flag) + 2 + std::strlen(((Flag*)tmp)->long_flag) + 5 + \
                     std::strlen("FLAG") + 1 + std::strlen(((Flag*)tmp)->info);
    }
    if (parser.flags.size && (parser.pos_args.size || parser.named_args.size)) {
        help_size++;
    }

    using PosArg = nargparse::ArgumentParser::PosArg;
    for (size_t i = 0; i < parser.pos_args.size; i++) {
        tmp = parser.pos_args.data[i];
        help_size += 5 + std::strlen("[POSITIONAL]") + 5 + \
                     std::strlen(types[(int)((PosArg*)tmp)->value_type]) + 1 + std::strlen(((PosArg*)tmp)->info);
        if (((PosArg*)tmp)->criteria) {
            help_size += 3 + std::strlen(((PosArg*)tmp)->criteria);
        }
    }
    if (parser.pos_args.size && parser.named_args.size) {
        help_size++;
    }

    using NamedArg = nargparse::ArgumentParser::NamedArg;
    for (size_t i = 0; i < parser.named_args.size; i++) {
        tmp = parser.named_args.data[i];
        help_size += 5 + std::strlen(((NamedArg*)tmp)->short_flag) + 2 + std::strlen(((NamedArg*)tmp)->long_flag) + 5 + \
                     std::strlen(types[(int)((NamedArg*)tmp)->value_type]) + 1 + std::strlen(((NamedArg*)tmp)->info);
        if (((NamedArg*)tmp)->criteria) {
            help_size += 3 + std::strlen(((NamedArg*)tmp)->criteria);
        }
    }
    help_size++;

    char* new_help = new char[help_size + 1];
    std::strcpy(new_help, parser.help);

    delete[] parser.help;
    parser.help = new_help;

    for (size_t i = 0; i < parser.flags.size; i++) {
        tmp = parser.flags.data[i];

        std::strcat(parser.help, "\n    ");
        std::strcat(parser.help, ((Flag*)tmp)->short_flag);
        std::strcat(parser.help, ", ");
        std::strcat(parser.help, ((Flag*)tmp)->long_flag);
        std::strcat(parser.help, " - [FLAG] ");
        std::strcat(parser.help, ((Flag*)tmp)->info);
    }
    if (parser.flags.size && (parser.pos_args.size || parser.named_args.size)) {
        std::strcat(parser.help, "\n");
    }

    for (size_t i = 0; i < parser.pos_args.size; i++) {
        tmp = parser.pos_args.data[i];

        std::strcat(parser.help, "\n    [POSITIONAL] - [");
        std::strcat(parser.help, types[(int)((PosArg*)tmp)->value_type]);
        std::strcat(parser.help, "] ");
        std::strcat(parser.help, ((PosArg*)tmp)->info);
        
        if (((PosArg*)tmp)->criteria) {
            std::strcat(parser.help, " (");
            std::strcat(parser.help, ((PosArg*)tmp)->criteria);
            std::strcat(parser.help, ")");
        }
    }
    if (parser.pos_args.size && parser.named_args.size) {
        std::strcat(parser.help, "\n");
    }

    for (size_t i = 0; i < parser.named_args.size; i++) {
        tmp = parser.named_args.data[i];

        std::strcat(parser.help, "\n    ");
        std::strcat(parser.help, ((NamedArg*)tmp)->short_flag);
        std::strcat(parser.help, ", ");
        std::strcat(parser.help, ((NamedArg*)tmp)->long_flag);
        std::strcat(parser.help, " - [");
        std::strcat(parser.help, types[(int)((NamedArg*)tmp)->value_type]);
        std::strcat(parser.help, "] ");
        std::strcat(parser.help, ((NamedArg*)tmp)->info);
        
        if (((NamedArg*)tmp)->criteria) {
            std::strcat(parser.help, " (");
            std::strcat(parser.help, ((NamedArg*)tmp)->criteria);
            std::strcat(parser.help, ")");
        }
    }
    std::strcat(parser.help, "\n");
}
void nargparse::PrintHelp(nargparse::ArgumentParser& parser) {
    std::cout << "--------------\nPARSER USAGE:\n" << parser.help << '\n';
}

void nargparse::FreeParser(nargparse::ArgumentParser& parser) {
    delete[] parser.help;
    delete[] parser.name;

    // чистим флаги
    for (size_t i = 0; i < parser.flags.size; i++) {
        using Flag = nargparse::ArgumentParser::Flag;
        Flag* flag = (Flag*)parser.flags.data[i];

        delete[] flag->info;
        delete[] flag->short_flag;
        delete[] flag->long_flag;

        delete flag;
    }
    FreeContainer(parser.flags);

    // чистим позционные
    for (size_t i = 0; i < parser.pos_args.size; i++) {
        using PosArg = nargparse::ArgumentParser::PosArg;
        PosArg* arg = (PosArg*)parser.pos_args.data[i];

        delete[] arg->info;
        delete[] arg->criteria;
        FreeContainer(arg->values, arg->value_type);

        delete arg;
    }
    FreeContainer(parser.pos_args);

    // чистим именованные
    for (size_t i = 0; i < parser.named_args.size; i++) {
        using NamedArg = nargparse::ArgumentParser::NamedArg;
        NamedArg* arg = (NamedArg*)parser.named_args.data[i];

        delete[] arg->info;
        delete[] arg->short_flag;
        delete[] arg->long_flag;
        delete[] arg->criteria;
        FreeContainer(arg->values, arg->value_type);

        delete arg;
    }
    FreeContainer(parser.named_args);
}