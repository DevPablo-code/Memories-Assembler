
#include "CodeGenerator.h"
#include "instructionsSet.h"
#include "getNumberSize.h"
#include "Parser.h"

CodeGenerator::CodeGenerator(std::vector<Instruction>& instructions)
:instructions(instructions)
{
}

std::stringstream& CodeGenerator::generate()
{
	encodeInstructions();

	return output;
}

void CodeGenerator::encodeInstructions()
{
	for (const auto& instruction : instructions)
	{
		instructionAddress = address;
		if (instruction.token.type == TokenType::INSTRUCTION)
		{
			encodeInstruction(instruction);
		}
		else if (instruction.token.type == TokenType::DATA_DEFINING_INSTRUCTION)
		{
			defineDataInstruction(instruction);
		}
		else if (instruction.token.type == TokenType::LOCAL_LABEL_DECLARATION) 
		{
			patchLabel(currentLabel + instruction.token.stringValue);
			labels.insert({ currentLabel + instruction.token.stringValue, { LabelType::ADDRESS_LABEL, address } });
		}
		else if (instruction.token.type == TokenType::GLOBAL_LABEL_DECLARATION || instruction.token.type == TokenType::DATA_LABEL_DECLARATION)
		{
			patchLabel(instruction.token.stringValue);
			labels.insert({ instruction.token.stringValue, { LabelType::ADDRESS_LABEL, address } });
		}
	}
	for (const auto& [labelToPatch, _] : labelsToPatch) 
	{
		std::cout << (labelToPatch  + ": not found") << '\n';
	}
}

