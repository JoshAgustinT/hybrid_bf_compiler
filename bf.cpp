/*
Joshua Tlatelpa-Agustin
9/18/24
bf language compiler
written for adv compilers course
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stack>
#include <map>
#include <unordered_set>
#include <sstream>
#include <cassert>
#include <regex>
#include <Python.h>

using namespace std;

vector<char> program_file;
ofstream *output_file;
int loop_num = -1;
stack<int> myStack;
const int tape_size = 1048576;

bool simple_loop_flag = false;
bool seek_flag = false;
bool optimization_flag = false;
string bf_file_name = "";

/// used for partial eval
char tape[tape_size];
int tape_position = tape_size / 2;
int min_offset = 0;
int max_offset = 0;

/*
jasm, short for josh asm :] outputs to output_file.
*/
void jasm(string text)
{
    *output_file << text << endl;
}

void print_padding()
{
    *output_file << endl;
}

/*
naive implementation, also easier to debug
*/
void bf_assembler(char token)
{

    switch (token)
    {
    case '>':
        // Load base address into %rax
        jasm("movq    -8(%rbp), %rax");
        // add one to pointer address
        jasm("addq    $1, %rax");
        // Store the adjusted pointer back at -8(%rbp)
        jasm("movq    %rax, -8(%rbp)");
        // tape_position++;
        break;
    case '<':

        // Load base address into %rax
        jasm("movq    -8(%rbp), %rax");
        // remove one from pointer address
        jasm("subq    $1, %rax");
        // Store the adjusted pointer back at -8(%rbp)
        jasm("movq    %rax, -8(%rbp)");
        // tape_position--;
        break;
    case '+':

        // Load base address into %rax
        jasm("movq    -8(%rbp), %rax");
        // Load byte into %cl (lower 8 bits)
        jasm("movb    (%rax), %cl");

        // Add 1 to the byte
        jasm("addb    $1, %cl");

        // Store the modified byte back to the address in %rax
        jasm("movb    %cl, (%rax)");

        // tape[tape_position] += 1;
        break;
    case '-':

        // Load the pointer from -8(%rbp) into %rax
        jasm("movq    -8(%rbp), %rax");

        // Load byte into %cl (lower 8 bits)
        jasm("movb    (%rax), %cl");

        // Decrrement byte in %cl
        jasm("subb    $1, %cl");

        // Store the modified byte back to the address in %rax
        jasm("movb    %cl, (%rax)");

        // tape[tape_position] -= 1;
        break;
    case '.':

        // Load the pointer from -8(%rbp) into %rax
        jasm("movq    -8(%rbp), %rax");

        // Load the byte from the address into %al (to use with putc)
        jasm("movb    (%rax), %al");

        // Prepare for putc
        // Load file descriptor for stdout into %rsi
        jasm("movq    stdout(%rip), %rsi");
        // Move and sign-extend byte in %al to %edi
        jasm("movsbl  %al, %edi");

        // Call putc to print the character
        jasm("call    putc@PLT");

        // cout << tape[tape_position];
        break;
    case ',':

        // Move the file pointer for stdin into the %rdi register
        jasm("movq    stdin(%rip), %rdi");

        // Call the getc function to read a character from stdin (returned in %al)
        jasm("call    getc@PLT");
        // Move the byte from %al into %bl
        jasm("movb    %al, %bl");
        // Load the pointer from -8(%rbp) into %rax
        jasm("movq    -8(%rbp), %rax");

        // Store the byte from %bl into the memory pointed to by %rax
        jasm("movb    %bl, (%rax)");

        // char nextByte;
        // cin.get(nextByte);
        // tape[tape_position] = nextByte;
        break;
    case '[':
    {
        loop_num++;
        myStack.push(loop_num);

        string start_label = "start_loop_";
        start_label += to_string(loop_num);
        string end_label = "end_loop_";
        end_label += to_string(loop_num);

        jasm(start_label + ":");
        // Load the pointer from -8(%rbp) into %rax
        jasm("movq    -8(%rbp), %rax");

        // Load byte into %cl (lower 8 bits)
        jasm("movb    (%rax), %cl");
        // jump to matching end label if 0
        jasm("cmpb    $0, %cl");
        jasm("je      " + end_label);
    }

    break;
    case ']':
    {
        int match_loop = myStack.top();
        myStack.pop();
        string start_label = "start_loop_";
        start_label += to_string(match_loop);

        string end_label = "end_loop_";
        end_label += to_string(match_loop);

        jasm(end_label + ":");
        // Load the pointer from -8(%rbp) into %rax
        jasm("movq    -8(%rbp), %rax");

        // Load byte into %cl (lower 8 bits)
        jasm("movb    (%rax), %cl");

        // jump to matching start label if not 0
        jasm("cmpb    $0, %cl");
        jasm("jne      " + start_label);
    }

    break;
    default:
        // non bf instruction, so we ignore
        break;
    } // end switch
}

bool startsWith(const std::string &str, const std::string &prefix)
{
    return str.compare(0, prefix.length(), prefix) == 0;
}

