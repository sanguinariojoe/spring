/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#ifndef BLOCK_MAP_H
#define BLOCK_MAP_H

#include <vector>

#include "System/creg/creg_cond.h"

template<typename T>
class BlockMap
{
public:
	typedef std::vector<T> DataType;
	static const int BLOCK_DIMENSIONS = 64;
	static const int BLOCK_SIZE = BLOCK_DIMENSIONS * BLOCK_DIMENSIONS;
	BlockMap() : blocksWidth(0) { }

	void resize(unsigned newWidth, unsigned newHeight) {
		blocksWidth = ((newWidth + BLOCK_DIMENSIONS - 1) / BLOCK_DIMENSIONS);
		data.clear();
		data.resize(((newHeight + BLOCK_DIMENSIONS - 1) / BLOCK_DIMENSIONS) * blocksWidth * BLOCK_SIZE);
	}

	T Get(unsigned x, unsigned y) const {
		const unsigned blockNum = (y / BLOCK_DIMENSIONS) * blocksWidth + (x / BLOCK_DIMENSIONS);
		return data[BLOCK_SIZE * blockNum + (y % BLOCK_DIMENSIONS) * BLOCK_DIMENSIONS + x % BLOCK_DIMENSIONS];
	}

	void Set(unsigned x, unsigned y, T value) {
		const unsigned blockNum = (y / BLOCK_DIMENSIONS) * blocksWidth + (x / BLOCK_DIMENSIONS);
		data[BLOCK_SIZE * blockNum + (y % BLOCK_DIMENSIONS) * BLOCK_DIMENSIONS + x % BLOCK_DIMENSIONS] = value;
	}


	// FOR CREG & CHECKSUM ONLY
	DataType& GetDataRef() { return data; }
	int& GetBlocksWidthRef() { return blocksWidth; }

private:
	DataType data;
	int blocksWidth;
};

#ifdef USING_CREG

namespace creg {

	// IType implemention for std::list
	template<typename T>
	struct BlockMapType : public IType
	{
		BlockMapType(std::shared_ptr<IType> t):elemType(t) {}
		~BlockMapType() {}

		void Serialize(ISerializer* s, void* inst) {
			T& ct = *(T*)inst;
			s->SerializeInt(&ct.GetBlocksWidthRef(), sizeof(int));
			DeduceType<typename T::DataType>::Get()->Serialize(s, &ct.GetDataRef());
		}
		std::string GetName() const { return "BlockMap<" + elemType->GetName() + ">"; }
		size_t GetSize() const { return sizeof(T); }

		std::shared_ptr<IType> elemType;
	};


	// List type
	template<typename T>
	struct DeduceType< BlockMap<T> > {
		static std::shared_ptr<IType> Get() {
			return std::shared_ptr<IType>(new BlockMapType< BlockMap<T> >(DeduceType<T>::Get()));
		}
	};
}

#endif // USING_CREG

#endif