void CodeGenerator::encodeInstruction(const Instruction& instruction)
{
	if (instruction.token.stringValue == "org")
	{
		orgInstruction(instruction);
	} 
	else
	{
		switch (instruction.arguments.size())
		{
			case 0: 
			{
				noOperandsInstruction(instruction);
				break;
			}
			case 1: 
			{
				switch (instruction.arguments.at(0).token.type) 
				{
					case TokenType::REGISTER:
					case TokenType::SEGMENT_REGISTER:
					{
						registerInstruction(instruction);
						break;
					}
					case TokenType::NUMBER: 
					{
						numberInstruction(instruction);
						break;
					}
					case TokenType::ARITHMETIC_BINARY_OPERATOR:
					case TokenType::ARITHMETIC_UNARY_OPERATOR: 
					case TokenType::GETSTARTADDRESS_OPERATOR:
					case TokenType::GETCURRENTADDRESS_OPERATOR:
					case TokenType::GETPROGRAMSIZE_OPERATOR:
					{
						resolveGetaddressOperators((Node*)&instruction.arguments.at(0));
						numberInstruction(instruction);
						break;
					}
					case TokenType::LOCAL_LABEL:
					{
						labelInstruction(instruction, currentLabel + instruction.arguments.at(0).token.stringValue);
						break;
					}
					case TokenType::GLOBAL_LABEL:
					{
						labelInstruction(instruction, instruction.arguments.at(0).token.stringValue);
						break;
					}
				}
				break;
			}
			case 2: 
			{
				if (instruction.arguments.at(0).token.type == TokenType::REGISTER && instruction.arguments.at(1).token.type == TokenType::REGISTER) 
				{
					RegisterAndRegisterInstruction(instruction, "G", "G");
				}

				else if (instruction.arguments.at(0).token.type == TokenType::REGISTER && instruction.arguments.at(1).token.type == TokenType::SEGMENT_REGISTER)
				{
					RegisterAndRegisterInstruction(instruction, "G", "S");
				}

				else if (instruction.arguments.at(0).token.type == TokenType::SEGMENT_REGISTER && instruction.arguments.at(1).token.type == TokenType::REGISTER)
				{
					RegisterAndRegisterInstruction(instruction, "S", "G");
				}

				else if (instruction.arguments.at(0).token.type == TokenType::REGISTER && instruction.arguments.at(1).token.type == TokenType::NUMBER) 
				{
					GPRAndNumberInstruction(instruction);
				}

				else if (instruction.arguments.at(0).token.type == TokenType::REGISTER && instruction.arguments.at(1).token.type == TokenType::GETOFFSET_OPERATOR)
				{
					switch (instruction.arguments.at(1).right->token.type)
					{
						case TokenType::LOCAL_LABEL: 
						{
							GPRAndOffsetInstruction(instruction, currentLabel + instruction.arguments.at(1).right->token.stringValue);
							break;
						}
						case TokenType::GLOBAL_LABEL:
						{
							GPRAndOffsetInstruction(instruction, instruction.arguments.at(1).right->token.stringValue);
							break;
						}
					}
				}

				else if (instruction.arguments.at(0).token.type == TokenType::REGISTER && instruction.arguments.at(1).token.type == TokenType::MEMORY_ADDRESSING)
				{
					RegisterAndMemoryAddressingInstruction(instruction, "G", "M");
				}

				else if (instruction.arguments.at(0).token.type == TokenType::MEMORY_ADDRESSING && instruction.arguments.at(1).token.type == TokenType::REGISTER)
				{
					MemoryAddressingAndRegisterInstruction(instruction, "M", "G");
				}

				else if (instruction.arguments.at(0).token.type == TokenType::MEMORY_ADDRESSING && instruction.arguments.at(1).token.type == TokenType::NUMBER)
				{
					MemoryAddressingAndNumberInstruction(instruction);
				}
				else if (instruction.arguments.at(0).token.type == TokenType::SEGMENT_REGISTER && instruction.arguments.at(1).token.type == TokenType::MEMORY_ADDRESSING)
				{
					RegisterAndMemoryAddressingInstruction(instruction, "S", "M");
				}

				else if (instruction.arguments.at(0).token.type == TokenType::MEMORY_ADDRESSING && instruction.arguments.at(1).token.type == TokenType::SEGMENT_REGISTER)
				{
					RegisterAndMemoryAddressingInstruction(instruction, "M", "S");
				}

				else if (instruction.arguments.at(0).token.type == TokenType::REGISTER && instruction.arguments.at(1).token.type == TokenType::GLOBAL_LABEL)
				{
					RegisterAndLabelInstruction(instruction, "G", "M",instruction.arguments.at(1).token.stringValue);
				}

				else if (instruction.arguments.at(0).token.type == TokenType::REGISTER && instruction.arguments.at(1).token.type == TokenType::LOCAL_LABEL)
				{
					RegisterAndLabelInstruction(instruction, "G", "M", currentLabel + instruction.arguments.at(1).token.stringValue);
				}

				else if (instruction.arguments.at(0).token.type == TokenType::GLOBAL_LABEL && instruction.arguments.at(1).token.type == TokenType::REGISTER)
				{
					LabelAndRegisterInstruction(instruction, "M", "G", instruction.arguments.at(0).token.stringValue);
				}

				else if (instruction.arguments.at(0).token.type == TokenType::LOCAL_LABEL && instruction.arguments.at(1).token.type == TokenType::REGISTER)
				{
					LabelAndRegisterInstruction(instruction, "M", "G", currentLabel + instruction.arguments.at(1).token.stringValue);
				}

				else if (instruction.arguments.at(0).token.type == TokenType::SEGMENT_REGISTER && instruction.arguments.at(1).token.type == TokenType::GLOBAL_LABEL)
				{
					RegisterAndLabelInstruction(instruction, "S", "M", instruction.arguments.at(1).token.stringValue);
				}

				else if (instruction.arguments.at(0).token.type == TokenType::SEGMENT_REGISTER && instruction.arguments.at(1).token.type == TokenType::LOCAL_LABEL)
				{
					RegisterAndLabelInstruction(instruction, "S", "M", currentLabel + instruction.arguments.at(1).token.stringValue);
				}

				else if (instruction.arguments.at(0).token.type == TokenType::GLOBAL_LABEL && instruction.arguments.at(1).token.type == TokenType::SEGMENT_REGISTER)
				{
					LabelAndRegisterInstruction(instruction, "M", "S", instruction.arguments.at(0).token.stringValue);
				}

				else if (instruction.arguments.at(0).token.type == TokenType::LOCAL_LABEL && instruction.arguments.at(1).token.type == TokenType::SEGMENT_REGISTER)
				{
					LabelAndRegisterInstruction(instruction, "M", "S", currentLabel + instruction.arguments.at(0).token.stringValue);
				}

				else if (instruction.arguments.at(0).token.type == TokenType::GLOBAL_LABEL && instruction.arguments.at(1).token.type == TokenType::NUMBER)
				{
					LabelAndNumberInstruction(instruction, instruction.arguments.at(0).token.stringValue);
				}

				else if (instruction.arguments.at(0).token.type == TokenType::LOCAL_LABEL && instruction.arguments.at(1).token.type == TokenType::NUMBER)
				{
					LabelAndNumberInstruction(instruction, currentLabel + instruction.arguments.at(0).token.stringValue);
				}

				else if (instruction.arguments.at(0).token.type == TokenType::GLOBAL_LABEL && instruction.arguments.at(1).token.type == TokenType::NUMBER)
				{
					LabelAndNumberInstruction(instruction, instruction.arguments.at(0).token.stringValue);
				}

				else if (instruction.arguments.at(0).token.type == TokenType::GLOBAL_LABEL && instruction.arguments.at(1).token.type == TokenType::GETOFFSET_OPERATOR)
				{
					switch (instruction.arguments.at(1).right->token.type)
					{
						case TokenType::LOCAL_LABEL:
						{
							LabelAndOffsetInstruction(instruction, instruction.arguments.at(0).token.stringValue, currentLabel + instruction.arguments.at(1).right->token.stringValue);
							break;
						}
						case TokenType::GLOBAL_LABEL:
						{
							LabelAndOffsetInstruction(instruction, instruction.arguments.at(0).token.stringValue, instruction.arguments.at(1).right->token.stringValue);
							break;
						}
					}
				}
				else if (instruction.arguments.at(0).token.type == TokenType::GLOBAL_LABEL && instruction.arguments.at(1).token.type == TokenType::GETOFFSET_OPERATOR)
				{
					switch (instruction.arguments.at(1).right->token.type)
					{
						case TokenType::LOCAL_LABEL:
						{
							LabelAndOffsetInstruction(instruction, currentLabel + instruction.arguments.at(0).token.stringValue, currentLabel + instruction.arguments.at(1).right->token.stringValue);
							break;
						}
						case TokenType::GLOBAL_LABEL:
						{
							LabelAndOffsetInstruction(instruction, currentLabel + instruction.arguments.at(0).token.stringValue, instruction.arguments.at(1).right->token.stringValue);
							break;
						}
					}
				}
				else 
				{
					error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
				}
				break;
			}       
			default: 
			{
				error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
				break;
			}
		}
	}
}