void asm_setup()
{
    // Assembly setup
    jasm(".file	\"bf compiler\"");

    jasm(".section .data");

    if (optimization_flag || seek_flag)
    {
        // offset masks= 1,2,4,8,16
        jasm(".p2align 5");
        jasm(".four_offset_mask:");
        jasm(".quad   282578783371521");
        jasm(".quad   282578783371521");
        jasm(".quad   282578783371521");
        jasm(".quad  282578783371521");

        jasm(".p2align 5");
        jasm(".one_offset_mask:");
        jasm(".quad   0");
        jasm(".quad   0");
        jasm(".quad   0");
        jasm(".quad   0");

        jasm(".p2align 5");
        jasm(".two_offset_mask:");
        jasm(".quad   281479271743489");
        jasm(".quad   281479271743489");
        jasm(".quad   281479271743489");
        jasm(".quad  281479271743489");

        jasm(".p2align 5");
        jasm(".eight_offset_mask:");
        jasm(".quad   282578800148737");
        jasm(".quad   282578800148737");
        jasm(".quad   282578800148737");
        jasm(".quad   282578800148737");

        jasm(".p2align 5");
        jasm(".sixteen_offset_mask:");
        jasm(".quad   72340172838076673");
        jasm(".quad   282578800148737");
        jasm(".quad   72340172838076673");
        jasm(".quad  282578800148737");

        jasm(".p2align 5");
        jasm(".thirty_two_offset_mask:");
        jasm(".quad   72340172838076673");
        jasm(".quad   72340172838076673");
        jasm(".quad   72340172838076673");
        jasm(".quad  282578800148737");
    }

    jasm(".text");
    jasm(".section	.text");
    jasm(".globl	main");
    jasm(".type	main, @function");
    print_padding();

    jasm("main:");

    jasm("pushq	%rbp");
    jasm("movq	%rsp, %rbp");
    // Allocate 16 bytes of stack space for local variables
    jasm("subq	$16, %rsp");
    // Allocate 100,000 bytes with malloc
    jasm("movl	$" + to_string(tape_size) + ", %edi");

    jasm("call	malloc@PLT");
    // Store the pointer returned by malloc in the local variable at -8(%rbp)
    jasm("movq	%rax, -8(%rbp)");

    // Calculate the address 50,000 bytes into the allocated memory
    jasm("addq    $" + to_string(tape_size / 2) + ", %rax"); // Add the offset to %rax

    // Store the adjusted pointer back at -8(%rbp)
    jasm("movq    %rax, -8(%rbp)");
    jasm("movq    -8(%rbp), %r13"); // keep our copy of the cell address at r13
}

void asm_cleanup()
{
    // Set the return value to 0 (successful completion)
    jasm("movl    $0, %eax");
    // Proper stack cleanup
    jasm("movq    %rbp, %rsp");
    // Restore the old base pointer
    jasm("popq    %rbp");
    // Return from the function
    jasm("ret");
    print_padding();
}
/*
Turn  vector<char> bf program to vector<string>, to support saving complex instructions
also this list will not contain non-instruction
*/
vector<string> init_optimized_program_list(vector<char> list)
{
    vector<string> return_list;

    for (int i = 0; i < list.size(); i++)
    {
        char token = list[i];
        switch (token)
        {
        case '>':
            return_list.push_back(string(1, token));
            break;
        case '<':
            return_list.push_back(string(1, token));
            break;
        case '+':
            return_list.push_back(string(1, token));
            break;
        case '-':
            return_list.push_back(string(1, token));
            break;
        case '.':
            return_list.push_back(string(1, token));
            break;
        case ',':
            return_list.push_back(string(1, token));
            break;
        case '[':
            return_list.push_back(string(1, token));
            break;
        case ']':
            return_list.push_back(string(1, token));
            break;
        default:
            // non bf instruction, so we ignore
            break;
        } // end switch

    } // end for loop

    return return_list;
}

/*
returns an unordered_set<int> of all starting loop indices.
*/
unordered_set<int> get_loop_indices(vector<string> list)
{
    unordered_set<int> return_set;

    for (int i = 0; i < list.size(); i++)
    {
        string token = list[i];

        if (token == "[")
            return_set.insert(i);
    }
    return return_set;
}

/*
Returns associated loop of given index from passed in list.
ie, [*.>><], given program index of [, will return [*.>><]
*/
vector<string> get_loop_string(int j, vector<string> list)
{
    vector<string> return_list;
    if (list[j] == "[")
    {
        return_list.push_back("[");
        int count = 1;
        while (count != 0)
        {
            j++;

            if (list[j] == "[")
                count++;

            if (list[j] == "]")
                count--;

            // Code for handling valid instr
            return_list.push_back(list[j]);
        }

        return return_list;
    }

    cout << "couldn't find '[' in get_loop_string(), wrong index?? " << endl;
    return return_list;
}

// tells us net change of our initial cell
int get_current_cell_change(vector<string> loop_string)
{
    int cell = 0;
    int cell_value = 0;
    // iterate over body of loop ie -> [...]
    for (int i = 1; i < loop_string.size() - 1; i++)
    {
        string token = loop_string[i];

        if (token == "<")
            cell--;
        if (token == ">")
            cell++;

        if (token == "+")
        {
            if (cell == 0)
            {
                cell_value++;
            }
        }
        if (token == "-")
        {
            if (cell == 0)
            {
                cell_value--;
            }
        }

    } // end for loop

    return cell_value;
}

/*
Checks if input string is a simple loop
*/
bool is_simple_loop(vector<string> loop_string)
{
    bool answer = true;
    int cell_offset = 0;
    int loop_cell_value = 0;
    // iterate over body of loop ie -> [...]
    for (int i = 1; i < loop_string.size() - 1; i++)
    {
        string token = loop_string[i];

        if (token == ",")
        {
            answer = false;
            break;
        }
        if (token == ".")
        {
            answer = false;
            break;
        }
        if (token == "[")
        {
            answer = false;
            break;
        }
        if (token == "]")
        {
            answer = false;
            break;
        }
        if (token == "<")
            cell_offset--;
        if (token == ">")
            cell_offset++;

        if (token == "+")
        {
            if (cell_offset == 0)
            {
                loop_cell_value++;
            }
        }
        if (token == "-")
        {
            if (cell_offset == 0)
            {
                loop_cell_value--;
            }
        }
        if (token.compare(0, 12, "expr_simple:") == 0)
        {
            answer = false;
            break;
        }
        if (token.compare(0, 10, "expr_seek:") == 0)
        {
            answer = false;
            break;
        }

    } // end for loop

    if (cell_offset != 0)
        answer = false;

    if (abs(loop_cell_value) != 1)
        answer = false;

    return answer;
}

/*
power of two or its negative counterpart
*/
bool is_power_of_two(int n)
{
    bool answer = false;
    switch (n)
    {
    case -32:
    case -16:
    case -8:
    case -4:
    case -2:
    case 2:
    case 4:
    case 16:
    case 32:
        answer = true;
        break;
    default:
        break;
    }

    return answer;
}
/*
Checks if input loop is a power of 2 seek loop
*/
int is_power_two_seek_loop(vector<string> loop_string)
{
    int offset = 0;
    // iterate over body of loop ie -> [...]
    for (int i = 1; i < loop_string.size() - 1; i++)
    {
        string token = loop_string[i];

        if (token != ">" && token != "<")
        {
            return false;
        }
        if (token == ">")
            offset++;

        if (token == "<")
            offset--;

    } // end for loop

    if (!is_power_of_two(offset))
        return false;
    // basically only true if we only have one type of seek
    return offset;
}

