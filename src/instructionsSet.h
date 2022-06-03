#pragma once

#include <string>
#include <map>

struct InstructionInfo 
{
	std::string operands[2];
	uint8_t operandsSizes[2];
};

struct Opcode 
{
	uint8_t opcode;
	uint8_t opcodeExtension;

	bool operator<(const Opcode& other) const noexcept
	{
		return opcode < other.opcode;
	}
};

extern const std::map<std::string, uint8_t> dataDefiningInstructions;

extern const std::map<std::string, const std::multimap<Opcode, InstructionInfo>> instructions;

bool checkInstuction(std::string instruction);

std::pair<int8_t, int8_t> getInstructionOpcode(const std::string& instruction, const std::string& operandA, const std::string operandB, uint8_t *sizeA, uint8_t *sizeB, bool isSizeAIdentical = false, bool isSizeBIdentical = false);