void CodeGenerator::resolveGetaddressOperators(Node *node)
{
	if (!node) 
	{
		return;
	}
	switch(node->token.type)
	{
		case TokenType::NONE: 
		{
			return;
		}
		case TokenType::GETCURRENTADDRESS_OPERATOR: 
		{
			node->token.type = TokenType::NUMBER;
			node->token.numberValue = instructionAddress;
			break;
		}
		case TokenType::GETSTARTADDRESS_OPERATOR: 
		{
			node->token.type = TokenType::NUMBER;
			node->token.numberValue = startAddress;
			break;
		}
		case TokenType::GETPROGRAMSIZE_OPERATOR: {
			node->token.type = TokenType::NUMBER;
			node->token.numberValue = instructionAddress - startAddress;
			break;
		}
		default: 
		{
			break;
		}
	}
	resolveGetaddressOperators(node->left.get());
	resolveGetaddressOperators(node->right.get());
}

void CodeGenerator::patchLabel(const std::string& label)
{
	auto range = labelsToPatch.equal_range(label);
	std::multimap<std::string, LabelToPatch>::iterator it = range.first;
	while (it != range.second)
	{
		output.seekp(it->second.outputAddress - startAddress);
		streamNumber(address - it->second.relativeTo, it->second.size);
		output.seekp(0, output.end);
		it = labelsToPatch.erase(it);
	}
}