/*
Checks if input loop is a seek loop
*/
int is_seek_loop(vector<string> loop_string)
{
    int offset = 0;
    // iterate over body of loop ie -> [...]
    for (int i = 1; i < loop_string.size() - 1; i++)
    {
        string token = loop_string[i];

        if (token != ">" && token != "<")
        {
            return false;
        }
        if (token == ">")
            offset++;

        if (token == "<")
            offset--;

    } // end for loop

    // basically only true if we only have one type of seek
    return offset;
}

/*
Prints all elements in a vector<string> with no new line in between elements, with a new line at end.
*/
void print_string_vector(vector<string> list)
{
    for (auto token : list)
        cout << token;

    cout << endl;
}

/*
prints string vector vecrtically and ignores white space
*/
void vprint_string_vector(vector<string> list)
{
    for (auto token : list)
    {
        if (token == " ")
            continue;
        cout << token << endl;
    }

    cout << endl;
}

string expr_dict_to_string(map<int, int> dict, int loop_increment)
{

    string sb = "expr_simple:";

    // will tell us if we have +1 or -1 simple loops.
    if (loop_increment >= 0)
        sb += "+";

    else
        sb += "-";

    for (const auto &pair : dict)
    {
        sb += to_string(pair.first);
        sb += ":";
        sb += to_string(pair.second);
        sb += ",";
    }

    return sb;
}

void print_int_int_map(map<int, int> dict)
{

    for (const auto &pair : dict)
    {
        cout << "{";
        cout << to_string(pair.first);
        cout << ":";
        cout << to_string(pair.second);
        cout << "}";
        cout << endl;
    }
}
vector<string> optimize_simple_loop(int loop_index, int loop_increment, vector<string> loop, vector<string> program)
{

    map<int, int> dict;
    int tape_offset = 0;

    for (auto token : loop)
    {
        if (token == ">")
            tape_offset++;
        if (token == "<")
            tape_offset--;
        if (token == "+")
        {
            if (dict.find(tape_offset) != dict.end())
                dict[tape_offset] = dict[tape_offset] + 1;
            else
                dict[tape_offset] = 0 + 1;
        }
        if (token == "-")
        {
            if (dict.find(tape_offset) != dict.end())
                dict[tape_offset] = dict[tape_offset] - 1;
            else
                dict[tape_offset] = 0 - 1;
        }
    } // end loop

    for (int i = 0; i < loop.size(); i++)
    {
        program[loop_index + i] = " ";
    }
    // remove loop if [], should never fire unless infinite loop
    // if (loop.size() == 2)
    //     return program;

    program[loop_index] = expr_dict_to_string(dict, loop_increment);

    return program;
}

map<int, int> expr_string_to_dict(string expr)
{
    map<int, int> dict;

    // Check if the string starts with "expr_simple:"
    if (expr.compare(0, 12, "expr_simple:") == 0)
    {
        // Remove the prefix, plus the sign attached
        expr.erase(0, 13);
        // cout << expr );
        //  example- 0:-1,1:7,2:10,3:3,4:1,
        std::string first, second;

        std::istringstream ss(expr); // Use stringstream for easy tokenization

        while (std::getline(ss, first, ':'))
        {
            // Extract string before ':'
            std::getline(ss, second, ','); // Extract string after ':' but before ','

            int int_first = stoi(first);
            int int_second = stoi(second);
            if (dict.find(int_first) != dict.end())
            {
                cout << "why was there a duplicate while parsing expr into dict?" << endl;
                cout << int_first << "," << int_second << endl;
                assert(1 == 0);
            }
            else
                dict[int_first] = int_second;
        }

    } // end expr_simple
    else
    {
        cout << "expr_string_to_dict(), you didn't pass in an expr string?" << endl;
        assert(1 == 0);
    }

    return dict;
}

int get_expr_seek_offset(string expr)
{
    int offset = 0;
    if (expr.compare(0, 10, "expr_seek:") == 0)
    {
        // Remove the prefix, plus the sign attached
        expr.erase(0, 10);
        offset = stoi(expr);
        // cout<< offset<<endl;
    }
    else
    {
        cout << "erm u didnt pass in a expr_seek string... in get_expr_seek_offset()" << endl;
        assert(1 == 0);
    }

    return offset;
}

vector<string> get_closed_form_expr_list(string expr)
{
    vector<string> list;

    if (expr.compare(0, 12, "closed_form:") == 0)
    {
        // Remove the prefix, plus the sign attached
        expr.erase(0, 12);

        std::string first;

        std::istringstream ss(expr); // Use stringstream for easy tokenization

        while (std::getline(ss, first, ';'))
        {

            // cout << first  << endl;
            list.push_back(first);

        } // end while
    }
    else
    {
        cout << "erm u didnt pass in a closed_form_expr... string... in get_closed_form_expr()" << endl;
        assert(1 == 0);
    }

    return list;
}

/*
get X from pX or p_X , where X is some number/numbers
return X, or -X if _X
*/
string p_get_offset(string pX)
{

    // Determine the starting index for extracting the number
    int start_index = (pX[1] == '_') ? 2 : 1;

    // Extract the numeric part from the string
    std::string number_str = pX.substr(start_index);
    int number;

    // Convert the string to an integer
    std::stringstream ss(number_str);
    ss >> number;
    number *= 1;

    // Return the number, negating it if the second character is '_'
    return (pX[1] == '_') ? to_string(-number) : to_string(number);
}

std::vector<std::string> separate_terms(const std::string &input)
{
    std::regex regex("([+-]?[^+-]+)");
    std::vector<std::string> terms(
        std::sregex_token_iterator(input.begin(), input.end(), regex),
        std::sregex_token_iterator());
    return terms;
}

bool is_number(const std::string &str)
{

    size_t start = 0;

    // Check for optional leading sign
    if (str[0] == '+' || str[0] == '-')
    {
        start = 1; // Start checking from the next character
    }

    // Check if all remaining characters are digits
    for (size_t i = start; i < str.length(); ++i)
    {
        if (!std::isdigit(str[i]) && str[i] != ' ')
        {
            return false; // Found a non-digit character
        }
    }

    return true; // All characters are valid digits
}

