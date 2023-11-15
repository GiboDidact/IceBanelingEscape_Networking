#pragma once
#include <bit>

/* Compression notes

sparse arrays
entropy encoding, if something is usually the same we can assume it is with a bool and more data if its not
fixed point. converting floats to uin32_t, even converting anything else lower like 4 byte int into a short or something
geometry compression on mathematical contraints. x^2 + y^2 = z^2 just pass in 2 of them and calculate third

for architecture you can even write a reflection system to automatically serialize member variables cleanly
*/

//WORKS ON LITTLE ENDIAN NEED TO BYTE SWAP IF USING OTHER ONE
//NEED TO SUPPORT BYTE SWAP
class BitStreamWriter
{
public:
	BitStreamWriter() : buf(nullptr), bithead(0), capacity(0)
	{
		AllocateMemory();
	}
	~BitStreamWriter()
	{
		delete[] buf;
	}

	void WriteBytes(const void* inData, size_t inByteCount)
	{ 
		WriteBits(inData, inByteCount << 3);
	}

	template<typename T>
	void Write(const std::vector<T>& inData)
	{
		size_t size = inData.size();
		Write(size);

		WriteBytes(inData.data(), inData.size() * sizeof(T));
	}

	void Write(uint32_t inData, size_t inBitCount = sizeof(uint32_t) * 8)
	{
		WriteBits(&inData, inBitCount);
	}
	void Write(int inData, size_t inBitCount = sizeof(int) * 8)
	{
		WriteBits(&inData, inBitCount);
	}
	void Write(float inData)
	{
		WriteBits(&inData, sizeof(float) * 8);
	}
	void Write(uint16_t inData, size_t inBitCount = sizeof(uint16_t) * 8)
	{
		WriteBits(&inData, inBitCount);
	}
	void Write(int16_t inData, size_t inBitCount = sizeof(int16_t) * 8)
	{
		WriteBits(&inData, inBitCount);
	}
	void Write(uint8_t inData, size_t inBitCount = sizeof(uint8_t) * 8)
	{
		WriteBits(&inData, inBitCount);
	}
	void Write(bool inData)
	{
		WriteBits(&inData, 1);
	}

	template<typename T>
	void Write(T inData, size_t inBitCount = sizeof(T) * 8)
	{
		static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value,
			"Generic Write only supports primitive data types");
		WriteBits(&inData, inBitCount);
	}

	void Print() const
	{
		uint32_t bytes = currentByte();
		uint32_t leftover = getbitheadOffset();

		int count = 0;
		for (int i = 0; i < bithead; i+=8)
		{
			if(bithead - i < 8)
				PrintByte(buf[count], bithead - i);
			else
				PrintByte(buf[count], 8);
			std::cout << " | ";
			count++;
		}
		std::cout << std::endl;
	}

	void reset()
	{
		if(buf)
			memset(buf, 0, (capacity / 8));
		bithead = 0;
	}
	char* getData() { return buf; }
	uint32_t getCapacity() { return capacity; }
	uint32_t getBytesUsed() 
	{ 
		if (bithead % 8 == 0)
			return (bithead >> 3);
		else
			return (bithead >> 3) + 1; 
	}
	uint32_t getbithead() { return bithead; }
private:
	void PrintByte(uint8_t byte, uint8_t bits) const
	{
		for (int i = bits - 1; i >= 0; i--)
		{
			std::cout << ((byte >> i) & 0x1);
		}
	}

	void WriteBits(const void* data, size_t bits)
	{
		const char* indata = static_cast<const char*>(data);
		int index = 0;
		int i = bits;

		for (i; i > 7; i -= 8)
		{
			WriteBitsinternal(indata[index++], 8);
		}
		if (i > 0)
			WriteBitsinternal(indata[index], i);
	}

	void WriteBitsinternal(uint8_t in_data, size_t bits)
	{
		//if we reach capacity create more
		if (bithead + bits >= capacity)
		{
			AllocateMemory();
		}

		//how many bits are available to copy in this byte
		uint32_t currentByte_copyablebits = (8-getbitheadOffset()); //8-1
		
		uint32_t bitstocopy = std::min(currentByte_copyablebits, static_cast<uint32_t>(bits));
		uint32_t leftoverbits = static_cast<uint32_t>(bits) - bitstocopy;
		bool overflow = (leftoverbits != 0);

		uint8_t datamask = (0xff >> (8 - bitstocopy));
		buf[currentByte()] |= ((in_data & datamask) << getbitheadOffset());
		bithead += bitstocopy;

		//if it overflows finish copying in next byte
		if (overflow)
		{
			uint8_t datamask = (0xff >> (8 - leftoverbits));
			buf[currentByte()] |= ((in_data >> bitstocopy) & datamask);
			bithead += leftoverbits;
		}
	}

	//0-7
	uint32_t getbitheadOffset() const
	{
		return bithead & 0x7;
	}
	uint32_t currentByte() const
	{
		return bithead >> 3;
	}
			
	void AllocateMemory()
	{
		int bytes = (capacity == 0) ? 128 : 2 * (capacity/8);
		//std::cout << "allocating bitwrite " << bytes << " bytes\n";
		char* newbuf = new char[bytes];
		memset(newbuf, 0, bytes);
		capacity = bytes * 8;

		if (!buf)
			buf = newbuf;
		else
		{
			memcpy(newbuf, buf, bytes / 2);
			delete[] buf;
			buf = newbuf;
		}

	}

	char* buf;
	uint32_t bithead;
	uint32_t capacity;
};