Node* CodeGenerator::findNumber(Node* node)
{
	if (!node || node->token.type == TokenType::NUMBER)
	{
		return node;
	}

	Node* left = findNumber(node->left.get());
	if (left) 
	{
		return left;
	}

	Node* right = findNumber(node->right.get());
	if (right) 
	{
		return right;
	}

	return nullptr;
}

void CodeGenerator::orgInstruction(const Instruction& instruction)
{
	startAddress = instruction.arguments.at(0).token.numberValue;
	address = startAddress;
}

void CodeGenerator::defineDataInstruction(const Instruction& instruction)
{
	for (auto& argument : instruction.arguments)
	{
		if (argument.token.type == TokenType::NUMBER)
		{
			streamNumber(argument.token.numberValue, instruction.token.size);
			address += instruction.token.size;
		}
		else if (argument.token.type == TokenType::DUPDATA_OPERATOR) 
		{
			resolveGetaddressOperators(argument.left.get());
			resolveGetaddressOperators(argument.right.get());
			Node amount = Parser::performArithmeticOperations(*argument.left.get());
			Node data = Parser::performArithmeticOperations(*argument.right.get());
			for (int i = 0; i < amount.token.numberValue; i++) 
			{
				streamNumber(data.token.numberValue, instruction.token.size);
			}
			address += instruction.token.size * amount.token.numberValue;
		}
		else 
		{
			error(argument.token.line, instruction.token.stringValue + ": data expected");
		}
	}
}

void CodeGenerator::noOperandsInstruction(const Instruction& instruction)
{
	if (const auto [opcode, _] = getInstructionOpcode(instruction.token.stringValue, "", "", 0, 0, true, true); opcode != -1)
	{   
		output << (uint8_t)opcode;

		address += 1;
	}
	else
	{
		error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
	}
}

void CodeGenerator::registerInstruction(const Instruction& instruction)
{
	if (const auto [opcode, _] = getInstructionOpcode(instruction.token.stringValue, instruction.arguments.at(0).token.stringValue, "", (uint8_t*)&instruction.arguments.at(0).token.size, 0, true, true); opcode != -1)
	{
		output << (uint8_t)opcode;

		address += 1;
	}
	else
	{
		error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
	}
}

void CodeGenerator::numberInstruction(const Instruction& instruction)
{
	if (const auto [opcode, _] = getInstructionOpcode(instruction.token.stringValue, "I", "", (uint8_t*)&instruction.arguments.at(0).token.size, 0, false, false); opcode != -1)
	{
		output << (uint8_t)opcode;

		address += 1;

		streamNumber(instruction.arguments.at(0).token.numberValue, instruction.arguments.at(0).token.size);

		address += instruction.arguments.at(0).token.size;
	}
	else
	{
		error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
	}
}

void CodeGenerator::labelInstruction(const Instruction& instruction, const std::string &label)
{
	uint8_t offsetSize = 1;
	if (const auto [opcode, _] = getInstructionOpcode(instruction.token.stringValue, "J", "", &offsetSize, 0, false, false); opcode != -1)
	{
		output << (uint8_t)opcode;

		address += 1;

		address += offsetSize;

		if (const auto& it = labels.find(label); it != labels.end())
		{
			streamNumber(it->second.address - address, offsetSize);
		}
		else
		{
			streamNumber(0, offsetSize);
			labelsToPatch.insert({ label, { (uint16_t)(address - offsetSize), address, offsetSize } });
		}
	}
	else
	{
		error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
	}
}

void CodeGenerator::RegisterAndRegisterInstruction(const Instruction& instruction, const std::string& operandA, const std::string& operandB)
{
	if (const auto [opcode, _] = getInstructionOpcode(instruction.token.stringValue, operandA, operandB, (uint8_t*)&instruction.arguments.at(0).token.size, (uint8_t*)&instruction.arguments.at(1).token.size, true, true); opcode != -1)
	{
		output << (uint8_t)opcode;

		address += 1;

		AddresingMode addressingMode = { 0b11, instruction.arguments.at(0).token.numberValue, instruction.arguments.at(1).token.numberValue };

		output << addressingMode.to_uint8t();

		address += 1;
	}
	else
	{
		error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
	}
}