std::string remove_whitespace(const std::string &str)
{
    std::string result; // Create a new string to store the result

    // Iterate through each character in the original string
    for (char ch : str)
    {
        // If the character is not a whitespace, add it to the result
        if (!std::isspace(static_cast<unsigned char>(ch)))
        {
            result += ch; // Concatenate non-whitespace characters
        }
    }

    return result; // Return the modified string
}

/*
our optimized assembler, usees vector strings to store instructions which allow for easier modification of bf source
we also further optimize by keeping out tape location at register r13
*/
void bf_string_assembler(string token)
{
    // print_padding();

    if (token == ">")
        // add one to pointer address
        jasm("addq    $1, %r13");
    if (token == "<")
        // remove one from pointer address
        jasm("addq    $-1, %r13");
    if (token == "+")
        // Add 1 to the byte
        jasm("addb    $1, (%r13)");

    if (token == "-")
        jasm("subb    $1, (%r13)");

    if (token == ".")
    {

        // Load the byte from the address into %al (to use with putc)
        // jasm("movb    (%r13), %al");

        // Prepare for putc
        // Load file descriptor for stdout into %rsi
        jasm("movq    stdout(%rip), %rsi");
        // Move and sign-extend byte in %al to %edi
        jasm("movsbl  (%r13), %edi");

        // Call putc to print the character
        jasm("call    putc@PLT");

        // cout << tape[tape_position];
    }
    if (token == ",")
    {

        // Move the file pointer for stdin into the %rdi register
        jasm("movq    stdin(%rip), %rdi");

        // Call the getc function to read a character from stdin (returned in %al)
        jasm("call    getc@PLT");
        // Move the byte from %al into %bl

        // Store the byte from %bl into r13 our current cell
        jasm("movb    %al, (%r13)");
    }
    if (token == "[")
    {

        loop_num++;
        myStack.push(loop_num);

        string start_label = "start_loop_" + to_string(loop_num);
        string end_label = "end_loop_" + to_string(loop_num);

        // jump to matching end label if 0
        jasm("cmpb    $0, (%r13)");
        jasm("je      " + end_label);
        jasm(start_label + ":");
    }
    if (token == "]")
    {
        int match_loop = myStack.top();
        myStack.pop();
        string start_label = "start_loop_" + to_string(match_loop);
        string end_label = "end_loop_" + to_string(match_loop);

        // jump to matching start label if not 0
        jasm("cmpb    $0, (%r13)");
        jasm("jne      " + start_label);
        jasm(end_label + ":");
    }

    if (startsWith(token, "expr_simple:"))
    {
        string sign_of_loop;

        // if +, we perform loop 256-255 times
        // if -, we perform full p[0] times
        if (startsWith(token, "expr_simple:+"))
            sign_of_loop = "+";

        if (startsWith(token, "expr_simple:-"))
            sign_of_loop = "-";

        map<int, int> simple_expr = expr_string_to_dict(token);

        // 8 bit regists, AH AL BH BL CH CL DH DL
        // ch and bl work

        // save current cell contents at start of loop in rcx

        if (sign_of_loop == "-")
            jasm("movq    (%r13), %rcx");

        if (sign_of_loop == "+")
        {
            // input can't be more than 255 so we're good
            jasm("movq    $256, %rcx ");
            jasm("subq    (%r13), %rcx  ");
        }

        for (const auto &pair : simple_expr)
        {
            if (pair.first == 0)
                continue;

            // pair.first = pointer offset
            // pair.second = cell +- change per loop

            // copy address of the first loop cell
            jasm("movq   %r13, %r12");
            // adjust pointer by our offset
            jasm("addq   $" + to_string(pair.first) + ", %r12");
            // set our constant change of cell
            jasm("movq    $" + to_string((pair.second)) + ", %r15");
            // constant multiplied by times loop happens
            jasm("imul   %rcx, %r15");

            jasm("addb    %r15b , (%r12)");
        }
        // our loop should always end in 0, this assures it, but would break
        // intentional infinite loops ¯\_(ツ)_/¯, saves us like 5 instr per loop
        jasm("movb    $0, (%r13)");

        print_padding();
    }

    if (startsWith(token, "expr_seek:"))
    {
        print_padding();
        loop_num++;

        int seek_offset = get_expr_seek_offset(token);

        if (is_power_of_two(seek_offset))
        {
            // cout<< seek_offset<<endl;

            string start_label = "start_seek_loop_" + to_string(loop_num);
            string end_label = "end_seek_loop_" + to_string(loop_num);

            // jasm("movb    (%r13), %cl");
            jasm("cmpb    $0, (%r13)");

            jasm("je      " + end_label);

            // this makes our offset masks easier to reason about (i like seeing 32)
            // subtracting 32, so our loop can always add 32, and on first iterations will be 0.

            if (seek_offset > 0)
            {
                jasm("addq $1, %r13");
                jasm("subq    $32, %r13");
            }
            else
            {
                // same alignment reason, but we don't need to offset our first loop since we want to read the previous bytes
                jasm("addq $1, %r13");
                // jasm("addq    $32, %r13");
            }

            if (abs(seek_offset) == 4)
                jasm("vmovdqa .four_offset_mask(%rip), %ymm0");
            if (abs(seek_offset) == 1)
                jasm("vmovdqa .one_offset_mask(%rip), %ymm0");
            if (abs(seek_offset) == 2)
                jasm("vmovdqa .two_offset_mask(%rip), %ymm0");
            if (abs(seek_offset) == 8)
                jasm("vmovdqa .eight_offset_mask(%rip), %ymm0");
            if (abs(seek_offset) == 16)
                jasm("vmovdqa .sixteen_offset_mask(%rip), %ymm0");
            if (abs(seek_offset) == 32)
                jasm("vmovdqa .thirty_two_offset_mask(%rip), %ymm0");

            // Loop for checking bytes in chunks of 32
            ////////////////////////////////////////////////////////////
            jasm(start_label + ":");

            if (seek_offset > 0)
                jasm("addq    $32, %r13");
            else
                jasm("subq    $32, %r13"); // CHANGE IF neg i think

            jasm("vpor    (%r13), %ymm0, %ymm2");
            jasm("vpxor   %xmm1, %xmm1, %xmm1");
            jasm("vpcmpeqb        %ymm1, %ymm2, %ymm2");
            jasm("vpmovmskb       %ymm2, %eax");
            jasm("testl   %eax, %eax");
            jasm("je      " + start_label);

            if (seek_offset > 0)

                jasm("bsfl    %eax, %eax"); // CHANGE IF neg i think

            else
                jasm("bsrl    %eax, %eax");

            jasm("addq %rax, %r13");

            /////////////////////////////////////////////////////////////////
            // save offset

            jasm(end_label + ":");
            print_padding();
        }

        // all other seek loops
        else
        {
            string start_label = "start_seek_loop_" + to_string(loop_num);
            string end_label = "end_seek_loop_" + to_string(loop_num);

            //  Load byte into %cl (lower 8 bits)
            jasm("movb    (%r13), %cl");
            // jump to matching end label if 0
            jasm("cmpb    $0, %cl");
            jasm("je      " + end_label);

            jasm(start_label + ":");

            // remove one from pointer address
            jasm("addq    $" + to_string(seek_offset) + ", %r13");

            // Load byte into %cl (lower 8 bits)
            jasm("movb    (%r13), %cl");
            // jump to matching end label if 0
            jasm("cmpb    $0, %cl");
            jasm("jne      " + start_label);

            jasm(end_label + ":");
        }
    } // end seek

    if (startsWith(token, "closed_form:"))
    {
        // p2 = 0
        // p4 = p0 + p4
        // p0 = 0

        loop_num++;
        myStack.push(loop_num);

        string start_label = "start_loop_" + to_string(loop_num);

        string end_label = "end_loop_" + to_string(loop_num);

        // jump to matching end label if 0
        jasm("cmpb    $0, (%r13)");
        jasm("je      " + end_label);
        jasm(start_label + ":");

        // SETUP
        // Initialize the available registers
        std::vector<std::string> availableRegisters = {"%r8", "%r9", "%r10", "%r11", "%r12", "%r14", "%r15"};
        std::unordered_map<std::string, std::string> registerMap; // Maps a string to a register

        // Function to insert a mapping
        auto insert = [&](const std::string &key)
        {
            if (availableRegisters.empty())
            {
                std::cerr << "No available registers left!\n";
                return;
            }
            // Map the key to the first available register
            registerMap[key] = availableRegisters.back();
            availableRegisters.pop_back(); // Remove the assigned register from the list
        };

        // Function to get the mapped register for a given key
        auto get = [&](const std::string &key) -> std::string
        {
            if (registerMap.find(key) != registerMap.end())
            {
                return registerMap[key];
            }
            else
            {

                cout << "Error in mapping registers." << endl;
                assert(1 == 0);
            }

            return "Not mapped"; // Return a default value if the key is not found
        };

        // END SETUP

        print_padding();
        //
        vector<string> closed_form = get_closed_form_expr_list(token);

        cout << "Found closed form: " << endl;
        // just print the closed form
        for (auto token : closed_form)
            cout << token << endl;

        // jasm("opt: ");
        //  save old variables
        for (auto token : closed_form)
        {
            size_t pos = token.find('=');

            std::string left = token.substr(0, pos);
            // //trim whitespace
            left.erase(0, left.find_first_not_of(" \t"));

            string offset = p_get_offset(left);

            insert(offset + "(%r13)");

            cout << offset + "(%r13)  : " << get(offset + "(%r13)") << endl;

            // save old variables on some place in stack that's easy to map to
            jasm("movq " + offset + "(%r13)," + get(offset + "(%r13)"));

        } // end saving old values loop

        // set the variables
        for (auto token : closed_form)
        {
            size_t pos = token.find('=');

            std::string left = token.substr(0, pos);
            std::string right = token.substr(pos + 1);

            // //trim whitespace
            left.erase(0, left.find_first_not_of(" \t"));
            right.erase(0, right.find_first_not_of(" \t"));

            string offset = p_get_offset(left);

            // reset our value

            jasm("movb $0, " + offset + "(%r13)");

            // add each part of the expr
            right; // string

            // split expression into parts
            vector<string> operations = separate_terms(right);

            for (auto token : operations)
            {
                // cout << token <<endl;
                if (is_number(token))
                {

                    string op = remove_whitespace(token);
                    cout << op << " : is_number " << endl;
                    jasm("addb  $" + op + "," + offset + "(%r13)");
                }

                else if (token[0] == '+')
                {
                    cout << token << " : + pX : ";
                    string op = remove_whitespace(token.substr(1));
                    string op_index = p_get_offset(op);

                    cout << op_index << endl;
                    jasm("addb " + get(op_index + "(%r13)") + "b, " + offset + "(%r13)");
                }

                else if (token[0] == '-')
                {
                    cout << token << " : + pX : ";
                    string op = remove_whitespace(token.substr(1));
                    string op_index = p_get_offset(op);

                    cout << op_index << endl;
                    jasm("subb " + get(op_index + "(%r13)") + "b, " + offset + "(%r13)");
                }

                else
                {
                    cout << token << " : pX : ";
                    string op_index = p_get_offset(token);
                    cout << op_index << endl;

                    jasm("addb " + get(op_index + "(%r13)") + "b, " + offset + "(%r13)");
                }

                // cout << token << endl;
            }

        } // end saving old values loop
        print_padding();

        int match_loop = myStack.top();
        myStack.pop();
        start_label = "start_loop_" + to_string(match_loop);
        end_label = "end_loop_" + to_string(match_loop);

        // jump to matching start label if not 0
        jasm("cmpb    $0, (%r13)");
        jasm("jne      " + start_label);
        jasm(end_label + ":");

    } // end closed_expr
} // end asm_string

