
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

#include "Lexer.h"
#include "Parser.h"
#include "CodeGenerator.h"

int main(int argc, char **argv) 
{
	std::ios::sync_with_stdio(false);

	if (argc < 2) {
		std::cout << "Program path not specified" << '\n';
		return -1;
	}

	std::filesystem::path path = argv[1];

	std::ifstream inputFile(path);
	std::stringstream ss;

	if (!inputFile.is_open())
	{
		std::cout << "Invalid path specified" << '\n';
		return -1;
	}

	ss << inputFile.rdbuf();
	inputFile.close();

	Lexer lexer(ss.str());
	std::vector<Token>& tokens = lexer.tokenize();

	std::cout << "Got " << tokens.size() - 1 << " tokens" << '\n';

	Parser parser(tokens);
	std::vector<Instruction>& instructions = parser.parse();

	std::cout << "Got " << instructions.size() << " instructions" << '\n';

	CodeGenerator codeGenerator(instructions);

	std::ofstream outputFile(path.replace_extension("bin"));

	if (!outputFile.is_open())
	{
		std::cout << "Can`t create or open output file" << '\n';
		return -1;
	}

	outputFile << codeGenerator.generate().rdbuf();

	outputFile.close();
}