void CodeGenerator::GPRAndNumberInstruction(const Instruction& instruction)
{
	if (const auto [opcode, _] = getInstructionOpcode(instruction.token.stringValue, instruction.arguments.at(0).token.stringValue, "I", (uint8_t*)&instruction.arguments.at(0).token.size, (uint8_t*)&instruction.arguments.at(1).token.size, true, false); opcode != -1)
	{
		output << (uint8_t)opcode;

		address += 1;

		streamNumber(instruction.arguments.at(1).token.numberValue, instruction.arguments.at(1).token.size);

		address += instruction.arguments.at(1).token.size;
	}
	else if (const auto [opcode, extension] = getInstructionOpcode(instruction.token.stringValue, "G", "I", (uint8_t*)&instruction.arguments.at(0).token.size, (uint8_t*)&instruction.arguments.at(1).token.size, true, false); opcode != -1)
	{
		output << (uint8_t)opcode;

		address += 1;

		AddresingMode addressingMode = { 0b00, instruction.arguments.at(0).token.numberValue, 0 };

		if (extension != -1) 
		{
			addressingMode.mod = 0b11;
			addressingMode.reg = extension;
			addressingMode.rm = instruction.arguments.at(0).token.numberValue;
		}

		output << addressingMode.to_uint8t();

		address += 1;

		streamNumber(instruction.arguments.at(1).token.numberValue, instruction.arguments.at(1).token.size);

		address += instruction.arguments.at(1).token.size;
	}
	else
	{
		error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
	}
}

void CodeGenerator::GPRAndOffsetInstruction(const Instruction& instruction, const std::string& label)
{
	uint8_t offsetSize = 2;
	if (const auto [opcode, _] = getInstructionOpcode(instruction.token.stringValue, instruction.arguments.at(0).token.stringValue, "I", (uint8_t*)&instruction.arguments.at(0).token.size, &offsetSize, false, false); opcode != -1)
	{
		output << (uint8_t)opcode;

		address += 1;

		address += offsetSize;

		if (const auto& it = labels.find(label); it != labels.end())
		{
			streamNumber(it->second.address - address, offsetSize);
		}
		else
		{
			streamNumber(0, offsetSize);
			labelsToPatch.insert({ label, { (uint16_t)(address - offsetSize), 0, offsetSize } });
		}
	}
	else
	{
		error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
	}
}

void CodeGenerator::RegisterAndMemoryAddressingInstruction(const Instruction& instruction, const std::string& operandA, const std::string& operandB)
{
	MemoryAddresing memoryAddressing = resolveMemoryAddressing(instruction.arguments.at(1).right.get());

	if (const auto [opcode, _] = getInstructionOpcode(instruction.token.stringValue, "G", "M", (uint8_t*)&instruction.arguments.at(0).token.size, (uint8_t*)&instruction.arguments.at(1).token.size, true, false); opcode != -1)
	{
		output << (uint8_t)opcode;

		address += 1;

		AddresingMode addresingMode = { memoryAddressing.addressingMode.mod, instruction.arguments.at(0).token.numberValue, memoryAddressing.addressingMode.rm };

		output << addresingMode.to_uint8t();

		address += 1;

		streamNumber(memoryAddressing.displacement, memoryAddressing.displacementSize);

		address += memoryAddressing.displacementSize;
	}
	else
	{
		error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
	}
}

void CodeGenerator::MemoryAddressingAndRegisterInstruction(const Instruction& instruction, const std::string& operandA, const std::string& operandB)
{
	MemoryAddresing memoryAddressing = resolveMemoryAddressing(instruction.arguments.at(0).right.get());

	if (const auto [opcode, _] = getInstructionOpcode(instruction.token.stringValue, operandA, operandB, (uint8_t*)&instruction.arguments.at(0).token.size, (uint8_t*)&instruction.arguments.at(1).token.size, true, false); opcode != -1)
	{
		output << (uint8_t)opcode;

		address += 1;

		AddresingMode addresingMode = { memoryAddressing.addressingMode.mod, instruction.arguments.at(1).token.numberValue, memoryAddressing.addressingMode.rm };

		output << addresingMode.to_uint8t();

		address += 1;

		streamNumber(memoryAddressing.displacement, memoryAddressing.displacementSize);

		address += memoryAddressing.displacementSize;
	}
	else
	{
		error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
	}
}