vector<string> optimize_seek_loop(int loop_index, int seek_offset, vector<string> loop, vector<string> program)
{
    string sb = "expr_seek:";
    for (int i = 0; i < loop.size(); i++)
    {
        program[loop_index + i] = " ";
    }
    sb += to_string(seek_offset);

    program[loop_index] = sb;
    return program;
}

bool is_simple_loop2(vector<string> loop)
{
    bool answer = true;
    int offset = 0;

    int loop_cell_value = 0;
    // we do not count the brackets only inside ie [...]
    for (int i = 1; i < loop.size() - 1; i++)
    {
        string t = loop[i];
        if (
            t != ">" &&
            t != "<" &&
            !startsWith(t, "expr_simple:") &&
            t != "+" &&
            t != "-" &&
            t != " ")
            answer = false;

        if (t == ">")
            offset++;
        if (t == "<")
            offset--;
        if (t == "+")
        {
            if (offset == 0)
                loop_cell_value++;
        }

        if (t == "-")
        {
            if (offset == 0)
                loop_cell_value--;
        }
    }

    if (offset != 0)
        answer = false;

    if (abs(loop_cell_value) != 1)
        answer = false;

    return answer;
}

void flush_tape_to_asm(int min, int max)
{

    for (int i = min; i < max + 1; i++)
    {
        if (tape[tape_position] != 0)
        {
            int ascii_value = static_cast<int>(tape[tape_position]);
            jasm("movb    $" + to_string(ascii_value) + ", " + to_string(i) + "(%r13)");
        }
    }
    jasm("addq    $" + to_string(tape_position - (tape_size / 2)) + ", %r13");
}

