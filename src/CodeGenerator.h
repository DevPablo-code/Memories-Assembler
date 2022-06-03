#pragma once

#include <vector>
#include <map>
#include <sstream>
#include "Instruction.h"

enum class LabelType 
{
	ADDRESS_LABEL,
	DATA_LABEL,
};

struct Label 
{
	LabelType type;
	uint16_t address;
};

struct LabelToPatch 
{
	uint16_t outputAddress;
	uint16_t relativeTo;
	uint8_t size;
};

struct AddresingMode 
{
	uint8_t mod : 2;
	uint8_t reg : 3;
	uint8_t rm : 3;

	uint8_t to_uint8t()
	{
		return mod << 6 | reg << 3 | rm;
	}
};

struct MemoryAddresing 
{
	AddresingMode addressingMode;
	int16_t displacement;
	uint8_t displacementSize;
};

class CodeGenerator 
{
	public:
		CodeGenerator(std::vector<Instruction>& instructions);
		std::stringstream& generate();
	private:
		std::vector<Instruction>& instructions;

		std::stringstream output;

		uint16_t startAddress = 0;
		uint16_t address = 0;
		uint16_t instructionAddress = 0;

		std::map<std::string, Label> labels = { {"", {} } };

		std::map<std::string, LabelToPatch> labelsToPatch = {};

		std::string currentLabel = "";

		void encodeInstructions();

		void encodeInstruction(const Instruction& instruction);

		void resolveGetaddressOperators(Node *node);
		Node* findNumber(Node* node);

		void orgInstruction(const Instruction& instruction);
		void defineDataInstruction(const Instruction& instruction);
		void noOperandsInstruction(const Instruction& instruction);
		void registerInstruction(const Instruction& instruction);
		void numberInstruction(const Instruction& instruction);
		void labelInstruction(const Instruction& instruction, const std::string& label);

		void RegisterAndRegisterInstruction(const Instruction& instruction, const std::string& operandA, const std::string& operandB);
		void GPRAndNumberInstruction(const Instruction& instruction);
		void GPRAndOffsetInstruction(const Instruction& instruction, const std::string& label);
		void RegisterAndMemoryAddressingInstruction(const Instruction& instruction, const std::string& operandA, const std::string& operandB);
		void MemoryAddressingAndRegisterInstruction(const Instruction& instruction, const std::string& operandA, const std::string& operandB);
		void MemoryAddressingAndNumberInstruction(const Instruction& instruction);

		void RegisterAndLabelInstruction(const Instruction& instruction, const std::string& operandA, const std::string& operandB, const std::string& label);
		void LabelAndRegisterInstruction(const Instruction& instruction, const std::string& operandA, const std::string& operandB, const std::string& label);
		void LabelAndNumberInstruction(const Instruction& instruction, const std::string& label);
		void LabelAndOffsetInstruction(const Instruction& instruction, const std::string& label, const std::string& secondLabel);

		MemoryAddresing resolveMemoryAddressing(Node *node);

		void streamNumber(int64_t number, uint16_t size);
		static void error(uint16_t line, const std::string& message);
};