void CodeGenerator::MemoryAddressingAndNumberInstruction(const Instruction& instruction)
{
	MemoryAddresing memoryAddressing = resolveMemoryAddressing(instruction.arguments.at(0).right.get());

	if (const auto [opcode, extension] = getInstructionOpcode(instruction.token.stringValue, "M", "I", (uint8_t*)&instruction.arguments.at(0).token.size, (uint8_t*)&instruction.arguments.at(1).token.size, true, false); opcode != -1)
	{
		output << (uint8_t)opcode;

		address += 1;

		AddresingMode addresingMode = { memoryAddressing.addressingMode.mod, memoryAddressing.addressingMode.rm, 0b000 };

		if (extension != -1)
		{
			addresingMode.reg = extension;
			addresingMode.rm = memoryAddressing.addressingMode.rm;
		}

		output << addresingMode.to_uint8t();

		address += 1;

		streamNumber(memoryAddressing.displacement, memoryAddressing.displacementSize);

		address += memoryAddressing.displacementSize;

		streamNumber(instruction.arguments.at(1).token.numberValue, instruction.arguments.at(1).token.size);

		address += instruction.arguments.at(1).token.size;
	}
	else
	{
		error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
	}
}

void CodeGenerator::RegisterAndLabelInstruction(const Instruction& instruction, const std::string& operandA, const std::string& operandB, const std::string& label)
{
	uint8_t offsetSize = 0;
	if (const auto [opcode, _] = getInstructionOpcode(instruction.token.stringValue, "G", "M", (uint8_t*)&instruction.arguments.at(0).token.size, &offsetSize, true, false); opcode != -1)
	{
		output << (uint8_t)opcode;

		address += 1;

		AddresingMode addresingMode = { 0b00, instruction.arguments.at(0).token.numberValue, 0b110 };

		output << addresingMode.to_uint8t();
		
		address += 1;

		address += 2;

		if (const auto& it = labels.find(label); it != labels.end())
		{
			streamNumber(it->second.address - address, 2);
		}
		else
		{
			streamNumber(0, 2);
			labelsToPatch.insert({ label, { (uint16_t)(address - 2), 0, 2 } });
		}
	}
	else
	{
		error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
	}
}

void CodeGenerator::LabelAndRegisterInstruction(const Instruction& instruction, const std::string& operandA, const std::string& operandB, const std::string& label)
{
	uint8_t offsetSize = 0;
	if (const auto [opcode, _] = getInstructionOpcode(instruction.token.stringValue, "M", "G", &offsetSize, (uint8_t*)&instruction.arguments.at(1).token.size, false, true); opcode != -1)
	{
		output << (uint8_t)opcode;

		address += 1;

		AddresingMode addresingMode = { 0b00, instruction.arguments.at(1).token.numberValue, 0b110 };

		output << addresingMode.to_uint8t();

		address += 1;

		address += 2;

		if (const auto& it = labels.find(label); it != labels.end())
		{
			streamNumber(it->second.address - address, 2);
		}
		else
		{
			streamNumber(0, 2);
			labelsToPatch.insert({ label, { (uint16_t)(address - 2), 0, 2 } });
		}
	}
	else
	{
		error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
	}
}