bool contains_user_input(vector<char> prog)
{
    for (auto token : prog)
    {
        if (token == ',')
            return true;
    }

    return false;
}

void call_partial_eval(int final_index)
{
    for (int i = 0; i < final_index; i++)
    {
        char token = program_file[i];
        switch (token)
        {
        case '>':
            tape_position++;
            max_offset = max(max_offset, tape_position - (tape_size / 2));

            break;
        case '<':
            tape_position--;
            min_offset = min(min_offset, tape_position - (tape_size / 2));

            break;
        case '+':
            tape[tape_position] += 1;
            break;
        case '-':
            tape[tape_position] -= 1;
            break;
        case '.':
        {
            int ascii_value = static_cast<int>(tape[tape_position]);
            // Load the byte from the address into %al (to use with putc)
            jasm("movb    $" + to_string(ascii_value) + ", %al");
            // Prepare for putc
            // Load file descriptor for stdout into %rsi
            jasm("movq    stdout(%rip), %rsi");
            // Move and sign-extend byte in %al to %edi
            jasm("movsbl  %al, %edi");
            // Call putc to print the character
            jasm("call    putc@PLT");
        }

        break;
        case ',':

            cout << "shouldn't have a , in partial eval..." << endl;
            assert(1 == 0);
            break;
        case '[':
            if (tape[tape_position] == 0)
            {

                int count = 1;
                while (count != 0)
                {
                    i++;
                    if (program_file[i] == '[')
                        count++;

                    if (program_file[i] == ']')
                        count--;
                }
                i--; // go back one, since the interpreter for loop will i++;
            }

            break;
        case ']':

            if (tape[tape_position] != 0)
            {
                int count = 1;
                while (count != 0)
                {
                    i--;
                    if (program_file[i] == ']')
                        count++;

                    if (program_file[i] == '[')
                        count--;
                };
                i--; // go back one, since the interpreter for loop will i++;
            }

            break;
        default:
            // non bf instruction, so we ignore
            break;
        } // end switch
    }

    program_file.erase(program_file.begin(), program_file.begin() + final_index);
}

int get_first_loop_index(vector<char> prog)
{
    for (int i = 0; i < prog.size(); i++)
    {
        char token = prog[i];
        if (token == '[')
            return i;
    }

    return prog.size();
}
// Function to get a Python string variable as a C++ std::string
std::string _getStringFromPython(const char *varName)
{
    // Get the main module
    PyObject *mainModule = PyImport_AddModule("__main__");

    // Get the dictionary of the main module
    PyObject *mainDict = PyModule_GetDict(mainModule);

    // Get the Python string object
    PyObject *pyString = PyDict_GetItemString(mainDict, varName);

    // Check if the object is a string
    if (pyString && PyUnicode_Check(pyString))
    {
        // Convert the Python string to a C-style string
        const char *cString = PyUnicode_AsUTF8(pyString);
        return std::string(cString); // Return as std::string
    }

    // Return an empty string if not found or not a string
    return "";
}

void test_py()
{
    Py_Initialize(); // Initialize the Python interpreter

    // Import the random module
    PyRun_SimpleString("from random import randint");
    // Import sympy module
    PyRun_SimpleString("import sympy");

    // Define the function to simulate the operation
    PyRun_SimpleString(
        "def function(p1, p2, p3, p0):\n"
        "    while p0 != 0:\n"
        "        p3 += p1\n"
        "        p2 += p1\n"
        "        p1 = p2\n"
        "        p2 = 0\n"
        "        p0 -= 1\n"
        "    return p1, p2, p3, p0\n");

    // Define the polynomial expansion function
    PyRun_SimpleString(
        "def poly_expression(p1, p2, p3, p0):\n"
        "    return (1 + p1 + p1**2) * (1 + p2 + p2**2) * (1 + p3 + p3**2) * (1 + p0 + p0**2)\n");

    // Define the function to get terms from the polynomial
    PyRun_SimpleString(
        "def get_terms(v):\n"
        "    v_expanded = v.expand()\n"
        "    v_terms = []\n"
        "    for term, coeff in v_expanded.as_coefficients_dict().items():\n"
        "        if sympy.total_degree(term) <= 2:\n"
        "            v_terms.append(term)\n"
        "    return v_terms\n");

    // Main function logic
    PyRun_SimpleString(
        "p1, p2, p3, p0 = sympy.symbols('p1 p2 p3 p0')\n"
        "v = poly_expression(p1, p2, p3, p0)\n"
        "v_terms = get_terms(v)\n"
        "coeffs = sympy.symbols(f'c:{len(v_terms)}')\n"
        "exp = sum(c * t for c, t in zip(coeffs, v_terms))\n"
        "points = [tuple(randint(1, 100) for _ in range(4)) for _ in range(len(coeffs))]\n"
        "symbols = [p1, p2, p3, p0]\n"
        "results = []\n"
        "for i, s in enumerate(symbols):\n"
        "    equations = [exp.subs(dict(zip(symbols, pt))) - function(*pt)[i] for pt in points]\n"
        "    sol = sympy.solve(equations, coeffs)\n"
        "    results.append(f'{s} = {exp.subs(sol)}')\n"
        "output_string = '\\n'.join(results)\n"               // Join results with newlines
        "output_string = output_string.replace('\\n', ';')\n" // Replace newlines with semicolons
    );
    // Now retrieve the output_string from Python
    std::string py_output = _getStringFromPython("output_string"); // Make sure getStringFromPython is defined correctly

    Py_Finalize(); // Finalize the Python interpreter
    cout << py_output << endl;
}