class BitStreamReader
{
public:
	BitStreamReader(char* _buff, uint32_t bytes, bool _free_memory) : buf(_buff), bithead(0), capacity(bytes*8), free_memory(_free_memory)
	{
	}
	BitStreamReader(size_t allocationbytes) : buf(nullptr), bithead(0), capacity(allocationbytes * 8), free_memory(true)
	{
		buf = new char[allocationbytes];
	}
	BitStreamReader() : buf(nullptr), bithead(0), capacity(0), free_memory(false) {}

	~BitStreamReader()
	{
		if(free_memory)
			std::free(buf);
		buf = nullptr;
	}

	void Allocate(size_t allocationbytes)
	{
		buf = new char[allocationbytes];
		capacity = allocationbytes * 8;
		bithead = 0;
		free_memory = true;
	}

	void reset()
	{
		if (buf)
			memset(buf, 0, capacity / 8);
		bithead = 0;
	}

	void ReadBytes(void* inData, size_t inByteCount)
	{
		ReadBits(inData, inByteCount << 3);
	}
	void Read(uint32_t* inData, size_t inBitCount = sizeof(uint32_t) * 8)
	{
		ReadBits(inData, inBitCount);
	}
	void Read(int* inData, size_t inBitCount = sizeof(int) * 8)
	{
		ReadBits(inData, inBitCount);
	}
	void Read(float* inData)
	{
		ReadBits(inData, sizeof(float) * 8);
	}
	void Read(uint16_t* inData, size_t inBitCount = sizeof(uint16_t) * 8)
	{
		ReadBits(inData, inBitCount);
	}
	void Read(int16_t* inData, size_t inBitCount = sizeof(int16_t) * 8)
	{
		ReadBits(inData, inBitCount);
	}
	void Read(uint8_t* inData, size_t inBitCount = sizeof(uint8_t) * 8)
	{
		ReadBits(inData, inBitCount);
	}
	void Read(bool* inData)
	{
		ReadBits(inData, 1);
	}
	template<typename T>
	void Read(T* inData, size_t inBitCount = sizeof(T) * 8)
	{
		static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value,
			"Generic Read only supports primitive data types");
		ReadBits(inData, inBitCount);
	}
	
	char* getData() { return buf; }
	uint32_t getCapacity() { return capacity; }
	uint32_t getCapacityBytes() { return capacity / 8; }
	uint32_t getbithead() { return bithead; }
	void setbithead(uint32_t bit) { bithead = bit; }
private:

	void ReadBits(void* outdata, size_t bits)
	{
		char* outdatac = static_cast<char*>(outdata);

		size_t bits_left = bits;
		int count = 0;
		while (bits_left > 0)
		{
			if (bits_left <= 8)
			{
				ReadBitsInternal(outdatac[count++], bits_left);
				break;
			}
			else
			{
				ReadBitsInternal(outdatac[count++], 8);
			}
			bits_left -= 8;
		}
	}

	void ReadBitsInternal(char& data, size_t bits)
	{
		if (bithead + bits > capacity)
		{
			std::cout << "ReadBitsInternal overflow!\n";
			return;
		}
		my_assert(bits > 0 && bits < 9, "readbitsinternal");
		uint32_t bits_available = 8 - getbitheadOffset();
		uint32_t bits_tocopy = std::min(bits_available, (uint32_t)bits);
		uint32_t bits_leftover = bits - bits_tocopy;
		bool overflow = (bits_leftover != 0);

		uint32_t first_byte = (buf[currentByte()] >> getbitheadOffset()) & ~(0xff << bits_tocopy);
		uint32_t second_byte = 0;
		if (overflow)
			second_byte = buf[currentByte() + 1] & ~(0xff << bits_leftover);

		bithead += bits;

		data = first_byte | (second_byte << bits_tocopy);
	}

	//0-7
	uint32_t getbitheadOffset() const
	{
		return bithead & 0x7;
	}
	uint32_t currentByte() const
	{
		return bithead >> 3;
	}

	char* buf;
	uint32_t bithead;
	uint32_t capacity;
	bool free_memory;
};