void CodeGenerator::LabelAndNumberInstruction(const Instruction& instruction, const std::string& label)
{
	uint8_t offsetSize = 0;
	if (const auto [opcode, extension] = getInstructionOpcode(instruction.token.stringValue, "M", "I", &offsetSize, (uint8_t*)&instruction.arguments.at(1).token.size, false, false); opcode != -1)
	{
		output << (uint8_t)opcode;

		address += 1;

		AddresingMode addresingMode = { 0b00, 0b110, 0b000 };

		if (extension != -1)
		{
			addresingMode.reg = extension;
			addresingMode.rm = 0b110;
		}

		output << addresingMode.to_uint8t();

		address += 1;

		address += 2;

		if (const auto& it = labels.find(label); it != labels.end())
		{
			streamNumber(it->second.address - address, 2);
		}
		else
		{
			streamNumber(0, 2);
			labelsToPatch.insert({ label, { (uint16_t)(address - 2), 0, 2 } });
		}

		streamNumber(instruction.arguments.at(1).token.numberValue, instruction.arguments.at(1).token.size);

		address += instruction.arguments.at(1).token.size;
	}
	else
	{
		error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
	}
}

void CodeGenerator::LabelAndOffsetInstruction(const Instruction& instruction, const std::string& label, const std::string& secondLabel)
{
	uint8_t offsetSize = 0;
	uint8_t secondOffsetSize = 2;
	if (const auto [opcode, extension] = getInstructionOpcode(instruction.token.stringValue, "M", "I", &offsetSize, &secondOffsetSize, false, false); opcode != -1)
	{
		output << (uint8_t)opcode;

		address += 1;

		AddresingMode addresingMode = { 0b00, 0b110, 0b000 };

		if (extension != -1)
		{
			addresingMode.reg = extension;
			addresingMode.rm = 0b110;
			offsetSize = 2;
		}

		output << addresingMode.to_uint8t();

		address += 1;

		address += 2;

		if (const auto& it = labels.find(label); it != labels.end())
		{
			streamNumber(it->second.address - address, 2);
		}
		else
		{
			streamNumber(0, 2);
			labelsToPatch.insert({ label, { (uint16_t)(address - 2), 0, 2 } });
		}

		address += secondOffsetSize;

		if (const auto& it = labels.find(secondLabel); it != labels.end())
		{
			streamNumber(it->second.address - address, secondOffsetSize);
		}
		else
		{
			streamNumber(0, offsetSize);
			labelsToPatch.insert({ label, { (uint16_t)(address - secondOffsetSize), 0, secondOffsetSize } });
		}
	}
	else
	{
		error(instruction.token.line, instruction.token.stringValue + ": invalid operands");
	}
}

MemoryAddresing CodeGenerator::resolveMemoryAddressing(Node* node)
{
	AddresingMode addressingMode;

	Node* displacementNode = findNumber(node);
	int16_t displacement = 0;
	uint8_t displacementSize = 0;

	resolveGetaddressOperators(node);
	Node valuesSumNode = Parser::performArithmeticOperations(*node, true);

	if (displacementNode) 
	{
		displacement = displacementNode->token.numberValue;
		displacementSize = displacementNode->token.size;
	}

	addressingMode.mod = displacementSize;

	switch (valuesSumNode.token.numberValue - displacement)
	{
		case 9: // 1001 = BX + SI
		{
			addressingMode.rm = 0; // 000;
			break;
		}
		case 10: // 1010 = BX + DI
		{
			addressingMode.rm = 1; // 001;
			break;
		}
		case 11: // 1011 = BP + SI
		{
			addressingMode.rm = 2; // 010;
			break;
		}
		case 12: // 1100 = BP + DI
		{
			addressingMode.rm = 3; // 011;
			break;
		}
		case 6: // 110 = SI
		{
			addressingMode.rm = 4; // 100;
			break;
		}
		case 7: // 111 = DI
		{
			addressingMode.rm = 5; // 101;
			break;
		}
		case 0:
			addressingMode.mod = 0;
		case 5: // 101 = BP
		{
			addressingMode.rm = 6; // 110;    
			break;
		}
		case 3: // 011 = BX
		{
			addressingMode.rm = 7; // 111;
			break;
		}
	}

	return { addressingMode, displacement, displacementSize };
}

void CodeGenerator::streamNumber(int64_t number, uint16_t size){
	for (int i = 0; i < size; i++) 
	{
		output << (uint8_t)((number >> (8 * i)) & 0xFF);
	}
}

void CodeGenerator::error(uint16_t line, const std::string& message)
{
	std::cout << "Line " << line << ": " << message << '\n';
	exit(-1);
}