/*
Sets up the code for python to be able to compute closed form
sets the 'result' variable in python to the result, could potentially be an infinite loop
but that should only happen if
#1 there's a bug in the python code c: , shouldnt be...
#2 if there's no closed form, which should never happen unless the bf has an infinite loop...
#3 we messed up the creation of the function to pass in... most likely i think??

Example usage of the python code at bottom
*/
void sympy_setup()
{
    Py_Initialize(); // Initialize the Python interpreter

    // Define the Python code as a string
    PyRun_SimpleString(
        "from random import randint\n"
        "import sympy\n"
        "import inspect\n"
        "\n"
        "# Generalized function to simulate an arbitrary operation\n"
        "def simulate_operation(func, *args):\n"
        "    return func(*args)\n"
        "\n"
        "# Generalized polynomial expansion of all possible terms (sympy minimized)\n"
        "def poly_expression(*variables):\n"
        "    # Create a polynomial of the form (1 + var + var^2) for each variable\n"
        "    poly = 1\n"
        "    for var in variables:\n"
        "        poly *= (1 + var + var**2)\n"
        "    return poly\n"
        "\n"
        "# Only keep relevant terms (with degree <= 2)\n"
        "def get_relevant_terms(v):\n"
        "    v_expanded = v.expand()\n"
        "    v_terms = []\n"
        "    for term, coeff in v_expanded.as_coefficients_dict().items():\n"
        "        if sympy.total_degree(term) <= 2:\n"
        "            v_terms.append(term)\n"
        "    return v_terms\n"
        "\n"
        "# Compute the closed form of a function with symbolic variables\n"
        "def compute_closed_form(func):\n"
        "    # Get the argument names and number of arguments the function expects\n"
        "    func_signature = inspect.signature(func)\n"
        "    arg_names = list(func_signature.parameters.keys())\n"
        "    num_args = len(arg_names)\n"
        "\n"
        "    # Dynamically create symbolic variables for each argument\n"
        "    symbols = sympy.symbols(arg_names)  # Use argument names directly as symbols\n"
        "\n"
        "    # Get polynomial and extract relevant terms\n"
        "    v = poly_expression(*symbols)\n"
        "    v_terms = get_relevant_terms(v)\n"
        "    \n"
        "    # Create coefficient symbols\n"
        "    coeffs = sympy.symbols(f'c:{len(v_terms)}')\n"
        "    \n"
        "    # Final expression with terms and coefficients\n"
        "    exp = sum(c * t for c, t in zip(coeffs, v_terms))\n"
        "    \n"
        "    # Generate random points\n"
        "    points = [tuple(randint(1, 100) for _ in range(num_args)) for _ in range(len(coeffs))]\n"
        "    \n"
        "    results = []\n"
        "    \n"
        "    # Solve the equations\n"
        "    for i, s in enumerate(symbols):\n"
        "        equations = [exp.subs(dict(zip(symbols, pt))) - simulate_operation(func, *pt)[i] for pt in points]\n"
        "        sol = sympy.solve(equations, coeffs)\n"
        "        results.append(f'{s} = {exp.subs(sol)}')\n"
        "    \n"
        "    output_string = '\\n'.join(results)\n" // Join results with newlines
        "    output_string = output_string.replace('\\n', ';')\n"
        "    output_string = output_string + ';'\n"
        "    return output_string\n");

    //// EXAMPLE, getting the string back from python
    // get closed form of a function
    // PyRun_SimpleString(
    //     "def sanity_check(p1, p2, p3, p0):\n"
    //     "    while p0 != 0:\n"
    //     "        p2 += 1 * p1\n"
    //     "        p3 += 1 * p1\n"
    //     "        p1 = 0\n"
    //     "        p1 += 1 * p2\n"
    //     "        p2 = 0\n"
    //     "        p0 -= 1\n"
    //     "    return p1, p2, p3, p0\n");
    // PyRun_SimpleString(
    //     "result = compute_closed_form(sanity_check)\n");
    // std::string py_output = _getStringFromPython("result");

    // cout << py_output << endl;
}

void sympy_cleanup()
{
    Py_Finalize();
}

// returns string 'pX', where X is what's passed ie p(3) returns p3
string p(int val)
{
    string val_string;
    if (val >= 0)
        val_string = to_string(val);
    else
        val_string = "_" + to_string(abs(val));

    return "p" + val_string;
}

/*
takes set of strings
ie "p0,p1,p2"
*/
string print_s_set(unordered_set<string> set)
{
    string str = "";
    for (auto token : set)
    {
        str += token;
        str += ",";
    }

    str.pop_back();
    return str;
}
string convert_simple_loop_to_function(vector<string> loop)
{
    // Example loop
    //[>expr_simple:-0:-1,1:1,2:1,        >expr_simple:--1:1,0:-1,     <<-]
    // should turn into:
    string example = "def sanity_check(p1, p2, p3, p0):\n"
                     "    while p0 != 0:\n"
                     "        p2 += 1 * p1\n"
                     "        p3 += 1 * p1\n"
                     "        p1 = 0\n"
                     "        p1 += 1 * p2\n"
                     "        p2 = 0\n"
                     "        p0 -= 1\n"
                     "    return p1, p2, p3, p0\n";

    unordered_set<string> terms;
    string body = "    while p0 != 0:\n";

    int offset = 0;

    //[>expr_simple:-0:-1,1:1,2:1,        >expr_simple:--1:1,0:-1,     <<-]
    for (auto token : loop)
    {
        if (startsWith(token, "expr_simple:"))
        {
            map<int, int> simple_loop = expr_string_to_dict(token);
            for (const auto &pair : simple_loop)
            {
                if (pair.first == 0)
                    continue;

                to_string(pair.first);
                to_string(pair.second);
                int index = pair.first + offset;
                body += "        " + p(index) + "+= " + to_string(pair.second) + " * " + p(offset) + "\n";
                terms.insert(p(index));
                terms.insert(p(offset));
            }
            // zero out at end of loop
            body += "        " + p(offset) + " = 0\n";

            terms.insert(p(offset));
        }

        if (token == ">")
            offset++;
        if (token == "<")
            offset--;
        if (token == "-")
        {
            body += "        " + p(offset) + "-= 1\n";
            terms.insert(p(offset));
        }
        if (token == "+")
        {
            body += "        " + p(offset) + "+= 1\n";
            terms.insert(p(offset));
        }

    } // end for loop

    if (offset != 0)
    {
        cout << "somehow u broke the function maker.." << endl;
        assert(1 == 0);
    }

    string def = "def sanity_check(" + print_s_set(terms) + "):\n";
    string ret = "    return " + print_s_set(terms) + "\n";

    if (terms.size() > 7)
        return "NONE";
    // cout<< (def + body + ret) <<endl;
    // return example
    return (def + body + ret);
}

vector<string> optimize_closed_form(int loop_index, string new_loop, vector<string> loop, vector<string> program)
{

    for (int i = 0; i < loop.size(); i++)
    {
        program[loop_index + i] = " ";
    }
    program[loop_index] = new_loop;

    return program;
}

bool string_contains_multiply(string str)
{

    bool answer = false;
    for (auto token : str)
    {
        if (token == '*')
            answer = true;
    }
    return answer;
}
int main(int argc, char *argv[])
{

    // setup

    if (argc < 2)
    {
        cout << "No input file?" << endl;
        return 1;
    }
    // set flags
    for (int i = 0; i < argc; i++)
    {
        string args = argv[i];

        if (args == "-O")
            simple_loop_flag = true;
        if (args == "-v")
            seek_flag = true;
        if (args == "-O1")
            optimization_flag = true;
        if (args.find(".b") != std::string::npos)
        {
            assert(bf_file_name == "");

            bf_file_name = args;
        }
    }

    // Open bf file
    ifstream inputFile(bf_file_name);
    if (!inputFile)
    {
        cout << "Couldn't open file: " << bf_file_name << endl;
        return 1;
    }
    // Read the file into a vector of chars
    program_file.assign((istreambuf_iterator<char>(inputFile)),
                        istreambuf_iterator<char>());
    // Close the file
    inputFile.close();

    // create our output file
    ofstream outFile("bf.s");
    output_file = &outFile;

    if (!outFile)
    {
        cout << "could not create output file" << endl;
        return 2; // Return with an error code
    }

    /*
    DONT TOUCH register r13! it's where we keep our current cell address
    */
    asm_setup();

    sympy_setup();

    // // doesn't contain user input so let's just do full partial eval!

    if (0 && !contains_user_input(program_file))
    {

        call_partial_eval(program_file.size());
    }
    // else
    // call_partial_eval(get_first_loop_index(program_file));
    // flush_tape_to_asm(min_offset, max_offset);
    else
    {

        vector<string> optimized_program = init_optimized_program_list(program_file);

        // print_string_vector(optimized_program);

        // print program without non-instructions
        // print_string_vector(optimized_program);
        unordered_set<int> loop_indices = get_loop_indices(optimized_program);

        // optimize all simple loops and seek loops
        for (auto token : loop_indices)
        {
            vector<string> loop = get_loop_string(token, optimized_program);
            if (is_simple_loop(loop))
            {
                int loop_increment = get_current_cell_change(loop);
                optimized_program = optimize_simple_loop(token, loop_increment, loop, optimized_program);

                // print_string_vector(optimized_program);
            } // end is simple loop

            if (is_seek_loop(loop))
            {
                int seek_offset = is_seek_loop(loop);

                optimized_program = optimize_seek_loop(token, seek_offset, loop, optimized_program);
            } // end is power two

        } // end looping over loop in program list

        // Get loop indices again, ;d
        loop_indices = get_loop_indices(optimized_program);

        // alright now lets retake a look and see if we can simplify the  new loops...
        for (auto token : loop_indices)
        {
            // token contains index of our loop!
            //  so we can optimize this loop variable and then just plug it back in
            vector<string> loop = get_loop_string(token, optimized_program);

            if (is_simple_loop2(loop))
            {
                // vprint_string_vector(loop);

                //  if (loop.size() > 100)
                //      continue;
                // print_string_vector(optimized_program);

                // loop as a function
                string function_string = convert_simple_loop_to_function(loop);

                // we had more than 7 terms... too much
                if (function_string == "NONE")
                    continue;
                // declare function in pyth     on
                PyRun_SimpleString(function_string.c_str());
                print_string_vector(loop);

                // print function that we'll be passing
                // cout << function_string << endl;
                //  compute closed form in py
                PyRun_SimpleString("result = compute_closed_form(sanity_check)\n");
                // extract answer as string to c++
                std::string py_output = _getStringFromPython("result");
                // encoding to pass to assembler
                string new_loop = "closed_form:" + py_output;

                cout << new_loop << endl
                     << endl;

                // lets start with additions only...
                if (!string_contains_multiply(new_loop))
                {
                    // print_string_vector(optimized_program);
                    // cout << endl;
                    optimized_program = optimize_closed_form(token, new_loop, loop, optimized_program);
                }

                //  Assuming token is defined and points to the correct index
            }
        } // end indices loop

        // print_string_vector(optimized_program);

        // output the assembly
        for (int i = 0; i < optimized_program.size(); i++)
        {
            string token = optimized_program[i];
            bf_string_assembler(token);
        }
    }

    // print_string_vector(optimized_program);

    if (!optimization_flag && !seek_flag && !simple_loop_flag)
    {
        // // begin our program compiler loop
        for (int i = 0; i < program_file.size(); i++)
        {
            char ch = program_file[i];
            bf_assembler(ch);
        }
    }

    asm_cleanup();
    sympy_setup();

    // Close the file
    outFile.close();

} // end main()
